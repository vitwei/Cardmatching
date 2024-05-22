#include "ProcessData.h"
#include "time.cpp"
#include "yaml-cpp/yaml.h"  

void session(boost::asio::ip::tcp::socket socket, YAML::Node config,
    std::vector<ImageInfo>& imageInfovector, std::shared_mutex& data_Mutex) {
    try {
        std::string dbname = config["dbname"].as<std::string>();
        std::string dbpassword = config["dbpassword"].as<std::string>();
        std::string dbport = config["dbport"].as<std::string>();
        int workport = config["workport"].as<int>();
        int updateport = config["updateport"].as<int>();
        std::string dbtable = config["dbtable"].as<std::string>();
        // ѭ����������
        std::cout << "start socket" << std::endl;
        while (socket.is_open()) {
            // ��������
            std::string received_json;
            while (true) {
                // ����һ������������������
                std::vector<char> buffer(10000000); // ���軺������СΪ 1024 �ֽ�
                // �� socket �ж�ȡ���ݵ���������
                size_t bytes_transferred = socket.read_some(boost::asio::buffer(buffer));
                // �����յ�������׷�ӵ� received_json ��
                received_json.append(buffer.data(), bytes_transferred);
                // ��� received_json �Ƿ���������� JSON ����
                
                if (received_json.find("]") != std::string::npos or received_json== "update") {
                    break; // ������ JSON �����Ѿ�������ϣ��˳�ѭ��
                }
            }
            // ������յ� "exit"�����˳�ѭ��
            if (received_json == "exit") break;
            if (received_json == "update") {
                try {
                    std::cout << "update signal accept" << std::endl;
                    std::string connection_string = "dbname=" + dbname + " user=postgres password=" + dbpassword + " port=" + dbport;
                    pqxx::connection db(connection_string);
                    std::vector<ImageInfo> tempInfovector = DBGetImageInfo(db, dbtable);
                    {
                        data_Mutex.lock_shared();
                        imageInfovector = tempInfovector;
                        data_Mutex.unlock_shared();
                    }

                }
                catch (const std::exception& e) {
                    std::cerr << "update error: " << e.what() << std::endl;
                    std::cerr << boost::stacktrace::stacktrace() << std::endl;
                }
                std::cout << "update success" << std::endl;
                socket.close();
            }
            else {
                try {
                    std::string response_message;
                    cv::Mat data = ProcessdescriptorsJson(received_json);
                    response_message = MatchImg7(data, imageInfovector);
                    boost::asio::write(socket, boost::asio::buffer(response_message));
                    std::cout << response_message << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "update error: " << e.what() << std::endl;
                    std::cerr << boost::stacktrace::stacktrace() << std::endl;
                }
                socket.close();
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in session: " << e.what() << std::endl;
    }
}

int main() {
    YAML::Node config = YAML::LoadFile(R"(E:\WorkWork\Ctest\resource\config.yaml)");
    if (!config) {
        std::cout << "Failed to load config file." << std::endl;
    }
    std::string folderPath = "E:\\WorkWork\\bugproject\\testimg";
    std::vector<ImageInfo> imageInfovector = Imgprocess(folderPath);
    std::shared_mutex data_Mutex;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket(io_context);
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 50300);
    boost::asio::ip::tcp::acceptor acceptor(io_context, endpoint);
    std::cout << "start netserver" << std::endl;
    while (true) {
        // �ȴ�����
        boost::asio::ip::tcp::socket socket(io_context);
        acceptor.accept(socket);
        // ����һ���Ự��������
        std::thread(session, std::move(socket), std::ref(config), std::ref(imageInfovector), std::ref(data_Mutex)).detach();
    }


}




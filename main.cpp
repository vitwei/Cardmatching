#include <iostream>
#include "ProcessData.h"
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>



using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;


int main() {
    string folderPath = "J:\\WorkWork\\bugproject\\data_images";
    vector<ImageInfo2> cvimg = Imgprocess3(folderPath); 
    cout << "数据预处理完成" << endl;

    io_service io;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 9898);
    ip::tcp::acceptor acceptor(io, endpoint);

    cout << "网络预处理完成" << endl;

    while (true) {
        ip::tcp::socket socket(io);
        acceptor.accept(socket);
        std::string received_data(1024, '\0');
        size_t bytes_transferred = socket.read_some(buffer(received_data));
        // 发送数据
        string response_message = MatchImg3(received_data, cvimg);
        boost::asio::write(socket, boost::asio::buffer(response_message));
        cout << "Response sent to Python." << endl;
        socket.close();
    }

}


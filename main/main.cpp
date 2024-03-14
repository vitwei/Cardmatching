#include <iostream>
#include "ProcessData.h"
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <stdexcept>
#include "time.cpp"
#include "pqxx/pqxx"


using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;                    


int main() {
    string folderPath = "J:\\WorkWork\\bugproject\\data_images";
    string configPath = "config.yaml";
    string testpath = "C:\\Users\\vitmcs\\Pictures\\Screenshots\\test.jpg";
    vector<string> imageFiles;
    string Path = "J:\\WorkWork\\bugproject\\testimg";
    for (const auto& entry : filesystem::directory_iterator(Path)) {
        if (entry.is_regular_file()) {
            string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }
    pqxx::connection db("dbname=cardmatch user=postgres password=123 port=5432");
    if (!db.is_open()) {
        cout << "Can't open database" << endl;
    }
    vector<ImageInfo> cvimg = Imgprocess3(folderPath);
    cout << "数据预处理完成" << endl;
   
    for (int i = 0; i < 100; ++i) {
        // 根据需要处理结果，这里只是简单输出结果
        std::cout << "Result " << i + 1 << endl;
        std::string res2 = MatchImg8(imageFiles[i], cvimg);
    }
    
}


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
    pqxx::connection db("dbname=cardmatch user=postgres password=123 port=5432");
    if (db.is_open()) {
        cout << "Opened database successfully: " << db.dbname() << endl;
    }
    else {
        cout << "Can't open database" << endl;
    }

    cout << "数据预处理完成" << endl;
        /*
    vector<ImageInfo> cvimg = Imgprocess3(folderPath);
    cout << "数据预处理完成" << endl;
    start_server(cvimg);
    */
}


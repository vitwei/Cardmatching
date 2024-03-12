#include "ProcessData.h"
#include "time.cpp"
#include "CardDB.hpp"


cv::Ptr<cv::cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher();
cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

vector<ImageInfoGPU> Imgprocess(const string& folderPath) {
    vector<string> imageFiles;
    vector<ImageInfoGPU> imageInfoVector;
    for (const auto& entry : filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }

    for (const auto& jpg : imageFiles) {
        cv::Mat img = cv::imread(jpg, cv::IMREAD_GRAYSCALE);
        vector< cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        sift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
        cv::cuda::GpuMat gpuDescriptors(descriptors);
        imageInfoVector.push_back({ img, jpg, descriptors, keypoints });
    }

    return imageInfoVector;
}

vector<ImageInfo> Imgprocess2(const string& folderPath) {
    vector<string> imageFiles;
    vector<ImageInfo> imageInfoVector;
    for (const auto& entry : filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }

    for (const auto& jpg : imageFiles) {
        cv::Mat img = cv::imread(jpg, cv::IMREAD_GRAYSCALE);
        vector< cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        sift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
        imageInfoVector.push_back({ img, jpg, descriptors });
    }

    return imageInfoVector;
}

vector<ImageInfo> Imgprocess3(const string& folderPath) {
    vector<string> imageFiles;
    vector<ImageInfo> imageInfoVector;
    for (const auto& entry : filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }
    // 用于存储异步任务的 std::future
    vector<future<ImageInfo>> futures;
    for (const auto& jpg : imageFiles) {
        // 异步执行任务
        futures.push_back(async([jpg]() {
            cv::Mat img = cv::imread(jpg, cv::IMREAD_GRAYSCALE);
            vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors;
            sift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
            return ImageInfo{ img, jpg, descriptors};
            }));
    }
    // 等待所有异步任务完成，并获取结果
    for (auto& future : futures) {
        imageInfoVector.push_back(future.get());
    }
    return imageInfoVector;
}

vector<ImageInfo> Imgprocess4(const std::string& folderPath) {
    std::vector<std::string> imageFiles;
    std::vector<ImageInfo> imageInfoVector;

    // 获取所有图像文件路径
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }

    // 并行处理图像
#pragma omp parallel for
    for (int i = 0; i < imageFiles.size(); ++i) {
        // 在每个线程中创建一个独立的 SIFT 对象
        cv::Ptr<cv::SIFT> localSift = cv::SIFT::create();
        std::string jpg = imageFiles[i];
        cv::Mat img = cv::imread(jpg, cv::IMREAD_GRAYSCALE);
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        localSift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
        // 向结果容器添加图像信息
#pragma omp critical
        {
            imageInfoVector.push_back({ img, jpg, descriptors});
        }
    }

    return imageInfoVector;
}

const float ratio_thresh = 0.7f;
std::mutex mtx;

string MatchImg(const string& Path, vector<ImageInfoGPU>& imageInfoVector) {
    cv::Mat test = cv::imread(Path, cv::IMREAD_GRAYSCALE);
    if (test.empty()) {
        // 图像加载失败，进行相应的处理
        return "Failed to load the image from ";
    }
    vector< cv::KeyPoint> testkeypoints;
    cv::Mat testdescriptors;
    sift->detectAndCompute(test, cv::noArray(), testkeypoints, testdescriptors);
    cv::cuda::GpuMat gpuDescriptors1(testdescriptors);
    vector<pair<string, int>> res;
    vector<vector< cv::DMatch> > knn_matches;
    for (const auto& img : imageInfoVector) {
        vector<cv::DMatch> good_matches;
        matcher->knnMatch(img.descriptors, gpuDescriptors1, knn_matches, 2);
        for (size_t i = 0; i < knn_matches.size(); i++)
        {
            if (knn_matches[i][0].distance < ratio_thresh* knn_matches[i][1].distance)
            {
                good_matches.push_back(knn_matches[i][0]);
            }
        }
        res.push_back({ img.filePath,good_matches.size() });
    }
    sort(res.begin(), res.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second; });

    res=vector<pair<string, int>>(res.begin(), res.begin() + 10);
    std::vector<std::string> stringVector;
    // 使用 std::transform 将所有 string 元素提取到 stringVector 中
    std::transform(res.begin(), res.end(), std::back_inserter(stringVector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringVector.begin(), stringVector.end(), std::string(""));
    return concatenatedString;
}

string MatchImg2(const string& Path, vector<ImageInfo>& imageInfoVector) {
    vector< cv::KeyPoint> testkeypoints;
    cv::Mat testdescriptors;
    cv::Mat test = cv::imread(Path, cv::IMREAD_GRAYSCALE);
    sift->detectAndCompute(test, cv::noArray(), testkeypoints, testdescriptors);
    cv::cuda::GpuMat gpuDescriptors1(testdescriptors);
    vector<pair<string, int>> res;
    vector<vector< cv::DMatch> > knn_matches;
    for (const auto& img : imageInfoVector) {
        cv::cuda::GpuMat gpuDescriptors(img.descriptors);
        vector<cv::DMatch> good_matches;
        matcher->knnMatch(gpuDescriptors, gpuDescriptors1, knn_matches, 2);
        for (size_t i = 0; i < knn_matches.size(); i++)
        {
            if (knn_matches[i][0].distance < ratio_thresh* knn_matches[i][1].distance)
            {
                good_matches.push_back(knn_matches[i][0]);
            }
        }
        res.push_back({ img.filePath,good_matches.size() });
    }
    sort(res.begin(), res.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second; });
    res = vector<pair<string, int>>(res.begin(), res.begin() + 10);
    std::vector<std::string> stringVector;
    // 使用 std::transform 将所有 string 元素提取到 stringVector 中
    std::transform(res.begin(), res.end(), std::back_inserter(stringVector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringVector.begin(), stringVector.end(), std::string(""));
    return concatenatedString;
}

string MatchImg3(const string& Path, vector<ImageInfo>& imageInfoVector) {
    vector< cv::KeyPoint> testkeypoints;
    cv::Mat testdescriptors;
    cv::Mat test = cv::imread(Path, cv::IMREAD_GRAYSCALE);
    sift->detectAndCompute(test, cv::noArray(), testkeypoints, testdescriptors);
    cv::cuda::GpuMat gpuDescriptors1(testdescriptors);
    vector<pair<string, int>> res;
    vector<vector< cv::DMatch> > knn_matches;
    for (const auto& img : imageInfoVector) {
        cv::cuda::GpuMat gpuDescriptors(img.descriptors);
        vector<cv::DMatch> good_matches;
        matcher->knnMatch(gpuDescriptors, gpuDescriptors1, knn_matches, 2);
        for (size_t i = 0; i < knn_matches.size(); i++)
        {
            if (knn_matches[i][0].distance < ratio_thresh* knn_matches[i][1].distance)
            {
                good_matches.push_back(knn_matches[i][0]);
            }
        }
        res.push_back({ img.filePath,good_matches.size() });
    }
    vector<pair<string, int>> result;
    for (const auto& item : res) {
        if (item.second > 1000) {
            result.push_back(item);
        }
    }
    sort(result.begin(), result.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second;
        });
    std::vector<std::string> stringVector;
    // 使用 std::transform 将所有 string 元素提取到 stringVector 中
    std::transform(result.begin(), result.end(), std::back_inserter(stringVector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringVector.begin(), stringVector.end(), std::string(""));
    return concatenatedString;
}

string MatchImg4(const string& Path, vector<ImageInfo>& imageInfoVector) {
    try {
        vector<pair<string, int>> result;
        vector<vector<cv::DMatch> > knn_matches;
        vector<cv::KeyPoint> basekeypoints;
        cv::Mat basedescriptors;

        cv::Mat base = cv::imread(Path, cv::IMREAD_GRAYSCALE);
        sift->detectAndCompute(base, cv::noArray(), basekeypoints, basedescriptors);

        cv::cuda::GpuMat basegpuDescriptors(basedescriptors);



        mtx.lock();
        for (const auto& img : imageInfoVector) {
            cv::cuda::GpuMat tempGPUDescriptors(img.descriptors);
            vector<cv::DMatch> good_matches;
            matcher->knnMatch(tempGPUDescriptors, basegpuDescriptors, knn_matches, 2);
            for (size_t i = 0; i < knn_matches.size(); i++)
            {
                if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
                {
                    good_matches.push_back(knn_matches[i][0]);
                }
            }
            /*
            取最大匹配点可能图像的阈值为1000
            */
            if (good_matches.size() > 1000) {
                result.push_back({ img.filePath,good_matches.size() });
            }

        }
        mtx.unlock();



        return Matchrule(result);



    }catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

string MatchImg5(const string& Path, vector<ImageInfo>& imageInfoVector) {
    try {
        vector<cv::KeyPoint> basekeypoints;
        cv::Mat basedescriptors;
        cv::Mat base = cv::imread(Path, cv::IMREAD_GRAYSCALE);
        sift->detectAndCompute(base, cv::noArray(), basekeypoints, basedescriptors);
        cv::cuda::GpuMat basegpuDescriptors(basedescriptors);

        vector<pair<string, int>> result;
        MatchData(imageInfoVector, basegpuDescriptors,result);
        return Matchrule(result);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }

}

vector<pair<string, int>> MatchData(vector<ImageInfo>& imageInfoVector, cv::cuda::GpuMat& basegpuDescriptors, vector<pair<string, int>>& result) {
    vector<vector<cv::DMatch> > knn_matches;
    for (const auto& img : imageInfoVector) {
        cv::cuda::GpuMat tempDescriptors(img.descriptors);
        vector<cv::DMatch> good_matches;
        matcher->knnMatch(tempDescriptors, basegpuDescriptors, knn_matches, 2);
        for (size_t i = 0; i < knn_matches.size(); i++)
        {
            if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
            {
                good_matches.push_back(knn_matches[i][0]);
            }
        }
        /*
        取最大匹配点可能图像的阈值为1000
        */
        if (good_matches.size() > 1000) {
            result.push_back({ img.filePath,good_matches.size() });
        }

    }
    return result;
}

string Matchrule(const vector<pair<string, int>>& result) {
    /*
    取最大匹配点的可能图像的Path 如果没有最大匹配点则返回false
    */
    auto maxElement = std::max_element(result.begin(), result.end(),
        [](const auto& lhs, const auto& rhs) {
            return lhs.second < rhs.second;
        });

    if (maxElement == result.end()) {
        return  "False";
    }
    return maxElement->first;
}

vector<float> ProcessBasecardPrice(std::string PriceUnit) {
    std::istringstream iss(PriceUnit);
    std::vector<float> PriceVector;
    char discard;
    float value;
    if (iss >> discard && discard == '{') {
        while (iss >> value) {
            PriceVector.push_back(value);
            if (iss.peek() == ',')
                iss.ignore();
        }
    }
    return PriceVector;
}

vector<string> ProcessBasecardPricetime(std::string PricetimeUnit) {
    std::istringstream iss(PricetimeUnit);
    std::vector<std::string> pricetime;
    char discard;
    std::string tempstr;

    // 读取 '{'，然后读取字符串，逐个添加到 vector 中
    if (iss >> discard && discard == '{') {
        while (std::getline(iss, tempstr, ',')) {
            // 去除字符串中的空格
            size_t start = tempstr.find_first_not_of(" \t");
            size_t end = tempstr.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos)
                pricetime.push_back(tempstr.substr(start, end - start + 1));
        }
    }
    if (!pricetime.empty()) {
        size_t lastPos = pricetime.back().find_last_not_of(" \t}");
        if (lastPos != std::string::npos)
            pricetime.back().erase(lastPos + 1);
    }
    return pricetime;
}

vector<Basecard> DBmatch(string imgstr, pqxx::connection& connection) {
    try {
        if (!connection.is_open()) {
            cout << "ERROR connection: " << connection.dbname() << endl;
        }
        pqxx::work transaction(connection);
        std::string sql = "SELECT img, textcontent, pricearray, creattime, updatetime, pricetimearray FROM basecard WHERE img = '";
        std::ostringstream oss;
        oss << sql << imgstr << "';";
        std::string ressql = oss.str();
        pqxx::result result = transaction.exec(ressql);
        transaction.commit();
        vector<Basecard> data;
        for (auto row : result) {
            std::vector<float> priceVector = ProcessBasecardPrice(row[2].as<string>());
            std::vector<std::string> pricetimeVector = ProcessBasecardPricetime(row[5].as<string>());
            Basecard temp(row[0].as<string>(), row[1].as<string>(), 
                priceVector, row[3].as<string>(),
                row[4].as<string>(), pricetimeVector);
            data.push_back(temp);
        }
        return data;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

cv::Mat ProcessdescriptorsJson(string& imgjson) {
    imgjson.erase(std::remove(imgjson.begin(), imgjson.end(), '['), imgjson.end());
    imgjson.erase(std::remove(imgjson.begin(), imgjson.end(), ']'), imgjson.end());
    std::stringstream countStream(imgjson);
    int count = 0;
    float temp;
    while (countStream >> temp) {
        ++count;
        countStream.ignore(std::numeric_limits<std::streamsize>::max(), ',');
    }
    float* data = new float[count];

    // 使用 stringstream 将字符串分割为数字
    std::stringstream ss(imgjson);

    // 逐个读取数字并存储到数组中
    for (int i = 0; i < count; ++i) {
        ss >> data[i];
        ss.ignore(std::numeric_limits<std::streamsize>::max(), ',');
    }
    int col = 128;
    int row = count / 128;
    cv::Mat mat(row, col, CV_32F, data);

    return  mat;

}

vector<ImageInfo> DBGetImageInfo(pqxx::connection& connection) {
    try {
        if (!connection.is_open()) {
            cout << "ERROR connection: " << connection.dbname() << endl;
        }
        pqxx::work transaction(connection);
        std::string sql = "SELECT img,imgjson FROM test.jsontest;";
        pqxx::result result = transaction.exec(sql);
        transaction.commit();
        vector<ImageInfo> data;
        for (auto row : result) {
                std::string img = row[0].as<string>();
                std::string imgjson = row[1].as<string>();
                cv::Mat mat = ProcessdescriptorsJson(imgjson);
                ImageInfo imgdata = ImageInfo(img, mat);
                data.push_back(imgdata);
                }
        return data;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}









void handle_connection(tcp::socket socket, vector<ImageInfo>& cvimg, int threadID) {
    try {
        std::string received_data(1024, '\0');
        size_t bytes_transferred = socket.read_some(buffer(received_data));

        // Process received data and generate response
        std::string response_message = MatchImg4(received_data, cvimg);

        // Send the response
        boost::asio::write(socket, boost::asio::buffer(response_message));
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;

        // Handle the exception or log the error...
        boost::asio::write(socket, boost::asio::buffer("Error"));
    }

    // Close the socket
    std::cout << threadID <<" back"<<endl;
    socket.close();
}

void start_server(vector<ImageInfo>& cvimg) {
    io_service io;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 9898);
    ip::tcp::acceptor acceptor(io, endpoint);
    cout << "网络预处理完成" << endl;
    while (true) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distribution(1, 100000000);
        int threadID = distribution(gen);
        ip::tcp::socket socket(io);
        acceptor.accept(socket);
        std::thread worker(handle_connection, std::move(socket), std::ref(cvimg), threadID);
        worker.detach();
        std::cout << "successfulCreate " << threadID << "thread" << endl;
    }
}
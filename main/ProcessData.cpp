#include "ProcessData.h"
#include "time.cpp"
#include "CardDB.h"
#include <omp.h>



cv::Ptr<cv::cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher();
cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

std::vector<ImageInfoGPU> Imgprocess2GPU(const std::string& folderPath) {
    std::vector<std::string> imageFiles;
    std::vector<ImageInfoGPU> imageInfovector;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }

    for (const auto& jpg : imageFiles) {
        cv::Mat img = cv::imread(jpg, cv::IMREAD_GRAYSCALE);
        std::vector< cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        sift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
        cv::cuda::GpuMat gpuDescriptors(descriptors);
        imageInfovector.push_back({ img, jpg, descriptors, keypoints });
    }

    return imageInfovector;
}

std::vector<ImageInfo> Imgprocess(const std::string& folderPath) {
    std::vector<std::string> imageFiles;
    std::vector<ImageInfo> imageInfovector;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }
    // 用于存储异步任务的 std::future
    std::vector<std::future<ImageInfo>> futures;
    for (const auto& jpg : imageFiles) {
        // 异步执行任务
        futures.push_back(std::async([jpg]() {
            cv::Mat img = cv::imread(jpg, cv::IMREAD_GRAYSCALE);
            std::vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors;
            sift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
            return ImageInfo{jpg, descriptors};
            }));
    }
    // 等待所有异步任务完成，并获取结果
    for (auto& future : futures) {
        imageInfovector.push_back(future.get());
    }
    return imageInfovector;
}

const float epsilon = 1e-5;
const float ratio_thresh = 0.7f;
std::mutex mtx;

std::string MatchImg(const std::string& Path, std::vector<ImageInfoGPU>& imageInfovector) {
    cv::Mat test = cv::imread(Path, cv::IMREAD_GRAYSCALE);
    if (test.empty()) {
        // 图像加载失败，进行相应的处理
        return "Failed to load the image from ";
    }
    std::vector< cv::KeyPoint> testkeypoints;
    cv::Mat testdescriptors;
    sift->detectAndCompute(test, cv::noArray(), testkeypoints, testdescriptors);
    cv::cuda::GpuMat gpuDescriptors1(testdescriptors);
    std::vector<std::pair<std::string, int>> res;
    std::vector<std::vector< cv::DMatch> > knn_matches;
    for (const auto& img : imageInfovector) {
        std::vector<cv::DMatch> good_matches;
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
    sort(res.begin(), res.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second; });

    res=std::vector<std::pair<std::string, int>>(res.begin(), res.begin() + 10);
    std::vector<std::string> stringvector;
    // 使用 std::transform 将所有 string 元素提取到 stringvector 中
    std::transform(res.begin(), res.end(), std::back_inserter(stringvector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringvector.begin(), stringvector.end(), std::string(""));
    return concatenatedString;
}

std::string MatchImg2(const std::string& Path, std::vector<ImageInfo>& imageInfovector) {
    std::vector< cv::KeyPoint> testkeypoints;
    cv::Mat testdescriptors;
    cv::Mat test = cv::imread(Path, cv::IMREAD_GRAYSCALE);
    sift->detectAndCompute(test, cv::noArray(), testkeypoints, testdescriptors);
    cv::cuda::GpuMat gpuDescriptors1(testdescriptors);
    std::vector<std::pair<std::string, int>> res;
    std::vector<std::vector< cv::DMatch> > knn_matches;
    for (const auto& img : imageInfovector) {
        cv::cuda::GpuMat gpuDescriptors(img.descriptors);
        std::vector<cv::DMatch> good_matches;
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
    sort(res.begin(), res.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second; });
    res = std::vector<std::pair<std::string, int>>(res.begin(), res.begin() + 10);
    std::vector<std::string> stringvector;
    // 使用 std::transform 将所有 std::string 元素提取到 stringstd::vector 中
    std::transform(res.begin(), res.end(), std::back_inserter(stringvector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringvector.begin(), stringvector.end(), std::string(""));
    return concatenatedString;
}

std::string MatchImg3(const std::string& Path, std::vector<ImageInfo>& imageInfovector) {
    std::vector< cv::KeyPoint> testkeypoints;
    cv::Mat testdescriptors;
    cv::Mat test = cv::imread(Path, cv::IMREAD_GRAYSCALE);
    sift->detectAndCompute(test, cv::noArray(), testkeypoints, testdescriptors);
    cv::cuda::GpuMat gpuDescriptors1(testdescriptors);
    std::vector<std::pair<std::string, int>> res;
    std::vector<std::vector< cv::DMatch> > knn_matches;
    for (const auto& img : imageInfovector) {
        cv::cuda::GpuMat gpuDescriptors(img.descriptors);
        std::vector<cv::DMatch> good_matches;
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
    std::vector<std::pair<std::string, int>> result;
    for (const auto& item : res) {
        if (item.second > 1000) {
            result.push_back(item);
        }
    }
    sort(result.begin(), result.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second;
        });
    std::vector<std::string> stringvector;
    // 使用 std::transform 将所有 std::string 元素提取到 stringstd::vector 中
    std::transform(result.begin(), result.end(), std::back_inserter(stringvector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringvector.begin(), stringvector.end(), std::string(""));
    return concatenatedString;
}

std::string MatchImg4(const std::string& Path, std::vector<ImageInfo>& imageInfovector) {
    try {
        std::vector<std::pair<std::string, int>> result;
        std::vector<std::vector<cv::DMatch> > knn_matches;
        std::vector<cv::KeyPoint> basekeypoints;
        cv::Mat basedescriptors;

        cv::Mat base = cv::imread(Path, cv::IMREAD_GRAYSCALE);
        sift->detectAndCompute(base, cv::noArray(), basekeypoints, basedescriptors);

        cv::cuda::GpuMat basegpuDescriptors(basedescriptors);



        mtx.lock();
        for (const auto& img : imageInfovector) {
            cv::cuda::GpuMat tempGPUDescriptors(img.descriptors);
            std::vector<cv::DMatch> good_matches;
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

std::string MatchImg5(const std::string& Path, std::vector<ImageInfo>& imageInfovector) {
    try {
        Timer time;
        std::vector<cv::KeyPoint> basekeypoints;
        cv::Mat basedescriptors;
        cv::Mat base = cv::imread(Path, cv::IMREAD_GRAYSCALE);
        sift->detectAndCompute(base, cv::noArray(), basekeypoints, basedescriptors);
        cv::cuda::GpuMat basegpuDescriptors(basedescriptors);

        std::vector<std::pair<std::string, int>> result;
        MatchData(imageInfovector, basegpuDescriptors,result);
        return Matchrule(result);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }

}

std::string MatchImg6(cv::Mat& data, std::vector<ImageInfo>& imageInfovector) {
    try {
        cv::cuda::GpuMat basegpuDescriptors(data);
        std::vector<std::pair<std::string, int>> result;
        MatchData(imageInfovector, basegpuDescriptors, result);
        return Matchrule(result);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }

}

std::string MatchImg7(cv::Mat& data, std::vector<ImageInfo>& imageInfovector) {
    try {
        cv::cuda::GpuMat basegpuDescriptors(data);
        std::vector<std::pair<std::string, int>> result;
        MatchDataAsync(imageInfovector, basegpuDescriptors, result);
        return Matchrule(result);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }

}

std::string MatchImg8(const std::string& Path, std::vector<ImageInfo>& imageInfovector) {
    try {
        Timer time;
        std::vector<cv::KeyPoint> basekeypoints;
        cv::Mat basedescriptors;
        cv::Mat base = cv::imread(Path, cv::IMREAD_GRAYSCALE);
        sift->detectAndCompute(base, cv::noArray(), basekeypoints, basedescriptors);
        cv::cuda::GpuMat basegpuDescriptors(basedescriptors);
        std::vector<std::pair<std::string, int>> result;
        MatchDataAsync(imageInfovector, basegpuDescriptors, result);
        return Matchrule(result);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught from MatchImg8" << e.what() << std::endl;
    }

}

std::vector<std::pair<std::string, int>> MatchData(std::vector<ImageInfo>& imageInfovector, cv::cuda::GpuMat& basegpuDescriptors, std::vector<std::pair<std::string, int>>& result) {
    for (const auto& img : imageInfovector) {
        std::vector<std::vector<cv::DMatch> > knn_matches;
        cv::cuda::GpuMat tempDescriptors(img.descriptors);
        std::vector<cv::DMatch> good_matches;
        matcher->knnMatch(tempDescriptors, basegpuDescriptors, knn_matches, 2);
        for (int i = 0; i < knn_matches.size(); i++)
        {
            double threshold_value = ratio_thresh * knn_matches[i][1].distance;
            double distance = knn_matches[i][0].distance;
            if (distance < threshold_value)
            {
                good_matches.push_back(knn_matches[i][0]);
            }
        }
        /*
        取最大匹配点可能图像的阈值为
        */
        if (good_matches.size() > 1000) {
            result.push_back({ img.filePath,good_matches.size() });
        }

    }
    return result;
}

void ProcessImageAsync(const ImageInfo img , cv::cuda::GpuMat& basegpuDescriptors, std::vector<std::pair<std::string, int>>& result, std::mutex& resmutex)
{
    std::vector<std::vector<cv::DMatch>> knn_matches;
    std::vector<cv::DMatch> good_matches;
    cv::cuda::GpuMat tempDescriptors(img.descriptors);
    matcher->knnMatch(tempDescriptors, basegpuDescriptors, knn_matches, 2);
    for (int i = 0; i < knn_matches.size(); i++)
    {
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
        {
            good_matches.push_back(knn_matches[i][0]);
        }
    }
    if (good_matches.size() >1000)
    {
        resmutex.lock(); // Protect shared data
        result.push_back(
            { img.filePath, good_matches.size() }
        );
        resmutex.unlock();
    }
}

std::vector<std::pair<std::string, int>> MatchDataAsync(std::vector<ImageInfo>& imageInfovector, cv::cuda::GpuMat& basegpuDescriptors, std::vector<std::pair<std::string, int>>& result) {
    try {
        std::mutex result_mutex;
        std::vector<std::future<void>> futures;
        for (auto img : imageInfovector)
        {
            futures.push_back(std::async(std::launch::async, ProcessImageAsync, img, std::ref(basegpuDescriptors), std::ref(result), std::ref(result_mutex)));
        }
        // Wait for all async tasks to finish
        for (auto& future : futures)
        {
            future.wait();
        }
        std::cout << result.size() <<std::endl;
        return result;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught from MatchDataAsync" << e.what() << std::endl;
    }
}

std::string Matchrule(const std::vector<std::pair<std::string, int>>& result) {
    /*
    取最大匹配点的可能图像的Path 如果没有最大匹配点则返回false
    */
    try {
        auto maxElement = std::max_element(result.begin(), result.end(),
            [](const auto& lhs, const auto& rhs) {
                return lhs.second < rhs.second;
            });

        if (maxElement == result.end()) {
            return  "False";
        }
        return maxElement->first;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught from Matchrule" << e.what() << std::endl;
    }
}

std::vector<float> ProcessBasecardPrice(std::string PriceUnit) {
    std::istringstream iss(PriceUnit);
    std::vector<float> Pricevector;
    char discard;
    float value;
    if (iss >> discard && discard == '{') {
        while (iss >> value) {
            Pricevector.push_back(value);
            if (iss.peek() == ',')
                iss.ignore();
        }
    }
    return Pricevector;
}

std::vector<std::string> ProcessBasecardPricetime(std::string PricetimeUnit) {
    std::istringstream iss(PricetimeUnit);
    std::vector<std::string> pricetime;
    char discard;
    std::string tempstr;

    // 读取 '{'，然后读取字符串，逐个添加到 std::vector 中
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

std::vector<Basecard> DBmatch(std::string imgstr, pqxx::connection& connection) {
    try {
        if (!connection.is_open()) {
            std::cout << "ERROR connection: " << connection.dbname() << std::endl;
        }
        pqxx::work transaction(connection);
        std::string sql = "SELECT img, textcontent, pricearray, creattime, updatetime, pricetimearray FROM basecard WHERE img = '";
        std::ostringstream oss;
        oss << sql << imgstr << "';";
        std::string ressql = oss.str();
        pqxx::result result = transaction.exec(ressql);
        transaction.commit();
        std::vector<Basecard> data;
        for (auto row : result) {
            std::vector<float> pricevector = ProcessBasecardPrice(row[2].as<std::string>());
            std::vector<std::string> pricetimevector = ProcessBasecardPricetime(row[5].as<std::string>());
            Basecard temp(row[0].as<std::string>(), row[1].as<std::string>(), 
                pricevector, row[3].as<std::string>(),
                row[4].as<std::string>(), pricetimevector);
            data.push_back(temp);
        }
        return data;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

cv::Mat ProcessdescriptorsJson(std::string& imgjson) {
    imgjson.erase(std::remove(imgjson.begin(), imgjson.end(), '['), imgjson.end());
    imgjson.erase(std::remove(imgjson.begin(), imgjson.end(), ']'), imgjson.end());
    imgjson.erase(std::remove_if(imgjson.begin(), imgjson.end(), ::isspace), imgjson.end());
    std::stringstream countStream(imgjson);
    int count = 0;
    float temp;
    while (countStream >> temp) {
        ++count;
        countStream.ignore(std::numeric_limits<std::streamsize>::max(), ',');
    }
    std::vector<float> data;
    countStream.clear();
    countStream.str(imgjson);

    // 逐个读取数字并存储到数组中
    while (countStream >> temp) {
        data.push_back(temp);
        countStream.ignore(std::numeric_limits<std::streamsize>::max(), ',');
    }
    int col = 128;
    int row = count / 128;
    cv::Mat mat(row, col, CV_32F, data.data());
    return  mat.clone();

}

std::vector<ImageInfo> DBGetImageInfo(pqxx::connection& connection,std::string dbtable) {
    try {
        if (!connection.is_open()) {
            std::cout << "ERROR connection: " << connection.dbname() << std::endl;
        }
        pqxx::work transaction(connection);
        std::string sql = "SELECT img,imgdesjson FROM public." + dbtable;
        pqxx::result result = transaction.exec(sql);
        transaction.commit();
        std::cout << "SQL success" << std::endl;
        std::vector<ImageInfo> data;
        data.reserve(result.size());
        omp_set_num_threads(4);
        #pragma omp parallel 
        {
            // 每个线程的临时向量
            std::vector<ImageInfo> temp_data;

            #pragma omp for nowait
            for (int i = 0; i < result.size(); ++i) {
                auto row = result[i];
                std::string img = row[0].as<std::string>();
                std::string imgdesjson = row[1].as<std::string>();
                cv::Mat mat = ProcessdescriptorsJson(imgdesjson);
                temp_data.emplace_back(img, mat);
            }

            // 临界区合并临时向量到主向量
            #pragma omp critical
            data.insert(data.end(), temp_data.begin(), temp_data.end());
        }
        return data;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

std::string vectorBasecardToJson(const std::vector<Basecard>& data) {
    std::string result = "["; // 开始 JSON 数组
    // 遍历 std::vector 中的每个 Basecard 对象
    for (const auto& card : data) {
        // 将当前 Basecard 对象转换为 JSON 字符串，并添加到结果中
        result += card.toJson() + ",";
    }

    // 删除最后一个逗号
    if (!data.empty()) {
        result.pop_back();
    }

    result += "]"; // 结束 JSON 数组

    return result;
}



void GPUserverhandle(boost::asio::ip::tcp::socket& socket, std::vector<ImageInfo>& imageInfovector, std::shared_mutex& data_Mutex) {
    try {
        std::string received_json(1024, '\0');
        size_t bytes_transferred = socket.read_some(boost::asio::buffer(received_json));
        std::string response_message;
        cv::Mat data = ProcessdescriptorsJson(received_json);
        {
            std::shared_lock<std::shared_mutex> lock(data_Mutex);
            response_message = MatchImg7(data, imageInfovector);
        }
        boost::asio::write(socket, boost::asio::buffer(response_message));
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        // Handle the exception or log the error...
        boost::asio::write(socket, boost::asio::buffer("Error"));
    }
    socket.close();
}


void GPUupdatehandle(boost::asio::ip::tcp::socket& socket, std::vector<ImageInfo>& imageInfovector, std::shared_mutex& data_Mutex, std::string dbname, std::string dbpassword, std::string dbport, std::string dbtable) {
    try {
        std::string received_json(1024, '\0');
        size_t bytes_transferred = socket.read_some(boost::asio::buffer(received_json));
        received_json.resize(bytes_transferred);
        std::ostringstream oss;
        oss << received_json << std::endl;
        if (received_json == "update") {
            oss << "update signal accept" << std::endl;
            std::string connection_string = "dbname=" + dbname + " user=postgres password=" + dbpassword + " port=" + dbport;
            pqxx::connection db(connection_string);
            std::vector<ImageInfo> tempImageInfo = DBGetImageInfo(db, dbtable);
            {
                std::unique_lock<std::shared_mutex> lock(data_Mutex);
                imageInfovector = tempImageInfo;
            }
            oss << "update success" << std::endl;
        }
        std::cout << oss.str();
    }
    catch (const std::exception& e) {
        std::cerr << "update error: " << e.what() << std::endl;
        std::cerr << boost::stacktrace::stacktrace() << std::endl;
    }
    socket.close();
}


void start_GPUserver(YAML::Node config) {
    std::string dbname = config["dbname"].as<std::string>();
    std::string dbpassword = config["dbpassword"].as<std::string>();
    std::string dbport = config["dbport"].as<std::string>();
    int workport = config["workport"].as<int>();
    int updateport = config["updateport"].as<int>();
    std::string dbtable = config["dbtable"].as<std::string>();
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::endpoint workendpoint(boost::asio::ip::tcp::v4(), 50301);
    boost::asio::ip::tcp::endpoint updateendpoint(boost::asio::ip::tcp::v4(), 50300);
    boost::asio::ip::tcp::acceptor workacceptor(io_context, workendpoint);
    boost::asio::ip::tcp::acceptor updateacceptor(io_context, updateendpoint);
    std::vector<ImageInfo> imageInfovector;
    std::shared_mutex data_Mutex;
    std::cout << "网络预处理完成" << std::endl;
    while (true) {
        boost::asio::ip::tcp::socket worksocket(io_context);
        workacceptor.accept(worksocket);
        std::thread work(GPUupdatehandle, std::ref(worksocket), std::ref(imageInfovector), std::ref(data_Mutex), dbname, dbpassword, dbport,dbtable);
        work.detach();
    }
}






















void Imagerelation(std::vector<ImageInfo> imageInfovector) {
    std::vector<std::pair<std::string, std::vector<std::pair<std::string, int>>>> res;
    int i = 0;
    for (auto img : imageInfovector) {
        Timer time();
        cv::cuda::GpuMat basegpuDescriptors(img.descriptors);
        std::vector<std::pair<std::string, int>> result;
        MatchDataAsync(imageInfovector, basegpuDescriptors, result);
        res.push_back({img.filePath ,result });
        i++;
        std::cout << i << std::endl;
    }
    std::ofstream file("output.txt");
    if (!file.is_open()) {
        std::cerr << "无法打开文件\n";
        return;
    }

    for (const auto& outerPair : res) {
        file << outerPair.first << " "; // 写入外部字符串  
        for (const auto& innerPair : outerPair.second) {
            file << "(" << innerPair.first << ", " << innerPair.second << ") "; // 写入内部对  
        }
        file << std::endl; // 每行末尾换行  
    }

    file.close();
}

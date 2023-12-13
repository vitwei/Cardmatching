#include "ProcessData.h"


cv::Ptr<cv::cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher();
cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
vector<ImageInfo> Imgprocess(const string& folderPath) {
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
        cv::cuda::GpuMat gpuDescriptors(descriptors);
        imageInfoVector.push_back({ img, jpg, descriptors, keypoints });
    }

    return imageInfoVector;
}

vector<ImageInfo2> Imgprocess2(const string& folderPath) {
    vector<string> imageFiles;
    vector<ImageInfo2> imageInfoVector;
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
        imageInfoVector.push_back({ img, jpg, descriptors, keypoints });
    }

    return imageInfoVector;
}

vector<ImageInfo2> Imgprocess3(const string& folderPath) {
    vector<string> imageFiles;
    vector<ImageInfo2> imageInfoVector;
    for (const auto& entry : filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }
    // ���ڴ洢�첽����� std::future
    vector<future<ImageInfo2>> futures;
    for (const auto& jpg : imageFiles) {
        // �첽ִ������
        futures.push_back(async([jpg]() {
            cv::Mat img = cv::imread(jpg, cv::IMREAD_GRAYSCALE);
            vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors;
            sift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
            return ImageInfo2{ img, jpg, descriptors, keypoints };
            }));
    }
    // �ȴ������첽������ɣ�����ȡ���
    for (auto& future : futures) {
        imageInfoVector.push_back(future.get());
    }
    return imageInfoVector;
}

vector<ImageInfo2> Imgprocess4(const std::string& folderPath) {
    std::vector<std::string> imageFiles;
    std::vector<ImageInfo2> imageInfoVector;

    // ��ȡ����ͼ���ļ�·��
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".png") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }

    // ���д���ͼ��
#pragma omp parallel for
    for (int i = 0; i < imageFiles.size(); ++i) {
        // ��ÿ���߳��д���һ�������� SIFT ����
        cv::Ptr<cv::SIFT> localSift = cv::SIFT::create();
        std::string jpg = imageFiles[i];
        cv::Mat img = cv::imread(jpg, cv::IMREAD_GRAYSCALE);
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        localSift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
        // �����������ͼ����Ϣ
#pragma omp critical
        {
            imageInfoVector.push_back({ img, jpg, descriptors, keypoints });
        }
    }

    return imageInfoVector;
}

const float ratio_thresh = 0.7f;
string MatchImg(const string& Path, vector<ImageInfo>& imageInfoVector) {
    cv::Mat test = cv::imread(Path, cv::IMREAD_GRAYSCALE);
    if (test.empty()) {
        // ͼ�����ʧ�ܣ�������Ӧ�Ĵ���
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
    // ʹ�� std::transform ������ string Ԫ����ȡ�� stringVector ��
    std::transform(res.begin(), res.end(), std::back_inserter(stringVector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringVector.begin(), stringVector.end(), std::string(""));
    return concatenatedString;
}

string MatchImg2(const string& Path, vector<ImageInfo2>& imageInfoVector) {
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
    // ʹ�� std::transform ������ string Ԫ����ȡ�� stringVector ��
    std::transform(res.begin(), res.end(), std::back_inserter(stringVector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringVector.begin(), stringVector.end(), std::string(""));
    return concatenatedString;
}

string MatchImg3(const string& Path, vector<ImageInfo2>& imageInfoVector) {
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
    // ʹ�� std::transform ������ string Ԫ����ȡ�� stringVector ��
    std::transform(result.begin(), result.end(), std::back_inserter(stringVector),
        [](const std::pair<std::string, int>& pair) { return pair.first; });
    std::string concatenatedString = std::accumulate(stringVector.begin(), stringVector.end(), std::string(""));
    return concatenatedString;
}
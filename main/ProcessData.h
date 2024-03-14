#pragma once
#ifndef ProcessData
#define ProcessData

#include "CardDB.h"
#include <random>
#include <omp.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <algorithm> 
#include <string>
#include <utility>
#include <algorithm>
#include <numeric>
#include <future>
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <mutex>
#include <string.h>
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>
#include <thread>


using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;


struct ImageInfoGPU {
    cv::Mat image;
    string filePath;
    cv::cuda::GpuMat descriptors;
    vector< cv::KeyPoint> keypoints;

    ImageInfoGPU(const  cv::Mat& img, const string& path, const  cv::Mat& desc, const vector< cv::KeyPoint>& kpts)
        : image(img), filePath(path), descriptors(desc), keypoints(kpts) {}
};

struct ImageInfo {
    cv::Mat image;
    string filePath;
    cv::Mat descriptors;
    ImageInfo(const cv::Mat& img, const string& path, const  cv::Mat& desc) :
        image(img), filePath(path), descriptors(desc) {}
    ImageInfo(const string& path, const  cv::Mat& desc) :
         filePath(path), descriptors(desc) {}
};

vector<ImageInfoGPU> Imgprocess(const string& folderPath);
vector<ImageInfo> Imgprocess2(const string& folderPath);
vector<ImageInfo> Imgprocess3(const string& folderPath);
vector<ImageInfo> Imgprocess4(const string& folderPath);


string  MatchImg(const string& Path, vector<ImageInfo>& imageInfoVector);
string  MatchImg2(const string& Path, vector<ImageInfo>& imageInfoVector);
string  MatchImg3(const string& Path, vector<ImageInfo>& imageInfoVector);
string  MatchImg4(const string& Path, vector<ImageInfo>& imageInfoVector);
string MatchImg5(const string& Path, vector<ImageInfo>& imageInfoVector);
string MatchImg6(cv::Mat& data, vector<ImageInfo>& imageInfoVector);
string MatchImg7(cv::Mat& data, vector<ImageInfo>& imageInfoVector);
string MatchImg8(const string& Path, vector<ImageInfo>& imageInfoVector);

string Matchrule(const vector<pair<string, int>>& result);

vector<Basecard> DBmatch(string imgstr, pqxx::connection& connection);

vector<float> ProcessBasecardPrice(std::string PriceUnit);

vector<string> ProcessBasecardPricetime(std::string PricetimeUnit);

vector<pair<string, int>> MatchData(vector<ImageInfo>& imageInfoVector, cv::cuda::GpuMat& basegpuDescriptors, vector<pair<string, int>>& result);

void ProcessImageAsync(const ImageInfo& img, cv::cuda::GpuMat& basegpuDescriptors, vector<pair<string, int>>& result, mutex& resmutex);
vector<pair<string, int>> MatchDataAsync(std::vector<ImageInfo>& imageInfoVector, cv::cuda::GpuMat& basegpuDescriptors, std::vector<std::pair<std::string, int>>& result);

cv::Mat ProcessdescriptorsJson(string& imgjson);

vector<ImageInfo> DBGetImageInfo(pqxx::connection& connection);

void serverhandle(tcp::socket& socket, tcp::socket& ToGpusocket, pqxx::connection& connection, std::string& config);
void GPUserverhandle(tcp::socket& socket, vector<ImageInfo>& cvimg);
void start_GPUserver();
void start_server();

#endif  // ProcessData
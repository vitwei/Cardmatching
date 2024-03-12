#pragma once
#ifndef ProcessData
#define ProcessData

#include "CardDB.hpp"
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

string Matchrule(const vector<pair<string, int>>& result);

vector<Basecard> DBmatch(string imgstr, pqxx::connection& connection);

vector<float> ProcessBasecardPrice(std::string PriceUnit);

vector<string> ProcessBasecardPricetime(std::string PricetimeUnit);

vector<pair<string, int>> MatchData(vector<ImageInfo>& imageInfoVector, cv::cuda::GpuMat& basegpuDescriptors, vector<pair<string, int>>& result);

cv::Mat ProcessdescriptorsJson(string& imgjson);

vector<ImageInfo> DBGetImageInfo(pqxx::connection& connection);

void handle_connection(tcp::socket socket, vector<ImageInfo>& cvimg, int threadID);
void start_server(vector<ImageInfo>& cvimg);
#endif  // ProcessData
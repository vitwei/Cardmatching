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
#include <fstream>  
#include "yaml-cpp/yaml.h"  
#include <boost/stacktrace.hpp>




struct ImageInfoGPU {
    cv::Mat image;
    std::string filePath;
    cv::cuda::GpuMat descriptors;
    std::vector< cv::KeyPoint> keypoints;

    ImageInfoGPU(const  cv::Mat& img, const std::string& path, const  cv::Mat& desc, const std::vector< cv::KeyPoint>& kpts)
        : image(img), filePath(path), descriptors(desc), keypoints(kpts) {}
};

struct ImageInfo {
    std::string filePath;
    cv::Mat descriptors;
    ImageInfo(const std::string& path, const  cv::Mat& desc) :
         filePath(path), descriptors(desc) {}

};

std::vector<ImageInfoGPU> Imgprocess2GPU(const std::string& folderPath);
std::vector<ImageInfo> Imgprocess(const std::string& folderPath);


std::string  MatchImg(const std::string& Path, std::vector<ImageInfo>& imageInfovector);
std::string  MatchImg2(const std::string& Path, std::vector<ImageInfo>& imageInfovector);
std::string  MatchImg3(const std::string& Path, std::vector<ImageInfo>& imageInfovector);
std::string  MatchImg4(const std::string& Path, std::vector<ImageInfo>& imageInfovector);
std::string MatchImg5(const std::string& Path, std::vector<ImageInfo>& imageInfovector);
std::string MatchImg6(cv::Mat& data, std::vector<ImageInfo>& imageInfovector);

//cv::Mat& data 为sift格式的descriptors
std::string MatchImg7(cv::Mat& data, std::vector<ImageInfo>& imageInfovector);

std::string MatchImg8(const std::string& Path, std::vector<ImageInfo>& imageInfovector);

std::string Matchrule(const std::vector<std::pair<std::string, int>>& result);

std::vector<Basecard> DBmatch(std::string imgstr, pqxx::connection& connection);
std::vector<float> ProcessBasecardPrice(std::string PriceUnit);

std::vector<std::string> ProcessBasecardPricetime(std::string PricetimeUnit);

std::vector<std::pair<std::string, int>> MatchData(std::vector<ImageInfo>& imageInfovector, cv::cuda::GpuMat& basegpuDescriptors, std::vector<std::pair<std::string, int>>& result);

void ProcessImageAsync(ImageInfo img, cv::cuda::GpuMat& basegpuDescriptors, std::vector<std::pair<std::string, int>>& result, std::mutex& resmutex);

std::vector<std::pair<std::string, int>> MatchDataAsync(std::vector<ImageInfo>& imageInfovector, cv::cuda::GpuMat& basegpuDescriptors, std::vector<std::pair<std::string, int>>& result);

cv::Mat ProcessdescriptorsJson(std::string& imgjson);


std::vector<ImageInfo> DBGetImageInfo(pqxx::connection& connection, std::string dbtable);

void GPUserverhandle(boost::asio::ip::tcp::socket& socket, std::vector<ImageInfo>& imageInfovector, std::shared_mutex& data_Mutex);

void GPUupdatehandle(boost::asio::ip::tcp::socket& socket, std::vector<ImageInfo>& imageInfovector, std::shared_mutex& data_Mutex, std::string dbname, std::string dbpassword, std::string dbport, std::string dbtable);

void Imagerelation(std::vector<ImageInfo> imageInfovector);

void start_GPUserver(YAML::Node config);
#endif  // ProcessData
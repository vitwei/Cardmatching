#pragma once
#ifndef ProcessData
#define ProcessData

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

using namespace std;

struct ImageInfo {
    cv::Mat image;
    string filePath;
    cv::cuda::GpuMat descriptors;
    vector< cv::KeyPoint> keypoints;

    ImageInfo(const  cv::Mat& img, const string& path, const  cv::Mat& desc, const vector< cv::KeyPoint>& kpts)
        : image(img), filePath(path), descriptors(desc), keypoints(kpts) {}
};

struct ImageInfo2 {
    cv::Mat image;
    string filePath;
    cv::Mat descriptors;
    vector< cv::KeyPoint> keypoints;
    ImageInfo2(const cv::Mat& img, const string& path, const  cv::Mat& desc, const vector< cv::KeyPoint>& kpts)
        : image(img), filePath(path), descriptors(desc), keypoints(kpts) {}
};

vector<ImageInfo> Imgprocess(const string& folderPath);
vector<ImageInfo2> Imgprocess2(const string& folderPath);
vector<ImageInfo2> Imgprocess3(const string& folderPath);
vector<ImageInfo2> Imgprocess4(const string& folderPath);


string  MatchImg(const string& Path, vector<ImageInfo>& imageInfoVector);
string  MatchImg2(const string& Path, vector<ImageInfo2>& imageInfoVector);
string  MatchImg3(const string& Path, vector<ImageInfo2>& imageInfoVector);

#endif  // ProcessData
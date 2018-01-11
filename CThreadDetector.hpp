#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class CThreadDetector{
	cv::Mat mProcessImage;
	bool CheckThread(const cv::Mat& pBinaryImage, const float pDefRate);
	float CheckSegment(const cv::Mat& pRowData);
public:
	CThreadDetector(const std::string pImageName);
	bool Detect();
};
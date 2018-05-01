#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class CThreadDetector{
	cv::Mat mProcessImage;
	std::string mImageFileName;
	std::string mExtension;
	static const std::string mcDebugFolder;
	bool mIsDebugMode;
	bool mIsPhotoMode;
	bool ReadImage(cv::Mat& dstImage);
	bool CheckThread(const cv::Mat& pBinaryImage, const float pDefRate);
	float CheckSegment(const cv::Mat& pRowData);
public:
	CThreadDetector(const std::string pImageName, bool pIsDebugMode, bool pIsPhotoMode);
	bool Detect();
};

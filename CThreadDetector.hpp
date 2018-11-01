#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class CReadConfig;

// [act]Detect a thread from the image
//		[ReadImage]
//		 1. Read color image file and decode the image.
//		 2. Convert read image to gray scale one.
//		[Detect]
//		 3. Differentiate the image with Y-prewitt filter to detect edges of the image.
//		 4. Binalyze the differentiated image with p-Tile method (meaningful data is "black" pixel.)
//		 5. Calculate each size of "black" mass and leave them except the largest one.
//		 6. Rotate the image to calculate the width of the thread.
//		[CheckThread]
//		 7. Evaluate the detected image.
class CThreadDetector{
	cv::Mat mProcessImage;					// image data
	std::string mImageFileName;				// imported file name
	std::string mExtension;					// file extension from mImageFileName
	static const std::string mcDebugFolder;	// Destination for the debug images
	bool mIsDebugMode;						// flag of the debug mode
	bool mIsPhotoMode;						// flag of the photo view mode
	bool mMinisizeImageFlag;				// flag of the mini size flag
	const double c_binalyzeRate;			// 
	const double c_blackRate;				// 
	const double c_squareRatio;				// 
	const double c_thickRatio;				// 

	// Read a image from mImageFileName
	bool ReadImage(cv::Mat& dstImage);
	
	// Evaluate the image after processing.
	bool CheckThread(const cv::Mat& pBinaryImage, const float pDefRate);
	
	// 
	float CheckSegment(const cv::Mat& pRowData);
public:
	CThreadDetector(const std::string pImageName, bool pIsDebugMode, bool pIsPhotoMode, bool pMinisizeFlag, const CReadConfig& pConfig);
	bool Detect();
};

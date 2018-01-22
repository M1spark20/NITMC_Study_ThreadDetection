#include "CThreadDetector.hpp"
#include "CImageProcessor.hpp"
#include <iomanip>
#include <numeric>

CThreadDetector::CThreadDetector(const std::string pImageName, bool pIsDebugMode){
	mProcessImage = cv::imread(pImageName);
	mIsDebugMode = pIsDebugMode;
	const std::string::size_type dotPos = pImageName.find(".");
	if(!mIsDebugMode) return;
	if(dotPos == std::string::npos)
		mImageFileName = pImageName;
	else
		mImageFileName = pImageName.substr(0, dotPos);
}

bool CThreadDetector::Detect(){
	if (mProcessImage.empty()) return false;
	CImageProcessor processor;
	cv::Mat procImage;
	const float binalyzeRate = 0.03f;
	cv::cvtColor(mProcessImage, procImage, CV_RGB2GRAY);
	procImage.convertTo(procImage, CV_8UC1);
	processor.Filter3x3(procImage, procImage, (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1));
	processor.BinalyzePTile(procImage, procImage, binalyzeRate);
	if (mIsDebugMode)
		 cv::imwrite("debug/" + mImageFileName + "#1.bmp", procImage);
	processor.LaberingMaxSize(procImage, procImage);
	if (mIsDebugMode)
		 cv::imwrite("debug/" + mImageFileName + "#2.bmp", procImage);
	// ƒf[ƒ^‚ª‚ ‚é‚Ì‚ð”’‰æ‘f‚Æ‚·‚é
	return CheckThread(procImage, binalyzeRate);
}

bool CThreadDetector::CheckThread(const cv::Mat& pBinaryImage, const float pDefRate){
	const cv::Size imageSizeData = pBinaryImage.size();
	const float size = static_cast<float>(imageSizeData.width * imageSizeData.height);
	// for debug image(write rotated thread)
	cv::Mat debugImage;
	if(mIsDebugMode)
		debugImage = cv::Mat(imageSizeData, CV_8UC1, cv::Scalar(255));

	std::vector<cv::Point> nonZeroList;
	cv::findNonZero(pBinaryImage, nonZeroList);
	const float blackNum = static_cast<float>(nonZeroList.size());
	const float blackRate = (blackNum / size) / pDefRate;
	std::cout << "decreaseRate: " << std::setprecision(3) << blackRate*100.f << "% require: 20%" << std::endl;
	/*if (blackRate < 0.2){
		std::cout << "      result: NG" << std::endl;
		return false;
	}*/

	cv::Point2f sidePoints[2];
	for (int x = 0; x < imageSizeData.width; ++x){
		if (CheckSegment(pBinaryImage.col(x)) < 0) continue;
		sidePoints[0].x = static_cast<float>(x);
		sidePoints[0].y = CheckSegment(pBinaryImage.col(x));
		break;
	}
	for (int x = imageSizeData.width-1; x >= 0; --x){
		if (CheckSegment(pBinaryImage.col(x)) < 0) continue;
		sidePoints[1].x = static_cast<float>(x);
		sidePoints[1].y = CheckSegment(pBinaryImage.col(x));
		break;
	}

	const cv::Point2f wholeSize(sidePoints[1].x - sidePoints[0].x, sidePoints[1].y - sidePoints[0].y);
	const double wholeAngle = atan2(wholeSize.y, wholeSize.x);
	const cv::Mat affineMat = (cv::Mat_<float>(2,2) <<
		cos(wholeAngle), sin(wholeAngle), -sin(wholeAngle), cos(wholeAngle));
	std::cout << "        line: " << sidePoints[0] << " - " << sidePoints[1]
		<< " / angle(deg): " << wholeAngle * 180.f / CV_PI << std::endl;

	cv::Point2f blackRect[2] = { nonZeroList[0], nonZeroList[0] };
	for (std::vector<cv::Point>::iterator it = nonZeroList.begin()+1; it != nonZeroList.end(); ++it){
		const cv::Mat roundedPos = 	affineMat * (cv::Mat_<float>(2,1) << it->x, it->y);
		const cv::Point2f check(roundedPos.at<float>(0), roundedPos.at<float>(1));
		blackRect[0].x = check.x < blackRect[0].x ? check.x : blackRect[0].x;
		blackRect[0].y = check.y < blackRect[0].y ? check.y : blackRect[0].y;
		blackRect[1].x = check.x > blackRect[1].x ? check.x : blackRect[1].x;
		blackRect[1].y = check.y > blackRect[1].y ? check.y : blackRect[1].y;
		// write debug image
		if(!mIsDebugMode) continue;
		const cv::Point2i dp(check);
		if(dp.x<0 || dp.x>=imageSizeData.height) continue;
		if(dp.y<0 || dp.y>=imageSizeData.width) continue;
		debugImage.at<uchar>(dp.y, dp.x) = 0;
	}
	if (mIsDebugMode)
		 cv::imwrite("debug/" + mImageFileName + "#3.bmp", debugImage);
	const cv::Point2f len = blackRect[1] - blackRect[0];
	const float ratio = len.x > len.y ? len.x / len.y : len.y / len.x;
	const float distance = static_cast<float>(cv::norm(sidePoints[1] - sidePoints[0]));
	const float expectThick = static_cast<float>(nonZeroList.size()) / distance;
	const float realThick = len.x > len.y ? len.y : len.x;
	const float thickRatio = realThick / expectThick;
	std::cout << "   convRange: " << std::setprecision(7) << len << std::endl;
	std::cout << "       ratio: " << std::setprecision(3) << ratio
		<< ", require: >=5" << std::endl;
	std::cout << "    distance: " << std::setprecision(5) << distance << " /";
	std::cout << " thick: " << "[" << expectThick << ", " << realThick << "]" << std::endl;
	std::cout << "  thickRatio: " << std::setprecision(3) << thickRatio << std::endl;
	if (ratio < 5 || blackRate < 0.2f){
		std::cout << "      result: NG" << std::endl;
		return false;
	}
	else {
		std::cout << "      result: OK!" << std::endl;
		return true;
	}
}

float CThreadDetector::CheckSegment(const cv::Mat& pRowData){
	if(cv::countNonZero(pRowData) == 0) return -1.f;
	std::vector<cv::Point> segmentNonZero;
	cv::findNonZero(pRowData, segmentNonZero);
	const float test = std::accumulate(segmentNonZero.begin(), segmentNonZero.end(),
		cv::Point(0, 0)).y / static_cast<float>(segmentNonZero.size());
	return test;
}

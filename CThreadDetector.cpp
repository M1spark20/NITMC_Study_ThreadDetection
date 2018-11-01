#include "CThreadDetector.hpp"
#include "CImageProcessor.hpp"
#include "CTimeMeasure.hpp"
#include "CReadConfig.hpp"
#include <fstream>
#include <iomanip>
#include <numeric>

// external initializer for constant string member; mcDebugFolder.
const std::string CThreadDetector::mcDebugFolder = "debugImage/";

// [act]Initialize CThreadDetector class with parameters and filename.
//		Set flags and detect file name and extension.
// [prm]pImageName		: set image's file name including the extension.
//		pIsDebugMode	: whether set the debug mode.
//		pIsPhotoMode	: whether set the photo view mode.
//		pMinisizeFlag	: whether set the mini size flag (Only available with bgr image.)
// [ret]None.
CThreadDetector::CThreadDetector(const std::string pImageName, bool pIsDebugMode, bool pIsPhotoMode, bool pMinisizeFlag, const CReadConfig& pConfig) : 
		c_binalyzeRate(pConfig.outputConfig(eBinalyzeThreshold)),
		c_blackRate(pConfig.outputConfig(eBlackRate)),
		c_squareRatio(pConfig.outputConfig(eSquareRatio)),
		c_thickRatio(pConfig.outputConfig(eBendRate)){
	// set flags
	mIsDebugMode		= pIsDebugMode;
	mIsPhotoMode		= pIsPhotoMode;
	mMinisizeImageFlag	= pMinisizeFlag;
	// check the extension
	const std::string::size_type dotPos = pImageName.find(".");
	// if extension is not found:
	if(dotPos == std::string::npos)
		mImageFileName = pImageName;
	// if extension is found:
	else {
		mImageFileName = pImageName.substr(0, dotPos);
		mExtension = pImageName.substr(dotPos);
	}
}

// dstImage: CV_8UC1
bool CThreadDetector::ReadImage(cv::Mat& dstImage){
	if (mExtension == ".bgr"){
		const int width  = 640 / (mMinisizeImageFlag ? 2:1);
		const int height = 480 / (mMinisizeImageFlag ? 2:1);
		const int pixel = width*height, colorNum = 3;
		std::ifstream ifs(mImageFileName + mExtension, std::ios::binary);
		if (!ifs) return false;
		dstImage = cv::Mat(height, width, CV_8UC3);
		const int readSize = sizeof(*dstImage.data)*pixel*colorNum;
		if(mIsDebugMode | mIsPhotoMode)
			std::cout << ".bgr dataSize[byte]: " << readSize << std::endl;
		ifs.read((char*)dstImage.data, readSize);
		/*std::ofstream ofs("sift.csv");
		ofs << cv::format(dstImage, "csv") << std::endl;*/
	}
	else {
		dstImage = cv::imread(mImageFileName + mExtension);
	}
	cv::cvtColor(dstImage, dstImage, CV_RGB2GRAY);
	dstImage.convertTo(dstImage, CV_8UC1);
	return true;
}

bool CThreadDetector::Detect(){
	CImageProcessor processor;
	cv::Mat procImage;
	CTimeMeasure timeManager;
	const float binalyzeRate = c_binalyzeRate;
	if(mIsDebugMode | mIsPhotoMode) timeManager.StartInput();

	ReadImage(procImage);
	if (procImage.empty()) return false;
	if (mIsDebugMode | mIsPhotoMode){
		timeManager.GoToNextSection("Read Image Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#0.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	if (mIsPhotoMode) {
		timeManager.WriteResult();
		cv::imshow("photoViewer : press any key to exit.", procImage);
		cv::waitKey();
		return true;
	}

	processor.Filter3x3(procImage, procImage, (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1));
	if (mIsDebugMode) timeManager.GoToNextSection("Y-Prewitt Filter Time");

	processor.BinalyzePTile(procImage, procImage, binalyzeRate);

	if (mIsDebugMode){
		timeManager.GoToNextSection("P-Tile Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#1.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	processor.LaberingMaxSize(procImage, procImage);
	if (mIsDebugMode){
		timeManager.GoToNextSection("Labeling Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#2.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	// ƒf[ƒ^‚ª‚ ‚é‚Ì‚ð”’‰æ‘f‚Æ‚·‚é
	bool ans = CheckThread(procImage, binalyzeRate);
	if (mIsDebugMode){
		timeManager.GoToNextSection("Detecting Time");
		timeManager.WriteResult();
	}
	std::cout << "\n";
	return ans;
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
	std::cout << "BinalyzeRate: " << std::setprecision(3) << pDefRate*100.f << "%" << std::endl;
	std::cout << "decreaseRate: " << std::setprecision(3) << blackRate*100.f
		<< "% require: " << std::setprecision(3) << c_blackRate*100.f << "%" << std::endl;
	/*if (blackRate < 0.2){
		std::cout << "      result: NG" << std::endl;
		return false;
	}*/

	cv::Point2f sidePoints[2];
	for (int x = 0; x < imageSizeData.width; ++x){
		const float yPos = CheckSegment(pBinaryImage.col(x));
		if (yPos < 0) continue;
		sidePoints[0].x = static_cast<float>(x);
		sidePoints[0].y = yPos;
		break;
	}
	for (int x = imageSizeData.width-1; x >= 0; --x){
		const float yPos = CheckSegment(pBinaryImage.col(x));
		if (yPos < 0) continue;
		sidePoints[1].x = static_cast<float>(x);
		sidePoints[1].y = yPos;
		break;
	}

	const cv::Point2f wholeSize(sidePoints[1].x - sidePoints[0].x, sidePoints[1].y - sidePoints[0].y);
	const double wholeAngle = atan2(wholeSize.y, wholeSize.x);
	const cv::Mat affineMat = (cv::Mat_<float>(2,2) <<
		cos(wholeAngle), sin(wholeAngle), -sin(wholeAngle), cos(wholeAngle));
	std::cout << "        line: " << sidePoints[0] << " - " << sidePoints[1]
		<< " / angle(deg): " << wholeAngle * 180.f / CV_PI << std::endl;

	cv::Point2f blackRect[2];
	for (std::vector<cv::Point>::iterator it = nonZeroList.begin(); it != nonZeroList.end(); ++it){
		const cv::Mat roundedPos = 	affineMat * (cv::Mat_<float>(2,1) << it->x, it->y);
		const cv::Point2f check(roundedPos.at<float>(0), roundedPos.at<float>(1));
		if (it == nonZeroList.begin()){
			blackRect[0] = check; blackRect[1] = check;
		} else {
			blackRect[0].x = check.x < blackRect[0].x ? check.x : blackRect[0].x;
			blackRect[0].y = check.y < blackRect[0].y ? check.y : blackRect[0].y;
			blackRect[1].x = check.x > blackRect[1].x ? check.x : blackRect[1].x;
			blackRect[1].y = check.y > blackRect[1].y ? check.y : blackRect[1].y;
		}
		// write debug image
		if(!mIsDebugMode) continue;
		const cv::Point2i dp(check);
		if(dp.x<0 || dp.x>=imageSizeData.width) continue;
		if(dp.y+50<0 || dp.y+50>=imageSizeData.height) continue;
		debugImage.at<uchar>(dp.y+50, dp.x) = 0;
	}
	if (mIsDebugMode)
		 cv::imwrite(mcDebugFolder + mImageFileName + "#3.bmp", debugImage);
	const cv::Point2f len = blackRect[1] - blackRect[0];
	const float ratio = len.x > len.y ? len.x / len.y : len.y / len.x;
	//const float distance = static_cast<float>(cv::norm(sidePoints[1] - sidePoints[0]));
	//const float expectThick = static_cast<float>(nonZeroList.size()) / distance;
	// 20180131 refact distance: norm->longer length of square (almost equal, norm > longer)
	// equally thick =  shorter / (num of black dot / longer)
	const float expectThick = static_cast<float>(nonZeroList.size()) / (len.x > len.y ? len.x : len.y);
	const float realThick = len.x > len.y ? len.y : len.x;
	const float thickRatio = len.x * len.y / static_cast<float>(nonZeroList.size());
	std::cout << "   convRange: " << std::setprecision(7) << len << std::endl;
	std::cout << "       ratio: " << std::setprecision(3) << ratio
		<< ", require: >=" << std::setprecision(3) << c_squareRatio << std::endl;
	std::cout << "    blackNum: " << std::setprecision(5) << nonZeroList.size() << " /";
	std::cout << " thick: " << "[" << expectThick << ", " << realThick << "]" << std::endl;
	std::cout << "  thickRatio: " << std::setprecision(3) << thickRatio
		<< ", require: <=" << c_thickRatio << std::endl;
	if (ratio < c_squareRatio || blackRate < c_blackRate || thickRatio > c_thickRatio){
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

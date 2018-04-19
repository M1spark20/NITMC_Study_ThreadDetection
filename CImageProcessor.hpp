// 4 charactor's tab is recommended.

#pragma once
#include <opencv2/opencv.hpp>

class CImageProcessor{
// [act]
	typedef std::vector<std::pair<cv::Point, int>> LabelData;

public:
	// [機能]指定範囲のMatをコピーする
	void CopyMatRect(const cv::Mat& pSrc, cv::Mat& pDst, const cv::Rect& pSrcRect, const cv::Rect& pDstRect);
	// 端点処理を含んだ3x3フィルタ(YPrewitt用)
	void Filter3x3(const cv::Mat& pSrcImage, cv::Mat& pDstImage, const cv::Mat& pFilter3x3);
	// 固定しきい値による2値化
	void BinalyzeThreshold(const cv::Mat& pSrcImage, cv::Mat& pDstImage, int pThrethold);
	// Pタイル法による2値化
	void BinalyzePTile(const cv::Mat& pSrcImage, cv::Mat& pDstImage, double pBlackRate);
	// ラベリングを行って面積が最大となる部分のみを抽出
	void LaberingMaxSize(const cv::Mat& pSrcImage, cv::Mat& pDstImage);
	// 拡大縮小処理(false: 縮小 / true: 拡大 | vector配列で指定)
	//cv::Mat MinimizeExtend(const cv::Mat pBinaryImage, std::queue<bool> pJob);

private:
	LabelData::reverse_iterator FindLabelRelative(LabelData& data, cv::Point findFor);
	void RefreshLabel(LabelData& data, int srcLabel, int dstLabel, std::vector<int>& pLabelCounter);
};

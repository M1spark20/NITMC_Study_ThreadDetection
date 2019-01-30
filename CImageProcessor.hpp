// 4 charactor's tab is recommended.

#pragma once
#include <opencv2/opencv.hpp>

// [act]撮影画像に対して画像処理を行う
//		CThreadDetectorクラスから画像を受け取り各関数で必要な処理を行う
//		本クラスの関数は画像処理に使用されるアルゴリズムが1つずつ実装されている
class CImageProcessor{
	typedef std::vector<std::pair<cv::Point, int>> LabelData;

public:
	// [act]指定範囲のMatをコピーする
	void CopyMatRect(const cv::Mat& pSrc, cv::Mat& pDst, const cv::Rect& pSrcRect, const cv::Rect& pDstRect);
	// [act]端点処理を含んだ3x3フィルタ(YPrewitt用)を画像に描ける
	void Filter3x3(const cv::Mat& pSrcImage, cv::Mat& pDstImage, const cv::Mat& pFilter3x3);
	// [act]固定しきい値による2値化を行う
	void BinalyzeThreshold(const cv::Mat& pSrcImage, cv::Mat& pDstImage, int pThrethold);
	// [act]Pタイル法による2値化を行う
	int  BinalyzePTile(const cv::Mat& pSrcImage, cv::Mat& pDstImage, double pBlackRate);
	// [act]ラベリングを行って面積が最大となる部分のみを抽出する
	void LaberingMaxSize(const cv::Mat& pSrcImage, cv::Mat& pDstImage);
	
	// 画像回転処理については、CThreadDetectorクラス内関数にて実装されている
	
	// [act]拡大縮小処理(false: 縮小 / true: 拡大 | vector配列で指定) - 不使用になりました
	//cv::Mat MinimizeExtend(const cv::Mat pBinaryImage, std::queue<bool> pJob);

private:
	// [act]LabelDataが整列されていると仮定して、FindForを対象としたマスがdataに存在するか検索する
	LabelData::reverse_iterator FindLabelRelative(LabelData& data, cv::Point findFor);
	
	// [act]ラベリングにおける判定終了後のラベル番号の再割り当てを行う
	void RefreshLabel(LabelData& data, int srcLabel, int dstLabel, std::vector<int>& pLabelCounter);
};

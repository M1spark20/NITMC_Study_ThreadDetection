// 4 charactor's tab is recommended.

#pragma once
#include <opencv2/opencv.hpp>

class CImageProcessor{
// [act]
	typedef std::vector<std::pair<cv::Point, int>> LabelData;

public:
	// [�@�\]�w��͈͂�Mat���R�s�[����
	void CopyMatRect(const cv::Mat& pSrc, cv::Mat& pDst, const cv::Rect& pSrcRect, const cv::Rect& pDstRect);
	// �[�_�������܂�3x3�t�B���^(YPrewitt�p)
	void Filter3x3(const cv::Mat& pSrcImage, cv::Mat& pDstImage, const cv::Mat& pFilter3x3);
	// �Œ肵�����l�ɂ��2�l��
	void BinalyzeThreshold(const cv::Mat& pSrcImage, cv::Mat& pDstImage, int pThrethold);
	// P�^�C���@�ɂ��2�l��
	void BinalyzePTile(const cv::Mat& pSrcImage, cv::Mat& pDstImage, double pBlackRate);
	// ���x�����O���s���Ėʐς��ő�ƂȂ镔���݂̂𒊏o
	void LaberingMaxSize(const cv::Mat& pSrcImage, cv::Mat& pDstImage);
	// �g��k������(false: �k�� / true: �g�� | vector�z��Ŏw��)
	//cv::Mat MinimizeExtend(const cv::Mat pBinaryImage, std::queue<bool> pJob);

private:
	LabelData::reverse_iterator FindLabelRelative(LabelData& data, cv::Point findFor);
	void RefreshLabel(LabelData& data, int srcLabel, int dstLabel, std::vector<int>& pLabelCounter);
};

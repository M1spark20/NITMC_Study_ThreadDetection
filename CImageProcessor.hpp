// 4 charactor's tab is recommended.

#pragma once
#include <opencv2/opencv.hpp>

// [act]�B�e�摜�ɑ΂��ĉ摜�������s��
//		CThreadDetector�N���X����摜���󂯎��e�֐��ŕK�v�ȏ������s��
//		�{�N���X�̊֐��͉摜�����Ɏg�p�����A���S���Y����1����������Ă���
class CImageProcessor{
	typedef std::vector<std::pair<cv::Point, int>> LabelData;

public:
	// [act]�w��͈͂�Mat���R�s�[����
	void CopyMatRect(const cv::Mat& pSrc, cv::Mat& pDst, const cv::Rect& pSrcRect, const cv::Rect& pDstRect);
	// [act]�[�_�������܂�3x3�t�B���^(YPrewitt�p)���摜�ɕ`����
	void Filter3x3(const cv::Mat& pSrcImage, cv::Mat& pDstImage, const cv::Mat& pFilter3x3);
	// [act]�Œ肵�����l�ɂ��2�l�����s��
	void BinalyzeThreshold(const cv::Mat& pSrcImage, cv::Mat& pDstImage, int pThrethold);
	// [act]P�^�C���@�ɂ��2�l�����s��
	int  BinalyzePTile(const cv::Mat& pSrcImage, cv::Mat& pDstImage, double pBlackRate);
	// [act]���x�����O���s���Ėʐς��ő�ƂȂ镔���݂̂𒊏o����
	void LaberingMaxSize(const cv::Mat& pSrcImage, cv::Mat& pDstImage);
	
	// �摜��]�����ɂ��ẮACThreadDetector�N���X���֐��ɂĎ�������Ă���
	
	// [act]�g��k������(false: �k�� / true: �g�� | vector�z��Ŏw��) - �s�g�p�ɂȂ�܂���
	//cv::Mat MinimizeExtend(const cv::Mat pBinaryImage, std::queue<bool> pJob);

private:
	// [act]LabelData�����񂳂�Ă���Ɖ��肵�āAFindFor��ΏۂƂ����}�X��data�ɑ��݂��邩��������
	LabelData::reverse_iterator FindLabelRelative(LabelData& data, cv::Point findFor);
	
	// [act]���x�����O�ɂ����锻��I����̃��x���ԍ��̍Ċ��蓖�Ă��s��
	void RefreshLabel(LabelData& data, int srcLabel, int dstLabel, std::vector<int>& pLabelCounter);
};

#include "CImageProcessor.hpp"
#include <array>
#include <fstream>

// CImageProcessor::CopyMatRect()
// [act]�w��͈͂�Mat���R�s�[����
// [prm]pSrc	: �f�[�^�R�s�[��
//		pDst	: �f�[�^�R�s�[��
//		pSrcRect: src�̃R�s�[�͈�(x,y,w,h)
//		pDstRect: dst�̃R�s�[�͈�(x,y,w,h)
void CImageProcessor::CopyMatRect(const cv::Mat& pSrc, cv::Mat& pDst, const cv::Rect& pSrcRect, const cv::Rect& pDstRect){
	const cv::Mat srcData(pSrc, pSrcRect);
	cv::Mat dstData(pDst, pDstRect);
	dstData = cv::Scalar(0) + srcData;
	/*std::ofstream ofs2("sift.csv");
	ofs2 << cv::format(pDst, "csv") << std::endl;*/
}

// CImageProcessor::Filter3x3()
// [act]�[�_�������܂�3x3�t�B���^(YPrewitt�p)
//		�[�_�������s�����ƂŐ}�`�̒[���Ńm�C�Y���������Ȃ��悤�H�v���Ă���
// [prm]pSrcImage	: �������s���摜(CV_8UC) (y,x)
//		pDstImage	: �������ʊi�[��(CV_32F)
//					  �������A���̉摜�ɂ�[0,255]�ȊO�̒l���܂މ\��������(���̒l���܂�)�B
//		pFilter3x3	: ��������3x3�̃t�B���^(CV_32FC1)
void CImageProcessor::Filter3x3(const cv::Mat& pSrcImage, cv::Mat& pDstImage, const cv::Mat& pFilter3x3){
	// �󂯎�����摜�f�[�^��float�^�ɕϊ�����
	cv::Mat processData;
	pSrcImage.convertTo(processData, CV_32FC1);
	
	/*	���ꂩ���邱�Ƃ�}�Ő���
		temp1�͌��摜�̑傫�����㉺���E+1
		temp2��temp1�̑傫����肳��ɏ㉺���E+1, �摜�O���͒[�_�̃f�[�^���R�s�[����
		temp1��temp2�̃f�[�^�����ɑ����グ�Ă����ƁA�[�_�������ł���
		����ƁA�Y�t�̉摜�̂悤�ɒ[�_�������s����
		�Ύl�p���摜�͈́A�l�p���g���͈�
		�Ԙg�����L���Ȏ擾�͈͂ł���A�����temp1���㉺���E-1����Əo�͂ɂȂ�
	*/
	// ���摜�̃T�C�Y���擾����temp1��temp2�𐶐��E������
	int row = processData.rows, col = processData.cols;	// row: �s(y), col:��(x)
	cv::Mat temp1(row+2, col+2, CV_32FC1, cv::Scalar(0)), temp2(row+4, col+4, CV_32FC1, cv::Scalar(0));
	//std::cout << processData.type() << ", " << temp1.type() << ", " << temp2.type() << std::endl;
	CopyMatRect(processData, temp2, cv::Rect(    0,     0,   1, row), cv::Rect(    1,     2,   1, row));
	CopyMatRect(processData, temp2, cv::Rect(col-1,     0,   1, row), cv::Rect(col+2,     2,   1, row));
	CopyMatRect(processData, temp2, cv::Rect(    0,     0, col,   1), cv::Rect(    2,     1, col,   1));
	CopyMatRect(processData, temp2, cv::Rect(    0, row-1, col,   1), cv::Rect(    2, row+2, col,   1));

	/*cv::Mat temp3; temp2.convertTo(temp3, CV_8UC1);
	cv::imshow("Display Image", temp3);
	cv::waitKey();*/
	CopyMatRect( processData, temp2, cv::Rect(0,0,col,row), cv::Rect(2,2,col,row) );
	/*// �f�o�b�O�o��
	temp2.convertTo(temp3, CV_8UC1);
	cv::imshow("Display Image", temp3);
	cv::waitKey();*/
	
	// �������
	for(int y=0; y<3; ++y)
	for(int x=0; x<3; ++x){
		const cv::Mat addition = cv::Mat(temp2, cv::Rect(y, x, col+2, row+2));
		// std::cout << processData.type() << ", " << temp1.type() << ", " << temp2.type() << ", " << addition.type() << std::endl;
		temp1 +=  addition * pFilter3x3.at<float>(cv::Point(y,x));
	}

	// �o�͏���
	cv::Mat out(temp1, cv::Rect(1, 1, col, row));
	out.copyTo(pDstImage);
}

// �Œ肵�����l�ɂ��2�l��
void CImageProcessor::BinalyzeThreshold(const cv::Mat& pSrcImage, cv::Mat& pDstImage, int pThrethold){
	cv::Mat processData = pSrcImage <= pThrethold;
	processData.copyTo(pDstImage);
}

// [act]P�^�C���@�ɂ��2�l��
//		��f�̍ő�l����w��臒l�̗��̉�f������f�ƂȂ�悤��2�l�����s��
// [prm]pSrcImage	: 2�l�����s���摜(CV_[XX]C1 -> CV_8UC1�ɓ����ϊ�)
//		pBlackRate	: ����f�Ƃ���䗦 range->[0.0, 1.0]
void CImageProcessor::BinalyzePTile(const cv::Mat& pSrcImage, cv::Mat& pDstImage, double pBlackRate){
	// CV_8UC1�ɕϊ�
	cv::Mat processData;
	pSrcImage.convertTo(processData, CV_8UC1);

	// �摜�T�C�Y���擾���ĕK�v�ȍ���f�����v�Z
	cv::Size s = processData.size();
	const int order = s.width * s.height * pBlackRate;

	// �q�X�g�O�����̌v�Z
	cv::MatND hist;
	const int bin_nums[] = { 256 }, channels[] = {0};
	const float hist_range[] = { 0, 256 }, *ranges[] = { hist_range };
	cv::calcHist(&processData, 1, channels, cv::Mat(), hist, 1, bin_nums, ranges);
	//std::cout << hist << std::endl;

	// �q�X�g�O�����̏ォ�獕��f�������Z
	// �P�x�̍��������瑫���グ�Ă����A臒l�𒴂����Ƃ���őł��؂�
	// openCV��0�����255�ւ̖O�a���������p����2�l�������摜���o��
	int bCheck = 0;
	for (int th = 255; th >= 0; --th){
		bCheck += static_cast<int>( hist.at<float>(th) );
		if (bCheck - order >= 0){
			std::cout << "Image Size: " << s.width << " * " << s.height << "\n";
			std::cout << "Pixels Purpose: " << order << ", Black Pixels: " << bCheck << ", Threshold: " << th << "\n";
			BinalyzeThreshold(processData, pDstImage, th);
			return;
		}
	}
}

// [act]���x�����O���s���Ėʐς��ő�ƂȂ镔���݂̂𒊏o
// [prm]pSrcImage	: 2�l�����s���摜(CV_[XX]C1 -> CV_8UC1�ɓ����ϊ�)
//		pDstImage	: �������ʊi�[��(CV_8UC1)
void CImageProcessor::LaberingMaxSize(const cv::Mat& pSrcImage, cv::Mat& pDstImage){
	// CV_8UC1�ɕϊ����A�摜�T�C�Y�𒊏o
	const cv::Size s = pSrcImage.size();
	const int scale = s.width * s.height;

	// �ϐ���`
	cv::Mat labelMat(s.height + 1, s.width + 1, CV_32SC1, cv::Scalar(0));
	cv::Mat processData(s.height + 1, s.width + 1, CV_8UC1, cv::Scalar(255));
	/* �����l�Ƃ��Č��摜���R�s�[ */{
		cv::Mat temp = processData(cv::Rect(1, 1, s.width, s.height));
		CopyMatRect(pSrcImage, processData,
			cv::Rect(0, 0, s.width, s.height), cv::Rect(1, 1, s.width, s.height));
	}
	std::vector<int> labelCounter;
	// labelNo, labelCount
	std::pair<int, int> maxLabel;

	// ���C������
	std::vector<cv::Point> nonZeroList;
	cv::findNonZero(cv::Scalar(255) - processData, nonZeroList);
	//std::cout << nonZeroList.size() << ", " << scale << std::endl;
	for (std::vector<cv::Point>::iterator it = nonZeroList.begin(); it != nonZeroList.end(); ++it){
		int *const pDataPtr = &labelMat.at<int>(*it);
		const int tempLabel[] = {
			labelMat.at<int>(*it - cv::Point(1,0)), labelMat.at<int>(*it - cv::Point(0,1))
		};
		//std::cout << "(" << tempLabel[0] << ", " << tempLabel[1] << ") " << *it << " " << (int)*pDataPtr << " " ;
		// �Q�Ɖ�f�̍��A��Ƃ����x�����t���Ă��Ȃ�(=0)�̏ꍇ:
		// �V�K���x���ԍ���t�^����
		if (tempLabel[0] == 0 && tempLabel[1] == 0){
			labelCounter.push_back(1);
			*pDataPtr = (int)labelCounter.size();
			if (maxLabel.second == 0)
				maxLabel = std::pair<int, int>(labelCounter.size(), 1);
			continue;
		}
		// �Q�Ɖ�f�̍��A��̃��x�������� or �����ꂩ���t���Ă��Ȃ��ꍇ:
		// ���Ă����ԍ���t�^
		if (tempLabel[0] == tempLabel[1] || !tempLabel[0] || !tempLabel[1]) {
			const int setPos = tempLabel[0] ? 0 : 1;
			*pDataPtr = tempLabel[setPos];
			const int nowCount = ++labelCounter[tempLabel[setPos] - 1];
			maxLabel = maxLabel.second < nowCount ?
				std::pair<int, int>(tempLabel[setPos], nowCount) : maxLabel;
			continue;
		}

		// �Q�Ɖ�f�̍��A��ɕʁX�̃��x�����t���Ă����ꍇ:
		// ���x���ԍ������������ɍ��킹��
		const int moveDir = tempLabel[0] < tempLabel[1] ? 0 : 1;
		*pDataPtr = tempLabel[moveDir];

		// ���x���ԍ��������ւ��郉�x���ʒu��t�^
		// compare elements (if true, set to 255, CV_8U) -> 0or1
		cv::Mat replacePos;
		cv::compare(labelMat, tempLabel[1 - moveDir], replacePos, CV_CMP_EQ);
		replacePos /= 255;

		// type convert to CV_16UC1 and saturate to 2^16
		replacePos.convertTo(replacePos, CV_32SC1);
		// ���x���ԍ��̍����ւ�
		// (2018/01/16)Substract larger number from difference of larger and smaller
		// to make them smaller number. replacePos works as a matrix mask.
		labelMat -= (tempLabel[1-moveDir]-tempLabel[moveDir])*replacePos;

		// ���x�����̉��Z
		labelCounter[tempLabel[moveDir] - 1] += labelCounter[tempLabel[1 - moveDir] - 1];
		labelCounter[tempLabel[1 - moveDir] - 1] = 0;
		const int nowCount = ++labelCounter[tempLabel[moveDir] - 1];
		maxLabel = maxLabel.second < nowCount ?
			std::pair<int, int>(tempLabel[moveDir], nowCount) : maxLabel;
	}
	//for(int i=0; i<labelCounter.size(); ++i)
	//	std::cout << labelCounter[i] << ", ";
	/*std::ofstream ofs("sift.csv");
	ofs << cv::format(out, "csv") << std::endl;*/

	// �[�_������������
	cv::Mat out(labelMat, cv::Rect(1, 1, s.width, s.height));
	// compare elements (if true, set to 255, CV_8U)
	out = out == (maxLabel.first);
	out.convertTo(out, CV_8UC1);
	out.copyTo(pDstImage);
}
// ((false: �k�� / true: �g�� | vector�z��Ŏw��)
/*cv::Mat CImageProcessor::MinimizeExtend(const cv::Mat pBinaryImage, std::queue<bool> pJob){
	return cv::Mat();
}*/

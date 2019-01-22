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

// [act]�Œ肵�����l�ɂ��2�l��
//		臒l�̒l�̉�f�͔���f�ƂȂ�Ӗ��̂Ȃ���f�ƂȂ�B
// [prm]pSrcImage	: 2�l�����s���摜(CV_8UC1 -> CV_8UC1�ɓ����ϊ�)
//		pDstImage	: ��l�����s�����摜(CV_8UC1)
//		pThreshold	: ����f�Ƃ���䗦 range->[0.0, 1.0] ���̒l�𒴂��Ȃ��ő�̔䗦������f�䗦�ƂȂ�
void CImageProcessor::BinalyzeThreshold(const cv::Mat& pSrcImage, cv::Mat& pDstImage, int pThrethold){
	cv::Mat processData = pSrcImage <= pThrethold;
	processData.copyTo(pDstImage);
}

// [act]P�^�C���@�ɂ��2�l��
//		��f�̍ő�l����w��臒l�̗��̉�f������f�ƂȂ�悤��2�l�����s��
// [prm]pSrcImage	: 2�l�����s���摜(CV_[XX]C1 -> CV_8UC1�ɓ����ϊ�)
//		pDstImage	: ��l�����s�����摜(CV_8UC1)
//		pBlackRate	: ����f�Ƃ���䗦 range->[0.0, 1.0]
// [ret]���ۂɏ����o���ꂽ��l�摜�̍���f��(pBlackRate�Ŏw�肳�ꂽ�����̍���f�������̒l�ƂȂ�)
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
// [prm]pSrcImage	: 2�l�����s���摜(CV_8UC1)
//		pDstImage	: �������ʊi�[��(CV_8UC1)
void CImageProcessor::LaberingMaxSize(const cv::Mat& pSrcImage, cv::Mat& pDstImage){
	// CV_8UC1�ɕϊ����A�摜�T�C�Y�𒊏o
	const cv::Size s = pSrcImage.size();
	const int scale = s.width * s.height;

	// �ϐ���`
	cv::Mat processData(s.height + 1, s.width + 1, CV_8UC1, cv::Scalar(255));
	/* �����l�Ƃ��Č��摜���R�s�[ : �I�[�o�[�t���[�h�~ */{
		cv::Mat temp = processData(cv::Rect(1, 1, s.width, s.height));
		CopyMatRect(pSrcImage, processData,
			cv::Rect(0, 0, s.width, s.height), cv::Rect(1, 1, s.width, s.height));
	}

	// LabelData = std::vector<std::pair<cv::Point, int>>: ����f�ʒu, ���x��ID���i�[����ϐ�
	LabelData labelData;
	// �e���x���̍���f�����J�E���g����ϐ��𐶐�
	std::vector<int> labelCounter;
	// �ő僉�x���L�^�p�ϐ��𐶐�(labelNo, labelCount)
	std::pair<int, int> maxLabel;

	// ���C������
	// ����f�̈ʒu���擾���AlabelCounter�ϐ��Ƀ��������m�ۂ��Ă���
	std::vector<cv::Point> nonZeroList;
	cv::findNonZero(cv::Scalar(255) - processData, nonZeroList);
	labelCounter.reserve(nonZeroList.size());

	//std::cout << nonZeroList.size() << ", " << scale << std::endl;
	// ���ׂĂ̍���f�ɑ΂��ď������s��
	for (std::vector<cv::Point>::iterator it = nonZeroList.begin(); it != nonZeroList.end(); ++it){
		int putData = 0;			// ���񊄂蓖�Ă郉�x���ԍ�
		int tempLabel[] = { 0, 0 };	// �Ώۉ�f�̍��E��̉�f�̃��x���ԍ�
		/* tempLabel�̐ݒ�: �Ώۉ�f�̍��E��̉�f������f�����肵�A����f�Ȃ犄�蓖�Ă�ꂽ���x�����擾 */{
			auto nextIt = FindLabelRelative(labelData, *it - cv::Point(1, 0));
			if (nextIt != labelData.rend()) tempLabel[0] = nextIt->second;
			nextIt = FindLabelRelative(labelData, *it - cv::Point(0, 1));
			if (nextIt != labelData.rend()) tempLabel[1] = nextIt->second;
		}
		
		//std::cout << "(" << tempLabel[0] << ", " << tempLabel[1] << ") " << *it << " " << (int)*pDataPtr << " " ;
		// �Q�Ɖ�f�̍��A��Ƃ����x�����t���Ă��Ȃ�(=0)�̏ꍇ:
		// �V�K���x���ԍ���t�^����
		if (tempLabel[0] == 0 && tempLabel[1] == 0){
			labelCounter.push_back(0);
			putData = labelCounter.size();
		}
		// �Q�Ɖ�f�̍��A��̃��x�������� or �����ꂩ���t���Ă��Ȃ��ꍇ:
		// ���Ă����ԍ���t�^
		else if (tempLabel[0] == tempLabel[1] || !tempLabel[0] || !tempLabel[1]) {
			const int setPos = tempLabel[0] ? 0 : 1;
			putData = tempLabel[setPos];
		}

		// �Q�Ɖ�f�̍��A��ɕʁX�̃��x�����t���Ă����ꍇ:
		// ���x���ԍ������������ɍ��킹��
		else {
			const int moveDir = tempLabel[0] < tempLabel[1] ? 0 : 1;
			putData = tempLabel[moveDir];
			RefreshLabel(labelData, tempLabel[1 - moveDir], tempLabel[moveDir], labelCounter);
		}

		// ���x�����̉��Z
		const int nowCount = ++labelCounter[putData - 1];
		maxLabel = maxLabel.second < nowCount ?
			std::pair<int, int>(putData, nowCount) : maxLabel;
		labelData.push_back(std::pair<cv::Point, int>(*it, putData));
	}

	// �[�_�������������������摜��ۑ�
	// �^�����ȉ摜�𐶐�
	cv::Mat out(s.height, s.width, CV_8UC1, cv::Scalar(255));
	// �ő�A�������̃��x���ԍ�������f������f�Ƃ���
	for (auto data : labelData){
		if (data.second != maxLabel.first) continue;
		out.at<unsigned char>(data.first - cv::Point(1, 1)) = 0;	// Black: thread data
	}
	// debug�o�͗p
	/*std::ofstream ofs("sift.csv");
	ofs << cv::format(label, "csv") << std::endl;*/
	// �o�͐�ɔ����𔽓]�����ď����o��: �A�����������F�ŕ\�������
	out = cv::Scalar(255) - out;
	out.convertTo(pDstImage, CV_8UC1);
}

// ���g�p
/*cv::Mat CImageProcessor::MinimizeExtend(const cv::Mat pBinaryImage, std::queue<bool> pJob){
	return cv::Mat();
}*/

// [act]LabelData�����񂳂�Ă���Ɖ��肵�āAFindFor��ΏۂƂ����}�X��data�ɑ��݂��邩��������
//		reverse_iterator���g�p���邱�ƂŔ�r�񐔂̍팸��ڎw���B
// [prm]data	: ��r�Ώۃf�[�^ = ����f���W�ꗗ
//		findFor	: �T���ʒu
// [ret]�f�[�^�̑��݈ʒu : ���݂��Ȃ��ꍇ��rend()
std::vector<std::pair<cv::Point, int>>::reverse_iterator CImageProcessor::FindLabelRelative(LabelData& data, cv::Point findFor){
	// ��r�Ώۃf�[�^�����ׂĒT���͈͂ɂ���
	for (auto it = data.rbegin(); it != data.rend(); ++it){
		// ���o�����f�[�^�̍��W��findFor�ƈ�v����ꍇ: reverse_iterator��Ԃ��ďI��
		if (it->first == findFor) return it;
		// ���o�����f�[�^�̍��W�����X�^�����ɂ����ĒT���ʒu����O�ɂ���ꍇ������Ȃ������Ƃ���rend()��Ԃ��ďI��
		if (it->first.x <= findFor.x && it->first.y <= findFor.y) break;
	}
	// �����܂ł���Ƃ������͌�����Ȃ������Ƃ������Ȃ̂�rend()��Ԃ�
	return data.rend();
}

// [act]���x�����O�ɂ����锻��I����̃��x���ԍ��̍Ċ��蓖�Ă��s��
//		���̃��x����������f���̒l�������ɍX�V����
// [prm]data			: ���x�����O�ɂ����Č��肳�ꂽ���x���f�[�^(��f���ƂɊi�[�����)
//		srcLabel		: �Ċ��蓖�Č��̃��x���f�[�^
//		dstLabel		: �Ċ��蓖�Đ�̃��x���f�[�^
//		pLabelCounter	: �e���x���̍���f�����i�[���ꂽ�f�[�^(���ڍX�V���s��)
void CImageProcessor::RefreshLabel(LabelData& data, int srcLabel, int dstLabel, std::vector<int>& pLabelCounter){
	int count = 0;	// ���x���̓\�芷�����s������f���J�E���^
	// �e��f�ɔ�r��K�p
	for (auto& check : data){
		// �T�������̃��x�����Ċ��蓖�đΏۂȂ烉�x�����Ċ��蓖�āB
		// ���蓖�Đ����Ċ��蓖�Č��̉�f���ɓ��B�������r���I��
		if (check.second == srcLabel){
			check.second = dstLabel;
			if (++count == pLabelCounter[srcLabel - 1]) break;
		}
	}
	// �Ċ��蓖�đO��̃��x�����ݒ肳�ꂽ����f�̐����X�V
	pLabelCounter[dstLabel - 1] += pLabelCounter[srcLabel - 1];
	pLabelCounter[srcLabel - 1] = 0;
}
#include "CThreadDetector.hpp"
#include "CImageProcessor.hpp"
#include "CTimeMeasure.hpp"
#include "CReadConfig.hpp"
#include <fstream>
#include <iomanip>
#include <numeric>

// �萔�ł���f�o�b�O�t�@�C���o�͐���N���X�O�Œ�`����:
const std::string CThreadDetector::mcDebugFolder = "debugImage/";

// [act]�p�����[�^�ƃt�@�C������p����CThreadDetector�N���X�̏��������s���B
//		�������ł̓t���O�̐ݒ肨��юw��t�@�C�����𖼑O�Ɗg���q�ɕ�������B
// [prm]pImageName		: �g���q���܂񂾉摜�ۑ���p�X
//		pIsDebugMode	: debugMode�ŃV�X�e�����N�����邩�ǂ���
//		pIsPhotoMode	: photoMode�ŃV�X�e�����N�����邩�ǂ���
//		pMinisizeFlag	: bgr�`���̉摜�ɂ����ď������摜�T�C�Y�ŏ������s�����ǂ���(true: 320x240, false: 640x480)
//		pConfig			: �܎��؂ꔻ��Ɏg�p����R���t�B�O�ꗗ
// [ret]�Ȃ�
CThreadDetector::CThreadDetector(const std::string pImageName, bool pIsDebugMode, bool pIsPhotoMode, bool pMinisizeFlag, const CReadConfig& pConfig) : 
		// ���������q�ɂ��N���X�萔�����o�̏����� - pConfig����p�����[�^��ǂݏo��
		c_binalyzeRate(pConfig.outputConfig(eBinalyzeThreshold)),
		c_blackRate(pConfig.outputConfig(eBlackRate)),
		c_squareRatio(pConfig.outputConfig(eSquareRatio)),
		c_thickRatio(pConfig.outputConfig(eBendRate)){
	// �t���O�̐ݒ���s��
	mIsDebugMode		= pIsDebugMode;
	mIsPhotoMode		= pIsPhotoMode;
	mMinisizeImageFlag	= pMinisizeFlag;
	// �g���q�̔���: std::string::find()�Ŋg���q���w���h�b�g��T��
	const std::string::size_type dotPos = pImageName.find(".");
	// �g���q��������Ȃ��ꍇ: ���ׂĂ̖��O�������o��
	if(dotPos == std::string::npos)
		mImageFileName = pImageName;
	else {
		// �g���q�����������ꍇ: �t�@�C�����𖼑O�Ɗg���q�ɕ���
		mImageFileName = pImageName.substr(0, dotPos);
		mExtension = pImageName.substr(dotPos);
	}
}

// [act]�w�肳�ꂽ�t�@�C������摜��ǂݍ��ށB
//		�T�|�[�g�͈�ʓI�ȉ摜�t�@�C�������bgr�`��
//		�ǂݍ��񂾉摜���O���[�X�P�[��������܂ł��̊֐����S������
// [prm]dstImage: �o�͉摜(CV_8UC1)
// [ret]�Ǎ��ɐ���������(true:����, false:���s)
bool CThreadDetector::ReadImage(cv::Mat& dstImage){
	if (mExtension == ".bgr"){
		// bgr�`���̏ꍇ
		// �p�����[�^�ɉ����ĉ摜�T�C�Y���w��
		const int width  = 640 / (mMinisizeImageFlag ? 2:1);
		const int height = 480 / (mMinisizeImageFlag ? 2:1);
		const int pixel = width*height, colorNum = 3;
		
		// �t�@�C�����o�C�i���`���œǍ� / �Ǎ����s����false��Ԃ��ďI��
		std::ifstream ifs(mImageFileName + mExtension, std::ios::binary);
		if (!ifs) return false;
		
		// �i�[��cv::Mat����
		dstImage = cv::Mat(height, width, CV_8UC3);
		// �Ǎ��T�C�Y�v�Z
		const int readSize = sizeof(*dstImage.data)*pixel*colorNum;
		// �f�o�b�O�E�摜�{�����[�h�̏ꍇ�t�@�C���T�C�Y�������o��
		if(mIsDebugMode | mIsPhotoMode)
			std::cout << ".bgr dataSize[byte]: " << readSize << std::endl;
		// �t�@�C���̓��e���g�p����摜�̃T�C�Y��cv::Mat::data�\���̂ɒ��ڏ����o��
		// bgr�`����cv::Mat::data�\���̂̃t�H�[�}�b�g�͓���ł���
		// �摜�T�C�Y������ł���΃t�@�C���̂��ׂĂ��ǂݍ��܂�鏈���ƂȂ�
		ifs.read((char*)dstImage.data, readSize);
		// �f�o�b�O�p
		/*std::ofstream ofs("sift.csv");
		ofs << cv::format(dstImage, "csv") << std::endl;*/
	}
	else {
		// bgr�`���ȊO�̏ꍇ
		dstImage = cv::imread(mImageFileName + mExtension);
	}
	// �O���[�X�P�[���摜�ɕϊ� �v�Z���͋P�xY=R*0.299+G*0.587+B*0.114
	// �����I�ɂ̓V�t�g���W�X�^���g�p���Đ����̂܂܏������s���Ă���炵��
	cv::cvtColor(dstImage, dstImage, CV_RGB2GRAY);
	// cv::Mat�^�C�v��CV_8UC1�ɕϊ����ďo�͐�ɏ����o��
	dstImage.convertTo(dstImage, CV_8UC1);
	return true;
}

// [act]�摜�����E�܎��؂ꔻ��̃��C���t���[������
//		�摜�ǂݍ��݁��摜�������܎��؂ꔻ��
// [ret]�܎����؂�Ă��Ȃ��������ǂ���(true:�؂�Ă��Ȃ�, false:�؂�Ă���)
bool CThreadDetector::Detect(){
	CImageProcessor processor;					// �摜�����S���N���X
	cv::Mat procImage;							// �����摜�i�[��
	CTimeMeasure timeManager;					// ���ԊǗ��S���N���X(debugMode/photoMode�Ŏg�p)
	const float binalyzeRate = c_binalyzeRate;	// �摜��l������臒l��`

	// debugMode/photoMode�ł���Ύ��Ԍv���J�n
	if(mIsDebugMode | mIsPhotoMode) timeManager.StartInput();

	// �摜�ǂݍ��݁E�O���[�X�P�[����(�ǂݍ��݂Ɏ��s�����ꍇ�͈܎��؂ꔻ��Ƃ���false��Ԃ�)
	ReadImage(procImage);
	if (procImage.empty()) return false;
	// ���v���Ԃ��L�^����E�ǂݍ��݉摜�������o��(bmp�`��)
	if (mIsDebugMode | mIsPhotoMode){
		timeManager.GoToNextSection("Read Image Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#0.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	// photoMode�Ȃ炱���ŏ��v���Ԃ������o���đҋ@�A�L�[���͂ŏI��
	if (mIsPhotoMode) {
		timeManager.WriteResult();
		cv::imshow("photoViewer : press any key to exit.", procImage);
		cv::waitKey();
		return true;
	}

	// 3x3��Y-prewitt�t�B���^�������摜�c�����̔������s��
	processor.Filter3x3(procImage, procImage, (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1));
	// ���v���Ԃ��L�^����
	if (mIsDebugMode) timeManager.GoToNextSection("Y-Prewitt Filter Time");

	// P-�^�C���@�ɂ���l�����s��
	processor.BinalyzePTile(procImage, procImage, binalyzeRate);
	// ���v���Ԃ��L�^����E��l�摜�������o��(bmp�`��)
	if (mIsDebugMode){
		timeManager.GoToNextSection("P-Tile Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#1.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	// ���x�����O�ɂ��ő�A�������݂̂����o���ăm�C�Y�������s��
	// �����I�ɂ����̉摜�͘A�����������F�ŕ\�������:findNonZero���g�p���邽��
	processor.LaberingMaxSize(procImage, procImage);
	// ���v���Ԃ��L�^����E�m�C�Y������̉摜�������o��(bmp�`��)
	if (mIsDebugMode){
		timeManager.GoToNextSection("Labeling Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#2.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	// ���o�����A�������𔒉�f�Ƃ���
	// �摜��]�␳�y�ш܎��؂ꔻ����s��
	bool ans = CheckThread(procImage, binalyzeRate);
	// ���v���Ԃ��L�^���ăR���\�[���ɏ����o��
	if (mIsDebugMode){
		timeManager.GoToNextSection("Detecting Time");
		timeManager.WriteResult();
	}
	
	// �R���\�[���ɂ����ăf�[�^����؂邽�߂ɉ��s������
	std::cout << "\n";
	
	// ���茋�ʂ�Ԃ�
	return ans;
}

// [act]�摜��]�␳����ш܎��؂ꔻ����s���B
//		�菇�Ƃ��āA�܂������J1���v�Z�����̔�����s���B
//		���ɃA�t�B���ϊ��ɂ��摜��]�␳���s���A�O�ڒ����`�̎l���̓_�̍��W���擾����B
//		���̗��R�͈܎��؂ꔻ��ɂ����ĊO�ڒ����`�֘A�ŕK�v�ƂȂ�Ӓ�X,Y�͏�L�f�[�^�Ō���ł��邽�߂ł���B
//		debugMode�ł̂ݎ��ۂ̉�]�␳���ʂ��摜�ɏo�͂���B
//		���̌�A����ꂽ�O�ڒ����`�̕Ӓ�X,Y�𗘗p���Ĕ����J2�����J3���v�Z���`�󔻒�y�ю֍s������s��
// [prm]pBinaryImage	: ����ΏۂƂȂ��l�摜->���x�����O�ɂ��m�C�Y�������s������l�摜�E�Ӗ��̂����f�͔���f
//		pDefRate		: P-�^�C���@�̓�l��臒l[0.0, 1.0]->P-�^�C���@�ɂ���l�摜�̍���f��NA���v�Z���邽�߂Ɏg�p
// [ret]�܎����؂�Ă��Ȃ��������ǂ���(true:�؂�Ă��Ȃ�, false:�؂�Ă���)
bool CThreadDetector::CheckThread(const cv::Mat& pBinaryImage, const float pDefRate){
	// ����摜�̑傫��[pixel]���擾
	const cv::Size imageSizeData = pBinaryImage.size();
	// ����摜�̖ʐ�[pixel^2]���v�Z
	const float size = static_cast<float>(imageSizeData.width * imageSizeData.height);
	// �摜��]�␳���l�摜�����o���p�ϐ�(debugMode���̂ݑS��f�𔒂Ƃ��ď������E�g�p)
	cv::Mat debugImage;
	if(mIsDebugMode)
		debugImage = cv::Mat(imageSizeData, CV_8UC1, cv::Scalar(255));

	// ��l�摜�̔���f(�L�Ӑ���)�̍��W�ꗗ���擾����
	// nonZeroList�̗v�f���͂��̂܂ܗL�Ӊ�f���ƂȂ�A����f��N�ƂȂ�
	std::vector<cv::Point> nonZeroList;
	cv::findNonZero(pBinaryImage, nonZeroList);
	
	// �����J1�̌v�Z: J1 = N/NA  (�܎���␬���̍���f�� / P-�^�C���@�ɂ���l�摜�̍���f��)
	const float blackNum = static_cast<float>(nonZeroList.size());	// N
	const float blackRate = (blackNum / size) / pDefRate;			// J1 = N/NA
	// �v�Z�����E��������E�����J1�̏����o��
	std::cout << "BinalyzeRate: " << std::setprecision(3) << pDefRate*100.f << "%" << std::endl;
	std::cout << "decreaseRate: " << std::setprecision(3) << blackRate*100.f
		<< "% require: " << std::setprecision(3) << c_blackRate*100.f << "%" << std::endl;
	// �����J1�������𖞂����Ȃ��ꍇ�����ň܎��؂ꔻ����o�͂�����ł��؂�ł���:
	/*if (blackRate < c_squareRatio){
		std::cout << "      result: NG" << std::endl;
		return false;
	}*/

	// ��������摜��]�␳���s���B�܂���]�p�����肷�邽�߂ɘA�������̗��[�ɂ�����y��f�̕��ϒl���v�Z����
	// �v�Z�͍��E���[���璆���Ɍ������ĉ摜�𑖍����A����f�����������n�_�ō���fy���W�̕��ϒl���v�Z�B
	// ���̒l����]�␳���̉�]�p�̊�Ƃ���B
	
	// ��_�i�[��ϐ����`
	cv::Point2f sidePoints[2];
	// �摜�������璆���Ɍ������đ���
	for (int x = 0; x < imageSizeData.width; ++x){
		// �摜�̂���1��̃f�[�^���擾���āAy���W�̕��ϒl���v�Z
		const float yPos = CheckSegment(pBinaryImage.col(x));
		if (yPos < 0) continue;	// ������Ȃ�����(yPos=-1.f)�ꍇ1�E�̗���Q�Ƃ���
		// ����������ϐ��Ɉʒu���L�^����
		sidePoints[0].x = static_cast<float>(x);
		sidePoints[0].y = yPos;
		break;
	}
	// ���l�ɉ摜�E�����璆���Ɍ������đ���
	for (int x = imageSizeData.width-1; x >= 0; --x){
		const float yPos = CheckSegment(pBinaryImage.col(x));
		if (yPos < 0) continue;	// ������Ȃ������ꍇ1���̗���Q�Ƃ���
		sidePoints[1].x = static_cast<float>(x);
		sidePoints[1].y = yPos;
		break;
	}

	// ��]�p���v�Z���邽�߂�arctan���g�p���邪�A���̌v�Z�ɗp�����_2�_��X��������, Y�������������ꂼ��v�Z����B
	const cv::Point2f wholeSize(sidePoints[1].x - sidePoints[0].x, sidePoints[1].y - sidePoints[0].y);
	// arctan�ɂ���]�p�����肷��
	const double wholeAngle = atan2(wholeSize.y, wholeSize.x);
	// ��]�p�ɉ������A�t�B���ϊ��s��𐶐�����
	const cv::Mat affineMat = (cv::Mat_<float>(2,2) <<
		cos(wholeAngle), sin(wholeAngle), -sin(wholeAngle), cos(wholeAngle));
	std::cout << "        line: " << sidePoints[0] << " - " << sidePoints[1]
		<< " / angle(deg): " << wholeAngle * 180.f / CV_PI << std::endl;

	// �O�ڒ����`�̒[�_�̍��W���i�[����ϐ�blackRect���`����: (0:����, 1:�E��)
	cv::Point2f blackRect[2];
	// �e�L�Ӊ�f�̍��W�ɑ΂���]�s���K�p����
	for (std::vector<cv::Point>::iterator it = nonZeroList.begin(); it != nonZeroList.end(); ++it){
		const cv::Mat roundedPos = 	affineMat * (cv::Mat_<float>(2,1) << it->x, it->y);
		// ��]��̍��W���v�Z���A�O�ڒ����`�̍ő�E�ŏ����X�V����Ȃ��blackRect���X�V����
		// ���������[�v����͖������ɒl�̑�����s��
		const cv::Point2f check(roundedPos.at<float>(0), roundedPos.at<float>(1));
		if (it == nonZeroList.begin()){
			blackRect[0] = check; blackRect[1] = check;
		} else {
			blackRect[0].x = check.x < blackRect[0].x ? check.x : blackRect[0].x;
			blackRect[0].y = check.y < blackRect[0].y ? check.y : blackRect[0].y;
			blackRect[1].x = check.x > blackRect[1].x ? check.x : blackRect[1].x;
			blackRect[1].y = check.y > blackRect[1].y ? check.y : blackRect[1].y;
		}
		// debugMode���͉�]��̍��W�ɍ���f��ł�
		if(!mIsDebugMode) continue;
		const cv::Point2i dp(check);
		if(dp.x<0 || dp.x>=imageSizeData.width) continue;
		if(dp.y+50<0 || dp.y+50>=imageSizeData.height) continue;
		debugImage.at<uchar>(dp.y+50, dp.x) = 0;
	}
	
	// debugMode���ɉ摜��]�␳��̓�l�摜���o�͂���
	if (mIsDebugMode)
		 cv::imwrite(mcDebugFolder + mImageFileName + "#3.bmp", debugImage);
	
	// �O�ڒ����`�̕Ӓ�X,Y���v�Z����
	const cv::Point2f len = blackRect[1] - blackRect[0];
	const float ratio = len.x > len.y ? len.x / len.y : len.y / len.x;					// �����J2: max(X,Y)/min(X,Y)
	//const float distance = static_cast<float>(cv::norm(sidePoints[1] - sidePoints[0]));
	//const float expectThick = static_cast<float>(nonZeroList.size()) / distance;
	// 20180131 refact distance: norm->longer length of square (almost equal, norm > longer)
	// equally thick =  shorter / (num of black dot / longer)
	const float expectThick = static_cast<float>(nonZeroList.size()) / (len.x > len.y ? len.x : len.y);
	const float realThick = len.x > len.y ? len.y : len.x;								
	const float thickRatio = len.x * len.y / static_cast<float>(nonZeroList.size());	// �����J3: X*Y/N
	// ���茋�ʏ����o���E���茋�ʂ��o�͂��ďI��
	std::cout << "   convRange: " << std::setprecision(7) << len << std::endl;
	std::cout << "       ratio: " << std::setprecision(3) << ratio
		<< ", require: >=" << std::setprecision(3) << c_squareRatio << std::endl;
	std::cout << "    blackNum: " << std::setprecision(5) << nonZeroList.size() << " /";
	std::cout << " thick: " << "[" << expectThick << ", " << realThick << "]" << std::endl;
	std::cout << "  thickRatio: " << std::setprecision(3) << thickRatio
		<< ", require: <=" << c_thickRatio << std::endl;
	if (ratio < c_squareRatio || blackRate < c_blackRate || thickRatio > c_thickRatio){
		// �܎��؂ꔻ��
		std::cout << "      result: NG" << std::endl;
		return false;
	} else {
		// ���픻��
		std::cout << "      result: OK!" << std::endl;
		return true;
	}
}

// [act]��f�[�^����͂��A���̒��ɔ���f�����邩������s��
//		����f������ꍇ�́A�����̉�f��y���W�̕��ϒl���v�Z���o�͂���
// [prm]pRowData: �摜���ɂ���������1��̃f�[�^(�L�Ӊ�f�͔���f)
// [ret]���͗�ɔ���f���Ȃ������ꍇ: -1.f, ����ȊO: ����f��y���W�̕��ϒl
float CThreadDetector::CheckSegment(const cv::Mat& pRowData){
	// �񒆂̔���f�����J�E���g�E0�Ȃ甒��f���Ȃ��Əo�͂��ďI��
	if(cv::countNonZero(pRowData) == 0) return -1.f;
	// ����f�����v�f�ꗗ���擾����
	std::vector<cv::Point> segmentNonZero;
	cv::findNonZero(pRowData, segmentNonZero);
	// ����f��y���W�̕��ϒl���v�Z����
	// std::accumulate(begin, end, init): �����linit�ɐݒ�v�f�f�[�^�̑����グ���s���B
	// ����f���Ŋ��邱�Ƃŕ��ς��擾���邱�Ƃ��ł���
	const float test = std::accumulate(segmentNonZero.begin(), segmentNonZero.end(),
		cv::Point(0, 0)).y / static_cast<float>(segmentNonZero.size());
	// y���W�̕��ϒl��Ԃ�
	return test;
}

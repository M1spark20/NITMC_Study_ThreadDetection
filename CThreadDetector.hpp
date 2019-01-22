#pragma once
#include <opencv2/opencv.hpp>
#include <string>

// �N���X�̎��O�錾
class CReadConfig;

// [act]�B�e�摜����܎������o����
//		[ReadImage�֐�]	- �摜�ǂݍ��ݕ�
//			1. �t�@�C������J���[�摜��ǂݎ��f�R�[�h����B�Ή��t�H�[�}�b�g�̓��W���[�Ȃ���+bgr�`���ł���B
//			2. �J���[�摜�����m�N���摜�ɕϊ�����
//		[Detect�֐�] - �摜������CImageProcessor�N���X�ɂčs����������CheckThread�֐��Ƀf�[�^��n���܎��؂ꔻ����s���B��̓I�ɂ�:
//			1. Y-prewitt�t�B���^�ɂ��摜�������s���A�摜�c�����̃G�b�W�����o����B
//			2. P-�^�C���@�ɂ������摜�̓�l�����s���B�Ӗ��̂����f�͍���f�ŕ\������B
//			3. ��l�摜�̍���f��4�ߖT�̘A���������Ƃɕ����A�ʐς��ő�̂��݂̂̂����o���B
//			4. �܎��̑������Z�o�����߂ɉ摜��]�␳���s���B�␳��̘A���������܎���␬���ƌĂԁB
//		[CheckThread�֐�] - ������̈܎������݂��邩���ȉ���3�̊�Ŕ��肷��B
//			1. �m�C�Y�����O��̓�l�摜�̍���f�����r���A�m�C�Y������Ɏc��������f���̊������Z�o����: ���̔���J1
//			2. �܎���␬���̊O�ڒ����`�̏c������Z�o����: �`�󔻒�J2
//			3. �܎���␬���̍���f���ƊO�ڒ����`�̖ʐς̔���Z�o����: �֍s����J3
class CThreadDetector{
	cv::Mat mProcessImage;					// �摜�f�[�^�i�[��
	std::string mImageFileName;				// �ǂݍ��݃t�@�C����
	std::string mExtension;					// �ǂݍ��݃t�@�C���g���q(�R���X�g���N�^�Ŕ����o��)
	static const std::string mcDebugFolder;	// �f�o�b�O�p�摜�o�͐�
	bool mIsDebugMode;						// �f�o�b�O���[�h�t���O(�����r���摜�̏����o���E�������Ԃ̏����o�����s��)
	bool mIsPhotoMode;						// �摜�{�����[�h�t���O(�t�@�C����ǂݍ���ŕ\�����邾���̃��[�h)
	bool mMinisizeImageFlag;				// ���T�C�Y�摜�t���O(�ǂݍ��މ摜�̃T�C�Y���w��: true-320x240, false-640x480)
	
	// �摜�����E�܎��؂ꔻ��p�����[�^�ꗗ(param.cfg���w��)
	const double c_binalyzeRate;			// P-�^�C���@�ɂ���l�����̍���f���̊���
	const double c_blackRate;				// �����J1�ɂ�����臒l: ������΍����قǃm�C�Y�ʂ����Ȃ�
	const double c_squareRatio;				// �����J2�ɂ�����臒l: ������΍����قǊO�ڒ����`�̏c���䂪�傫��
	const double c_thickRatio;				// �����J3�ɂ�����臒l: �Ⴏ��ΒႢ�قǈ܎��͒�����ł���

	// [act]�ϐ�mImageFileName��p���ăt�@�C���ǂݍ��݂��s��
	bool ReadImage(cv::Mat& dstImage);
	
	// [act]�摜�������{������l�摜��]�����܎��؂ꔻ����s���B
	bool CheckThread(const cv::Mat& pBinaryImage, const float pDefRate);
	
	// [act]��f�[�^����͂��A���̒��ɔ���f�����邩������s��
	float CheckSegment(const cv::Mat& pRowData);
public:
	// �R���X�g���N�^
	CThreadDetector(const std::string pImageName, bool pIsDebugMode, bool pIsPhotoMode, bool pMinisizeFlag, const CReadConfig& pConfig);
	
	// [act]�摜��]�␳����ш܎��؂ꔻ����s��
	bool Detect();
};

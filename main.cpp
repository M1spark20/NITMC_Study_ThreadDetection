#include <iostream>
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "CThreadDetector.hpp"
#include "CReadConfig.hpp"

using namespace cv;
// cmd�ɂ��V�X�e���̋N�����@(�����̃t�@�C������x�ɉ�͂��邱�Ƃ��ł���):
// ���s�t�@�C���� �R���t�B�O�t�@�C���� �N�����[�h�p�����[�^(-d/-p) �T�C�Y�p�����[�^1(-s,����̃t�@�C���ɂ̂ݓK�p) �ǂݍ��݃t�@�C����1 �T�C�Y�p�����[�^2(-s) �ǂݍ��݃t�@�C����2 ...
// [ret]0�ȏ�: �܎��؂�Ɣ��肳�ꂽ�摜��(1���̏ꍇ0�Ȃ琳��A1�Ȃ�܎��؂�)
//		   -1: �p�����[�^�ݒ�~�X���ُ̈�I��
int main(int argc, char** argv)
{
	int dataStart = 1;				// ���݂̃R�}���h���C���p�����[�^�ǂݍ��݈ʒu
	bool debugMode		 = false;	// debugMode�ŋN�����邩�̃t���O(-d)
	bool photoMode		 = false;	// photoMode�ŋN�����邩�̃t���O(-p)
	bool minisizeFlag	 = false;	// bgr�`���̃t�@�C�����g�p����ۏ������T�C�Y�œǂݍ��ނ��̃t���O(-s)
	
	if(argc == 1){
		std::cout << "Please set config file path as 1st cmd parameter." << std::endl;
		return -1;
	}
	
	// �R���t�B�O�ǂݍ���
	std::string arg1(argv[dataStart]);
	CReadConfig config(arg1);
	if(!config.readFile()){
		std::cout << "Failed to read all configurations." << std::endl;
		return -1;
	}
	++dataStart;
	
	arg1 = std::string(argv[dataStart]);
	if (arg1 == "-d"){
		// -d param: �w�肷��ƃV�X�e�����f�o�b�O���[�h�ŋN������B�����r���̉摜�̏����o���Ǝ��Ԍv�����s����
		std::cout << "Launch debug mode." << std::endl;
		debugMode = true; ++dataStart;
	} else if (arg1 == "-p"){
		// -p param: �w�肷��ƃV�X�e�����t�H�g���[�h�ŋN������B�摜����t�@�C����ǂݍ��ݕۑ��E�E�B���h�E�ɏo�͂��ďI������B
		// �t�@�C����ǂݍ��ނ����ŉ摜������܎��؂ꔻ��͍s���Ȃ��B
		std::cout << "Launch photoView mode." << std::endl;
		photoMode = true; ++dataStart;
	}

	// �����ŃR�}���h���C�����I���ꍇ�摜�t�@�C�������w�肳��Ă��Ȃ��̂Ŏw��𑣂�
	if (argc == dataStart){
		std::cout << "Please set imageName as cmd parameter." << std::endl;
		return -1;
	}

	int result = 0;
	// �w�肳�ꂽ�e�摜�ɑ΂��Ĉ܎��؂ꌟ�m���s���B
	// bgr�f�[�^�̏ꍇ�A�e�t�@�C�����̒��O��-s�I�v�V���������邱�Ƃ�320x240�T�C�Y�̉摜��ǂݍ���(�f�t�H���g��640x480)
	for(; dataStart<argc; ++dataStart){
		const std::string data(argv[dataStart]);
		if (data == "-s"){ minisizeFlag = true; continue; }
		std::cout << data << std::endl;
		CThreadDetector detector(data, debugMode, photoMode, minisizeFlag, config);
		result = detector.Detect() ? result : result+1;
		minisizeFlag = false;
	}
	return result;
}

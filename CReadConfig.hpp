#pragma once
#include <string>

// �����^�p ���錾
class CImageProcessor;
class CThreadDetector;

// [act]�R���t�B�O�t�@�C���Őݒ肷��R���t�B�O�ꗗ
enum EReadConfigList{
	eBinalyzeThreshold,	// P-�^�C���@�ɂ���l������臒l(0-100[%])
	eBlackRate,			// ����1: ���̔���臒l(�l�ȏォ)
	eSquareRatio,		// ����2: �`�󔻒�臒l(�l�ȏォ)
	eBendRate,			// ����3: �֍s����臒l(�l������)
	eConfigListMax		// �v�f�̍ő�l(�z��ϐ������Ɏg�p)
};

// [act]1. �R���t�B�O�t�@�C������R���t�B�O��ǂݎ��
//		2. �e�摜�����N���X�ɕK�v�ȃR���t�B�O��ǂݏo��
class CReadConfig{
	double m_readingList[eConfigListMax];	// �ǂ݂������R���t�B�O���i�[����z��ϐ�
	const std::string m_configFileName;		// �ǂ݂����R���t�B�O�̃t�@�C����: �R���X�g���N�^�Ŏw��
public:
	// [act]�R���X�g���N�^ �O������ǂ݂����t�@�C�����ł���m_configFileName�̎w����󂯂�B
	// [prm]pFileName	: �R���t�B�O�t�@�C����
	CReadConfig(std::string pFileName) : m_configFileName(pFileName){};
	// [act]�R���t�B�O�t�@�C������l��ǂݏo���B���ׂẴR���t�B�O��ǂݏo���Ȃ������ꍇ�G���[�I������B
	bool readFile();
	// [act]�w�����ꂽ�R���t�B�O�̒l�����o��
	double outputConfig(const EReadConfigList pList) const;
};

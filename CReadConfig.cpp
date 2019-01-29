#include "CReadConfig.hpp"
#include "CImageProcessor.hpp"
#include "CThreadDetector.hpp"
#include <fstream>
#include <sstream>
#include <bitset>
#include <iostream>

// [act]�R���t�B�O�t�@�C������l��ǂݏo���B���ׂẴR���t�B�O��ǂݏo���Ȃ������ꍇ�G���[�I������B
// [ret]���ׂẴR���t�B�O���ڂ��������ǂݍ��܂ꂽ��
bool CReadConfig::readFile(){
	// �t�@�C�����w�肳��Ă��Ȃ��E�t�@�C��������ɓǂݍ��߂Ȃ��ꍇfalse��Ԃ��ďI������
	if(m_configFileName.empty()) return false;
	std::ifstream ifs(m_configFileName);
	if (!ifs){
		std::cout << "File \"" << m_configFileName << "\" not found." << std::endl;
		return false;
	}
	// �t�@�C������̕�����Ǎ���ϐ�
	std::string readString;		
	// �R���t�B�O��ǂݍ��񂾂��̃t���O: �ǂݍ��ރR���t�B�O�̐��Ԃ񐶐�����
	std::bitset<eConfigListMax> checker;	// initial: all "false"
	// �ǂݍ��ރR���t�B�O�̃��x�����`����
	const std::string label[eConfigListMax] = {
		"BinalyzeThreshold",
		"Judge1_BlackRate",
		"Judge2_SquareRatio",
		"Judge3_BendRatio",
	};
	// �t�@�C����1�s����������
	while(!ifs.eof()){
		std::getline(ifs,readString);
		for(int i=0; i<eConfigListMax; ++i){
			// �ǂݍ��ݕ����񂩂烉�x����=�ŋ�؂��ēǂݏo��
			const std::string::size_type eqPos = readString.find("=");
			const std::string nowLabel = readString.substr(0, eqPos);
			// ���x�����ǂݍ��ނ��̂Ɋ܂܂�Ă���΁A���̒l���擾���ǂݍ��݃t���O�𗧂Ă�
			if(label[i] != nowLabel) continue;
			std::istringstream isstream(readString.substr(eqPos+1));
			isstream >> m_readingList[i];
			checker.set(i);
		}
	}
	// �S�R���t�B�O���ݒ肳��ĂȂ����false��Ԃ� ���̎��W���o�͂œǂ߂Ȃ������R���t�B�O���o�͂���B
	if(checker.count() < eConfigListMax){
		for(unsigned int i=0; i<checker.size(); ++i)
			if(!checker[i]) std::cout << "Failed to read config: " << label[i] << std::endl;
		return false;
	}
	// �S�R���t�B�O���ݒ肳��Ă����true��Ԃ�
	return true;
}

// [act]�w�����ꂽ�R���t�B�O�̒l�����o��
// [prm]pList	: ���o���R���t�B�O�̎�ނ��w��
// [ret]�w�肵���R���t�B�O�̒l
double CReadConfig::outputConfig(const EReadConfigList pList) const{
	return m_readingList[pList];
}

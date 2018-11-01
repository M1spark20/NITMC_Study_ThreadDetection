#include "CReadConfig.hpp"
#include "CImageProcessor.hpp"
#include "CThreadDetector.hpp"
#include <fstream>
#include <sstream>
#include <bitset>
#include <iostream>

bool CReadConfig::readFile(){
	if(m_configFileName.empty()) return false;
	std::ifstream ifs(m_configFileName);
	if (!ifs){
		std::cout << "File \"" << m_configFileName << "\" not found." << std::endl;
		return false;
	}
	std::string readString;		// initial: all "0"
	std::bitset<eConfigListMax> checker;
	const std::string label[eConfigListMax] = {
		"BinalyzeThreshold",
		"Judge1_BlackRate",
		"Judge2_SquareRatio",
		"Judge3_BendRatio",
	};
	while(!ifs.eof()){
		std::getline(ifs,readString);
		for(int i=0; i<eConfigListMax; ++i){
			const std::string::size_type eqPos = readString.find("=");
			const std::string nowLabel = readString.substr(0, eqPos);
			if(label[i] != nowLabel) continue;
			std::istringstream isstream(readString.substr(eqPos+1));
			isstream >> m_readingList[i];
			checker.set(i);
		}
	}
	// 全コンフィグが設定されてなければfalseを返す この時標準出力で読めなかったコンフィグを出力する。
	if(checker.count() < eConfigListMax){
		for(unsigned int i=0; i<checker.size(); ++i)
			if(!checker[i]) std::cout << "Failed to read config: " << label[i] << std::endl;
		return false;
	}
	// 全コンフィグが設定されていればtrueを返す
	return true;
}

double CReadConfig::outputConfig(const EReadConfigList pList) const{
	return m_readingList[pList];
}

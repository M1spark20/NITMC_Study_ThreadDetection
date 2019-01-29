#include "CReadConfig.hpp"
#include "CImageProcessor.hpp"
#include "CThreadDetector.hpp"
#include <fstream>
#include <sstream>
#include <bitset>
#include <iostream>

// [act]コンフィグファイルから値を読み出す。すべてのコンフィグを読み出せなかった場合エラー終了する。
// [ret]すべてのコンフィグ項目が正しく読み込まれたか
bool CReadConfig::readFile(){
	// ファイルが指定されていない・ファイルが正常に読み込めない場合falseを返して終了する
	if(m_configFileName.empty()) return false;
	std::ifstream ifs(m_configFileName);
	if (!ifs){
		std::cout << "File \"" << m_configFileName << "\" not found." << std::endl;
		return false;
	}
	// ファイルからの文字列読込先変数
	std::string readString;		
	// コンフィグを読み込んだかのフラグ: 読み込むコンフィグの数ぶん生成する
	std::bitset<eConfigListMax> checker;	// initial: all "false"
	// 読み込むコンフィグのラベルを定義する
	const std::string label[eConfigListMax] = {
		"BinalyzeThreshold",
		"Judge1_BlackRate",
		"Judge2_SquareRatio",
		"Judge3_BendRatio",
	};
	// ファイルを1行ずつ処理する
	while(!ifs.eof()){
		std::getline(ifs,readString);
		for(int i=0; i<eConfigListMax; ++i){
			// 読み込み文字列からラベルを=で区切って読み出す
			const std::string::size_type eqPos = readString.find("=");
			const std::string nowLabel = readString.substr(0, eqPos);
			// ラベルが読み込むものに含まれていれば、その値を取得し読み込みフラグを立てる
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

// [act]指示されたコンフィグの値を取り出す
// [prm]pList	: 取り出すコンフィグの種類を指定
// [ret]指定したコンフィグの値
double CReadConfig::outputConfig(const EReadConfigList pList) const{
	return m_readingList[pList];
}

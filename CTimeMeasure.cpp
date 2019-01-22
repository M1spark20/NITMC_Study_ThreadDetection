#include "CTimeMeasure.hpp"
#include <iostream>

// コンストラクタ・デストラクタでは何もしない
CTimeMeasure::CTimeMeasure(){}
CTimeMeasure::~CTimeMeasure(){}

// [act]所要時間の測定を開始する
//		既に開始している測定がある場合は、開始時間を上書きして新たに開始する
void CTimeMeasure::StartInput(){
	if (m_buffer.empty())
		m_buffer.push_back(std::chrono::system_clock::now());
	else
		*m_buffer.rbegin() = std::chrono::system_clock::now();
}

// [act]所要時間の測定を終了し新たに開始する、終了した測定に名前を付ける
// [prm]pTagName	: 終了した測定に付ける名前
void CTimeMeasure::GoToNextSection(std::string pTagName){
	m_tagName.push_back(pTagName);
	m_buffer.push_back(std::chrono::system_clock::now());
}

// [act]記録したすべての測定結果と総計時間を書き出す
void CTimeMeasure::WriteResult(){
	if (m_buffer.size() <= 1) return;
	std::cout << "--------------------------------------------------\n";
	std::vector<std::chrono::system_clock::time_point>::iterator itTime;
	std::vector<std::string>::iterator itTag;
	long long totalDuration = 0;
	for (itTime = m_buffer.begin() + 1, itTag = m_tagName.begin(); itTime != m_buffer.end(); ++itTime, ++itTag){
		if (*itTag == "") continue;
		const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(*itTime - *(itTime - 1)).count();
		totalDuration += duration;
		std::cout << *itTag << ": " << duration << "[ms]\n";
	}
	std::cout << "Total Time: " << totalDuration << "[ms]\n";
	std::cout << "--------------------------------------------------\n";
}

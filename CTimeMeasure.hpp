#pragma once
#include <chrono>	// available after C++11
#include <string>
#include <vector>

// [act]システムの所要時間を計測する
class CTimeMeasure
{
public:
	CTimeMeasure();
	~CTimeMeasure();
	
	// [act]所要時間の測定を開始する
	void StartInput();
	
	// [act]所要時間の測定を終了し新たに開始する、終了した測定に名前を付ける
	void GoToNextSection(std::string pTagName);
	
	// [act]記録したすべての測定結果と総計時間を書き出す
	void WriteResult();

private:
	std::vector<std::chrono::system_clock::time_point>	m_buffer;	// 入力開始・終了点の時間を格納する変数
	std::vector<std::string>							m_tagName;	// 測定の名前を格納する変数
};

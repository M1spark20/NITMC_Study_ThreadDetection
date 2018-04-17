#pragma once
#include <chrono>
#include <string>
#include <vector>

class CTimeMeasure
{
public:
	CTimeMeasure();
	~CTimeMeasure();
	void StartInput();
	void GoToNextSection(std::string pTagName);
	void WriteResult();

private:
	std::vector<std::chrono::system_clock::time_point>	m_buffer;
	std::vector<std::string>							m_tagName;
};

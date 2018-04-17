#include "CTimeMeasure.hpp"
#include <iostream>

CTimeMeasure::CTimeMeasure(){}
CTimeMeasure::~CTimeMeasure(){}

void CTimeMeasure::StartInput(){
	if (m_buffer.empty())
		m_buffer.push_back(std::chrono::system_clock::now());
	else
		*m_buffer.rbegin() = std::chrono::system_clock::now();
}

void CTimeMeasure::GoToNextSection(std::string pTagName){
	m_tagName.push_back(pTagName);
	m_buffer.push_back(std::chrono::system_clock::now());
}

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

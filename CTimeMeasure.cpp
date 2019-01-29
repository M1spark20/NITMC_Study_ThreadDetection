#include "CTimeMeasure.hpp"
#include <iostream>

// �R���X�g���N�^�E�f�X�g���N�^�ł͉������Ȃ�
CTimeMeasure::CTimeMeasure(){}
CTimeMeasure::~CTimeMeasure(){}

// [act]���v���Ԃ̑�����J�n����
//		���ɊJ�n���Ă��鑪�肪����ꍇ�́A�J�n���Ԃ��㏑�����ĐV���ɊJ�n����
void CTimeMeasure::StartInput(){
	if (m_buffer.empty())
		m_buffer.push_back(std::chrono::system_clock::now());
	else
		*m_buffer.rbegin() = std::chrono::system_clock::now();
}

// [act]���v���Ԃ̑�����I�����V���ɊJ�n����A�I����������ɖ��O��t����
// [prm]pTagName	: �I����������ɕt���閼�O
void CTimeMeasure::GoToNextSection(std::string pTagName){
	m_tagName.push_back(pTagName);
	m_buffer.push_back(std::chrono::system_clock::now());
}

// [act]�L�^�������ׂĂ̑��茋�ʂƑ��v���Ԃ������o��
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

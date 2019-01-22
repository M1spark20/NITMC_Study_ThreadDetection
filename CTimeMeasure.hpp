#pragma once
#include <chrono>	// available after C++11
#include <string>
#include <vector>

// [act]�V�X�e���̏��v���Ԃ��v������
class CTimeMeasure
{
public:
	CTimeMeasure();
	~CTimeMeasure();
	
	// [act]���v���Ԃ̑�����J�n����
	void StartInput();
	
	// [act]���v���Ԃ̑�����I�����V���ɊJ�n����A�I����������ɖ��O��t����
	void GoToNextSection(std::string pTagName);
	
	// [act]�L�^�������ׂĂ̑��茋�ʂƑ��v���Ԃ������o��
	void WriteResult();

private:
	std::vector<std::chrono::system_clock::time_point>	m_buffer;	// ���͊J�n�E�I���_�̎��Ԃ��i�[����ϐ�
	std::vector<std::string>							m_tagName;	// ����̖��O���i�[����ϐ�
};

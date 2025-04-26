#pragma once

#include "IDSTrace16.h"

class IDSChannelBlob
{
public:
	IDSChannelBlob()
	{
		m_iDataVersion = 16;
		m_iSample      = 512;
		m_iTraceCount  = 0;
	};
	~IDSChannelBlob()
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, IDSTrace16*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			IDSTrace16* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	};

	/*
	* Fun:����ͨ�����ݲ���
	* Param:
	*     int iVersion     trace���ݵ�λ��,16λ��32λ
	*     int iSample      ÿ��trace����������
	*     int iTraceCount  Trace����
	*/
	void setChannelParam(int iVersion, int iSample, int iTraceCount)
	{
		m_iDataVersion = iVersion;
		m_iSample      = iSample;
		m_iTraceCount  = iTraceCount;
	};

	//���һ��16λTrace
	int addTrace16(IDSTrace16 *trace)
	{
		m_iTraceCount++;
		m_lstData.insert(std::pair<long, IDSTrace16*>(m_iTraceCount, trace));

		return 0;
	};

	//��ȡtrace����
	std::map<long, IDSTrace16*> *getTraceList()
	{
		return &m_lstData;
	};

private:
	int m_iDataVersion = 16;   //trace���ݵ�λ��
	int m_iSample      = 0;    //һ��trace�е���������
	int m_iTraceCount  = 0;    //һ��ͨ����Trace������

	std::map<long, IDSTrace16*> m_lstData;
	char m_FileName[1024] = { 0 };
};
 
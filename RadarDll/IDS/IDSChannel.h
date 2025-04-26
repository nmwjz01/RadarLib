#pragma once

#include "IDSChannelHeader.h"
#include "IDSChannelBlob.h"

class IDSChannel
{
public:
	IDSChannel() {};
	~IDSChannel() {};

	int setChannelParam(int iVersion, int iSample, int iTraceCount)
	{
		//���ÿ����
		m_oBlob.setChannelParam(iVersion, iSample, iTraceCount);

		return 0;
	};

	//����ID
	void setID( int id )
	{
		m_iNo = id;
	};
	int getID()
	{
		return m_iNo;
	}

	//��ͨ�������trace����
	void appendData(IDSTrace16 *trace)
	{
		//Trace������һ
		int traceCount = m_oHeader.GetTraceCount();
		traceCount++;
		m_oHeader.SetTraceCount(traceCount);

		//���Trace����
		m_oBlob.addTrace16(trace);
	};

	//����ͨ��ͷ��Ϣ
	IDSChannelHeader * getHeader()
	{
		return &m_oHeader;
	};

	//����ͨ�����ݿ�
	IDSChannelBlob * getBlob()
	{
		return &m_oBlob;
	};

private:
	IDSChannelHeader m_oHeader;
	IDSChannelBlob   m_oBlob;
	int m_iNo      = 0;

};

#pragma once

#include "MalaChannelHeader.h"
#include "MalaChannelBlob.h"

class MalaChannel
{
public:
	MalaChannel() {};
	~MalaChannel() {};

	int setChannelParam(int iVersion, int iSample, int iTraceCount)
	{
		//���ÿ����
		m_oBlob.setChannelParam(iVersion, iSample, iTraceCount);

		return 0;
	};

	//����ͨ��ͷ�ļ��������ļ�����ʼ��һ��ͨ������
	int init(char *channelHeader, char *channelBlob, int bitFlag)
	{
		//�����Ϸ���У��
		if ((!channelHeader) || (!channelBlob))
			return 1;
		if ((0 == strlen(channelHeader)) || (0 == strlen(channelBlob)))
			return 2;

		//���ļ��ж�ȡͷ��Ϣ
		int result = m_oHeader.init(channelHeader);
		if (result)
			return result;

		//���ļ��ж�ȡ������Ϣ
		return m_oBlob.init(channelBlob, bitFlag, m_oHeader.GetSample(), m_oHeader.GetTraceCount());
	}

	//����ID
	void setID(int id)
	{
		m_iNo = id;
	};
	int getID()
	{
		return m_iNo;
	};

	//����ͨ��ͷ��Ϣ
	MalaChannelHeader * getHeader()
	{
		return &m_oHeader;
	};

	//����ͨ�����ݿ�
	MalaChannelBlob * getBlob()
	{
		return &m_oBlob;
	};

private:
	MalaChannelHeader m_oHeader;
	MalaChannelBlob   m_oBlob;
	int m_iNo = 0;
};

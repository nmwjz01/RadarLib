#pragma once


//#include "Impluse\\ImpluseChannelHeader.h"
//#include "Impluse\\ImpluseChannelBlob.h"

class SwathChannel
{
public:
	SwathChannel(){}
	~SwathChannel(){}

	/*
	* Fun:   ��ʼ��ͨ������
	* Param: char *pIPRB ͨ��������
	*        char *pIPRH ͨ����ͷ
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	int init( char *pIPRB , char *pIPRH )
	{
		int iResult = 0;

		//��ʼ����ͷ
		iResult = m_oHeader.init(pIPRH);
		if (0 != iResult)
			return iResult;

		//��ʼ��������
		iResult = m_oBlob.init(pIPRB);
		if (0 != iResult)
			return iResult;

		//���ÿ����
		m_oBlob.setChannelParam(m_oHeader.getDataVersion() , m_oHeader.getSample() , m_oHeader.getTraceCount() );

		return 0;
	}

	/*
	* Fun:   ����ͨ��ID
	* Param: int iNo ͨ��ID
	* Return:��
	*/
	void setNo( int iNo )
	{
		m_iNo = iNo;
	}

	/*
	* Fun:   ��ȡͨ��ID
	* Param: ��
	* Return:ͨ��ID
	*/
	int getNo()
	{
		return m_iNo;
	}

	/*
	* Fun:��ÿ����
	* Return:�ɹ����ؿ����ʧ�ܷ���NULL
	*/
	SwathChannelBlob* getChannelBlob()
	{
		return &m_oBlob;
	}

	/*
	* Fun:��ÿ�ͷ
	* Return:�ɹ����ؿ�ͷ��ʧ�ܷ���NULL
	*/
	SwathChannelHeader* getChannelHeader()
	{
		return &m_oHeader;
	}

private:
	SwathChannelHeader m_oHeader;
	SwathChannelBlob   m_oBlob;
	int m_iNo = 0;
};

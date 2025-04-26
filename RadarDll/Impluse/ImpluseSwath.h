#pragma once

//#include "ImpluseCor.h"
//#include "ImpluseTime.h"
//#include "ImpluseChannel.h"

class Swath
{
public:
	Swath(){}
	~Swath()
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, SwathChannel*>::iterator iter = m_lstSwathChannel.begin(); iter != m_lstSwathChannel.end(); iter++)
		{
			SwathChannel* p = iter->second;
			delete p;
		}
		m_lstSwathChannel.clear();
	}

	/*
	* Fun:   ��ʼ������
	* Param: char* pPath   �����ļ�·��
	*        char *pSwathID����ID��������
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	int init(char* pPath, char *pSwathID)
	{
		int iResult = 0;
		char szPathFile[512]     = { 0 };
		char szPathFileIPRB[512] = { 0 };
		char szPathFileIPRH[512] = { 0 };

		//��¼����ID
		if (128 <= strlen(pSwathID))
			strncpy(m_szSwathID, pSwathID, 128);
		else
			strcpy(m_szSwathID, pSwathID);

		//��װcor�ļ���·�����ļ�����Ȼ���ʼ��Cor����
		sprintf(szPathFile, "%s\\%s.cor", pPath, pSwathID);
		iResult = m_oSwathCor.init( szPathFile );
		if (0 != iResult)
			printf( "init cor failed" );
		//	return iResult;

		//��װtime�ļ���·�����ļ�����Ȼ���ʼ��Time����
		sprintf(szPathFile, "%s\\%s.time", pPath, pSwathID);
		iResult = m_oSwathTime.init( szPathFile );
		if (0 != iResult)
			printf("init time failed");
			//return iResult;

		//�����ʼ����Ӧ��SwathChannel��Ȼ�󽫵õ���SwathChannel����map
		int i = 1;
		while( true )
		{
			//��װĿ¼�µ�.iprb
			sprintf(szPathFileIPRB, "%s\\%s_A%02d.iprb", pPath, pSwathID , i );
			if (!Utils::checkExistFile(szPathFileIPRB))
				break;

			//��װĿ¼�µ�.iprh
			sprintf(szPathFileIPRH, "%s\\%s_A%02d.iprh", pPath, pSwathID , i );
			if (!Utils::checkExistFile(szPathFileIPRH))
				break;

			SwathChannel* pSwathChannel = new SwathChannel();
			iResult = pSwathChannel->init(szPathFileIPRB, szPathFileIPRH);
			if (0 != iResult)
			{
				delete pSwathChannel;
				break;
			}

			//����ͨ��ID
			pSwathChannel->setNo( i );

			m_lstSwathChannel.insert(std::pair<int, SwathChannel*>( i , pSwathChannel ));

			i++;
		}

		return 0;
	}

	/*
	* Fun:   ͨ��ͨ��ID��ȡͨ������
	* Param: int iChannelID ͨ��ID
	* Return:�ɹ�����ͨ������ʧ�ܷ���NULL
	*/
	SwathChannel*getChannel( int iChannelID )
	{
		return m_lstSwathChannel[iChannelID];
	}

	/*
	* Fun:   ��ȡͨ������
	* Param: int& iCount ���������ͨ������
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	int getChannelCount( int &iCount )
	{
		iCount = ( int )m_lstSwathChannel.size();
		return 0;
	}

	/*
	* Fun:   ��ȡ����ID
	* Param: ��
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	char* getSwathID()
	{
		return m_szSwathID;
	}

	/*
	* Fun:��ȡ���ߵ�ʱ������Ϣ
	* Param:��
	* Return:�ɹ�����SwathTime����ʧ�ܷ���NULL
	*/
	SwathTime * getSwathTime()
	{
		return &m_oSwathTime;
	}

	/*
	* Fun:��ȡ���ߵ�λ�ü����Ϣ
	* Param:��
	* Return:�ɹ�����SwathCor����ʧ�ܷ���NULL
	*/
	SwathCor * getSwathCor()
	{
		return &m_oSwathCor;
	}

	//ͨ��Trace������ʱ�� ��ȡ Trace��
	int getTraceNumByTime(char *szTraceTime)
	{
		return m_oSwathTime.getTraceNumByTime(szTraceTime);
	}

private:
	char m_szSwathID[128] = { 0 };
	SwathCor  m_oSwathCor;
	SwathTime m_oSwathTime;

	std::map<int, SwathChannel*> m_lstSwathChannel;
};

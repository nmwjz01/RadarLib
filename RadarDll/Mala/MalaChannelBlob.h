#pragma once

#include "MalaTrace16.h"
#include "MalaTrace32.h"

class MalaChannelBlob
{
public:
	MalaChannelBlob()
	{
		m_iDataVersion = 16;
		m_iSample      = 512;
		m_iTraceCount  = 0;
	};
	~MalaChannelBlob()
	{
		//�ͷ����з���Ŀռ�--16λ
		for (std::map<long, MalaTrace16*>::iterator iter16 = m_lstData16.begin(); iter16 != m_lstData16.end(); iter16++)
		{
			MalaTrace16* p = iter16->second;
			delete p;
		}
		m_lstData16.clear();

		//�ͷ����з���Ŀռ�--32λ
		for (std::map<long, MalaTrace32*>::iterator iter32 = m_lstData32.begin(); iter32 != m_lstData32.end(); iter32++)
		{
			MalaTrace32* p = iter32->second;
			delete p;
		}
		m_lstData32.clear();
	};

	//����ͨ�������ļ���ʼ��ͨ�����ݶ���
	int init( char *pathFile, int bitFlag, int iSample, int iTraceCount )
	{
		//�����Ϸ����ж�
		if (!pathFile)
			return 1;
		if (!strlen(pathFile))
			return 1;

		//��¼blob�ļ���
		strcpy(m_FileName, pathFile);

		m_iDataVersion = bitFlag;		//blob��32λ����16λ
		m_iSample      = iSample;       //ÿTrace�Ĳ�������
		m_iTraceCount  = iTraceCount;   //Trace����

		//����λ������trace����
		if (16 == m_iDataVersion)
		{
			return loadTrace16();
		}
		else if (32 == m_iDataVersion)
		{
			return loadTrace32();
		}

		return 2;
	}

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

	//��ȡtrace����--16λ
	std::map<long, MalaTrace16*> *getTrace16List()
	{
		return &m_lstData16;
	};
	//��ȡtrace����--32λ
	std::map<long, MalaTrace32*> *getTrace32List()
	{
		return &m_lstData32;
	};

	int getDataVersion()
	{
		return m_iDataVersion;
	}
	int getSample()
	{
		return m_iSample;
	}
	int getTraceCount()
	{
		return m_iTraceCount;
	}
private:
	//��ȡһ��ͨ�������е�16λTrace
	int loadTrace16()
	{
		FILE * m_pFile = fopen(m_FileName, "rb");
		if (nullptr == m_pFile)
			return -2;

		//��ȡ��ʼλ��Ϊ0
		fseek(m_pFile, 0, SEEK_SET);
		//ѭ����ȡĿ��������
		for (int i = 0; i < m_iTraceCount; i++)
		{
			MalaTrace16 *pTrace = new MalaTrace16();
			short *data16 = (short *)malloc(m_iSample * 2);

			fread(data16, 2, m_iSample, m_pFile);

			pTrace->setTrace(data16, m_iSample);
			pTrace->setTraceNum(i+1);

			m_lstData16.insert(std::pair<long, MalaTrace16*>(i, pTrace));
		}

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}
		return 0;
	}
	//��ȡһ��ͨ�������е�32λTrace
	int loadTrace32()
	{
		FILE * m_pFile = fopen(m_FileName, "rb");
		if (nullptr == m_pFile)
			return -2;

		//��ȡ��ʼλ��Ϊ0
		fseek(m_pFile, 0, SEEK_SET);
		//ѭ����ȡĿ��������
		for (int i = 0; i < m_iTraceCount; i++)
		{
			MalaTrace32 *pTrace = new MalaTrace32();
			long *data32 = (long *)malloc(m_iSample * 4);

			fread(data32, 4, m_iSample, m_pFile);

			pTrace->setTrace(data32, m_iSample);
			pTrace->setTraceNum(i + 1);

			m_lstData32.insert(std::pair<long, MalaTrace32*>(i, pTrace));
		}

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}
		return 0;
	};

private:
	int m_iDataVersion = 16;   //trace���ݵ�λ��
	int m_iSample      = 0;    //һ��trace�е���������
	int m_iTraceCount  = 0;    //һ��ͨ����Trace������

	std::map<long, MalaTrace16*> m_lstData16;
	std::map<long, MalaTrace32*> m_lstData32;
	char m_FileName[1024] = { 0 };
};
 
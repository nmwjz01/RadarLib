#pragma once

#include <fstream>
#include <strstream>
#include <map>
//#include "Utils.h"

class MalaTimeData
{
public:
	MalaTimeData() {};
	~MalaTimeData() {};

	void setData(long iTraceNum, char* pDate, char* pTime)
	{
		m_iTraceNum = iTraceNum;
		strcpy(m_szDate, pDate);
		strcpy(m_szTime, pTime);
		Utils::time2long(pTime, m_iTime);
	}
	long getTimeInt()
	{
		return m_iTime;
	}
	long getTraceNum()
	{
		return m_iTraceNum;
	}
	char* getDateString()
	{
		return m_szDate;
	}
	char* getTimeString()
	{
		return m_szTime;
	}

private:
	long m_iTraceNum = 0;
	long m_iTime = 0;
	char m_szDate[64] = { 0 };
	char m_szTime[16] = { 0 };

};

class MalaTime
{
public:
	MalaTime() {};
	~MalaTime()
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, MalaTimeData*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaTimeData* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	};

	int init(char* pPath, char *pSwathID)
	{
		//�����Ϸ���У��
		if ((nullptr == pPath) || (nullptr == pSwathID))
			return -1;
		if ((0 >= strlen(pPath)) || ( 0 >= strlen(pSwathID)) )
			return -1;

		//time�ļ�
		char pPathFile[512] = { 0 };
		sprintf(pPathFile, "%s\\%s.mtts", pPath, pSwathID);

		std::ifstream ifs;
		char s[300], sf[100];

		//���ļ�
		ifs.open(pPathFile);
		if (ifs.fail())
			return -2;

		traceCount = 0;

		//�б��е��������
		m_lstData.clear();
		//���ļ��м�����������
		while (!ifs.eof())
		{
			int iTraceNum = 0;      //trace��
			char szDate[64] = { 0 };  //����
			char szTime[16] = { 0 };  //ʱ��

			//��ȡһ�����ݣ����300�ֽ�
			ifs.getline(s, 300);

			//ȡ��TraceNum
			int iResult = Utils::findField(s, 0, ',', sf);
			if (0 == iResult)
				iTraceNum = atoi(sf);
			else
				continue;

			//��ȡ���ں�ʱ��
			char szTmp[64] = { 0 };
			iResult = Utils::findField(s, 1, ',', szTmp);
			if (0 != iResult)
				continue;

			//��ȡ����ʱ�䴮��ǰ10���ֽ���Ϊ����
			strncpy(szDate, szTmp, 10);
			//��ȡ�ո�������Ϊʱ��
			char *startTmp = strstr(szTmp, " " );
			startTmp++;
			strcpy(szTime, startTmp);

			//iResult = Utils::findField(s, 1, ',', szDate);
			//if (0 != iResult)
			//	continue;

			//��ȡʱ��
			//iResult = Utils::findField(s, 2, ',', szTime);
			//if (0 != iResult)
			//	continue;

			MalaTimeData *data = new MalaTimeData();
			data->setData(iTraceNum, szDate, szTime);
			//��Trace��Cor���ݼ���map
			m_lstData.insert(std::pair<int, MalaTimeData*>(iTraceNum, data));

			traceCount++;
		}

		//�P�]�ļ�
		ifs.close();

		return 0;
	};

	MalaTimeData *getData(long iTraceNum)
	{
		std::map<int, MalaTimeData*>::iterator  iter = m_lstData.find(iTraceNum);
		return iter->second;
	}

	std::map<int, MalaTimeData*> *getListData()
	{
		return &m_lstData;
	};

	int getTraceCount()
	{
		return traceCount;
	}
private:
	std::map<int, MalaTimeData*> m_lstData;
	int traceCount = 0;
};

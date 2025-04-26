#pragma once

#include <fstream>
//#include "Utils.h"

class SwathTimeData
{
public:
	SwathTimeData() {};
	~SwathTimeData() {};

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

	//�Ƚ�ʱ��ľ���ֵ
	int CompareAbs(char *traceDateTime)
	{
		long long myDataL = 0;
		char myDate[64] = { 0 };
		sprintf(myDate, "%s	%s", m_szDate, m_szTime);

		Utils::date2long(myDate, myDataL);

		long long traceDataL = 0;
		Utils::date2long(traceDateTime, traceDataL);

		return (int)abs(traceDataL - myDataL);
	}

private:
	long m_iTraceNum  = 0;
	long m_iTime      = 0;
	char m_szDate[32] = { 0 };
	char m_szTime[16] = { 0 };

};

class SwathTime
{
public:
	SwathTime(){}
	~SwathTime()
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, SwathTimeData*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			SwathTimeData* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	}

	int init( char* pPathFile )
	{
		//�����Ϸ���У��
		if (nullptr == pPathFile)
			return -1;
		if (0 >= strlen(pPathFile))
			return -1;

		std::ifstream ifs;
		char s[300], sf[100];

		//���ļ�
		ifs.open(pPathFile);
		if (ifs.fail())
			return 0;    //time�ļ�����Ҫ���������������ļ�
			//return -2;

		int iPrevTraceNum = 0;

		//�б��е��������
		m_lstData.clear();
		//���ļ��м�����������
		while (!ifs.eof())
		{
			int iTraceNum   = 0;      //trace��
			char szDate[16] = { 0 };  //����
			char szTime[16] = { 0 };  //ʱ��

			//��ȡһ�����ݣ����300�ֽ�
			ifs.getline(s, 300);

			//ȡ��TraceNum
			int iResult = Utils::findField(s, 0, '\t', sf);
			if (0 == iResult)
				iTraceNum = atoi(sf);
			else
				continue;

			//��ȡ����
			iResult = Utils::findField(s, 1, '\t', szDate);
			if (0 != iResult)
				continue;

			//��ȡʱ��
			iResult = Utils::findField(s, 2, '\t', szTime);
			if (0 != iResult)
				continue;

			//���trace�ţ�������һ��
			if (iTraceNum == iPrevTraceNum)
			{
				if (m_lstData.size() > 0)
				{
					m_lstData.erase(iTraceNum);
				}
			}
			iPrevTraceNum = iTraceNum;

			SwathTimeData *data = new SwathTimeData();
			data->setData(iTraceNum, szDate, szTime);
			//��Trace��Cor���ݼ���map
			m_lstData.insert(std::pair<int, SwathTimeData*>(iTraceNum, data));
		}

		//�P�]�ļ�
		ifs.close();

		return 0;
	}

	void addData(long iTraceNum, SwathTimeData *data)
	{
		std::map<long, SwathTimeData*>::iterator  iter = m_lstData.find(iTraceNum);
		if (  iter != m_lstData.end() )
		{
			SwathTimeData *tmpData = iter->second;
			delete tmpData;

			m_lstData.insert(std::pair<long, SwathTimeData*>(iTraceNum, data));
		}
		else
		{
			m_lstData.insert(std::pair<long, SwathTimeData*>(iTraceNum, data));
		}
	}

	SwathTimeData *getData(long iTraceNum)
	{
		std::map<long, SwathTimeData*>::iterator  iter = m_lstData.find(iTraceNum);
		return iter->second;
	}

	int getTraceCount()
	{
		return (int)m_lstData.size();
	}

	int saveTime( const char * pathFile )
	{
		FILE * m_pFile = fopen(pathFile, "wt+");
		if (nullptr == m_pFile)
			return -2;

		//��ȡ��ʼλ��Ϊ0
		fseek(m_pFile, 0, SEEK_SET);
		//ѭ����ȡĿ��������
		for (int i = 1; i <= m_lstData.size(); i++)
		{
			std::map<long, SwathTimeData*>::iterator  iter = m_lstData.find(i);
			if (iter == m_lstData.end())
				break;

			SwathTimeData *timeData = iter->second;
			if (!timeData)
				continue;

			char data[256] = { 0 };
			sprintf(data, "%d\t%s\t%s\n", i, timeData->getDateString(), timeData->getTimeString());

			//����д���ļ�
			fwrite(data, 1, strlen(data) , m_pFile);
		}

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}
		return 0;
	};

	//ͨ��Trace������ʱ�� ��ȡ Trace��
	int getTraceNumByTime(char *szTraceTime)
	{
		SwathTimeData *theTime = NULL;

		int diffTimeMin = 10000;
		int traceNum = -1;
		for (std::map<long, SwathTimeData*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			SwathTimeData* p = iter->second;
			if (!p)
				continue;

			//�Ƚ�ʱ���ֵ
			int diffTimeTmp = p->CompareAbs(szTraceTime);
			if (diffTimeTmp < diffTimeMin)
			{
				theTime = p;
				diffTimeMin = diffTimeTmp;
			}
		}

		if (theTime)
			return theTime->getTraceNum();
		else
			return -1;
	}

	//ͨ��Trace������ʱ�� ��ȡ Trace��
	int getTraceTimeByNum(int traceNum, char *szTraceTime)
	{
		if (!szTraceTime)
			return -1;

		std::map<long, SwathTimeData*>::iterator it = m_lstData.find(traceNum);
		if (it == m_lstData.end())
			return -1;

		SwathTimeData* swathTimeData = it->second;
		if(!swathTimeData)
			return -1;
		sprintf(szTraceTime, "%s	%s", swathTimeData->getDateString(), swathTimeData->getTimeString());

		return 0;
	}

private:
	std::map<long, SwathTimeData*> m_lstData;
};

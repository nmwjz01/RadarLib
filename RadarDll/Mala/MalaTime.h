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
		//释放所有分配的空间
		for (std::map<int, MalaTimeData*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaTimeData* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	};

	int init(char* pPath, char *pSwathID)
	{
		//参数合法性校验
		if ((nullptr == pPath) || (nullptr == pSwathID))
			return -1;
		if ((0 >= strlen(pPath)) || ( 0 >= strlen(pSwathID)) )
			return -1;

		//time文件
		char pPathFile[512] = { 0 };
		sprintf(pPathFile, "%s\\%s.mtts", pPath, pSwathID);

		std::ifstream ifs;
		char s[300], sf[100];

		//打开文件
		ifs.open(pPathFile);
		if (ifs.fail())
			return -2;

		traceCount = 0;

		//列表中的数据清空
		m_lstData.clear();
		//从文件中加载所有数据
		while (!ifs.eof())
		{
			int iTraceNum = 0;      //trace号
			char szDate[64] = { 0 };  //日期
			char szTime[16] = { 0 };  //时间

			//读取一行数据，最大300字节
			ifs.getline(s, 300);

			//取得TraceNum
			int iResult = Utils::findField(s, 0, ',', sf);
			if (0 == iResult)
				iTraceNum = atoi(sf);
			else
				continue;

			//读取日期和时间
			char szTmp[64] = { 0 };
			iResult = Utils::findField(s, 1, ',', szTmp);
			if (0 != iResult)
				continue;

			//读取日期时间串的前10个字节作为日期
			strncpy(szDate, szTmp, 10);
			//读取空格后面的作为时间
			char *startTmp = strstr(szTmp, " " );
			startTmp++;
			strcpy(szTime, startTmp);

			//iResult = Utils::findField(s, 1, ',', szDate);
			//if (0 != iResult)
			//	continue;

			//读取时间
			//iResult = Utils::findField(s, 2, ',', szTime);
			//if (0 != iResult)
			//	continue;

			MalaTimeData *data = new MalaTimeData();
			data->setData(iTraceNum, szDate, szTime);
			//将Trace的Cor数据加入map
			m_lstData.insert(std::pair<int, MalaTimeData*>(iTraceNum, data));

			traceCount++;
		}

		//關閉文件
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

#pragma once

#include <map>
#include <fstream>
//#include "Utils.h"

typedef struct _SwathCorData
{
public:
	void setData(long iTraceNum, char* pDate, char* pTime, double dLat, double dLon, double dMSL, int iFixQuality)
	{
		m_iTraceNum = iTraceNum;
		strcpy(m_szDate, pDate);
		strcpy(m_szTime, pTime);
		m_dLat = dLat;
		m_dLon = dLon;
		m_dMSL = dMSL;
		m_iFixQuality = iFixQuality;

		Utils::time2long(pTime , m_iTime);
	}
	long getTraceNum()
	{
		return m_iTraceNum;
	}
	long getTimeInt()
	{
		return m_iTime;
	}
	double getLat()
	{
		return m_dLat;
	}
	double getLon()
	{
		return m_dLon;
	}
	double getMSL()
	{
		return m_dMSL;
	}
	int getFixQuality()
	{
		return m_iFixQuality;
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
	long m_iTraceNum  = 0;
	long m_iTime      = 0;
	double m_dLat     = 0;
	double m_dLon     = 0;
	double m_dMSL     = 0;
	int m_iFixQuality = 0;
	char m_szDate[16] = { 0 };
	char m_szTime[16] = { 0 };
} SwathCorData;

class SwathCor
{
public:
	SwathCor(){}
	~SwathCor()
	{
		std::map<long, SwathCorData*>::iterator oIter = m_lstData.begin();
		while (oIter != m_lstData.end())
		{
			SwathCorData* pData = oIter->second;

			if (NULL == pData)
				continue;

			delete pData;

			oIter++;
		}

		m_lstData.clear();
	}

	int init( char *pPathFile )
	{
		//�����Ϸ���У��
		if (nullptr == pPathFile)
			return -1;
		if( 0 >= strlen(pPathFile) )
			return -1;

		std::ifstream ifs;
		char s[300], sf[100];

		int iPrevTraceNum = 0;

		//���ļ�
		ifs.open(pPathFile);
		if (ifs.fail())
			return -2;

		//�б��е��������
		m_lstData.clear();
		//���ļ��м�����������
		while (!ifs.eof())
		{
			int iTraceNum   = 0;      //trace��
			char szDate[16] = { 0 };  //����
			char szTime[16] = { 0 };  //ʱ��
			double dLat     = 0;      //γ��
			double dLon     = 0;      //����
			double dMSL     = 0;      //��Ժ�ƽ��߶�
			int iFixQuality = 0;

			//��ȡһ�����ݣ����300�ֽ�
			ifs.getline(s, 300);

			//ȡ��TraceNum
			int iResult = Utils::findField(s, 0, '\t', sf);
			if ( 0 == iResult )
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

			//ȡ��γ��
			iResult = Utils::findField(s, 3, '\t', sf);
			if (0 == iResult)
				dLat = atof(sf);
			else
				continue;
			//�������γ����γ�ȱ�Ϊ����
			iResult = Utils::findField(s, 4, '\t', sf);
			if (0 == iResult)
			{
				if ('S' == sf[0]) dLat = -dLat;
			}
			else
				continue;

			//ȡ�þ���
			iResult = Utils::findField(s, 5, '\t', sf);
			if (0 == iResult)
				dLon = atof(sf);
			else
				continue;
			//������������򾭶ȱ�Ϊ����
			iResult = Utils::findField(s, 6, '\t', sf);
			if (0 == iResult)
			{
				if ('W' == sf[0]) dLon = -dLon;
			}
			else
				continue;

			//ȡ�ø߶�
			iResult = Utils::findField(s, 7, '\t', sf);
			if (0 == iResult)
				dMSL = atof(sf);
			else
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

			SwathCorData *pData = new SwathCorData();
			pData->setData(iTraceNum, szDate, szTime, dLat, dLon, dMSL, iFixQuality);
			//��Trace��Cor���ݼ���map
			m_lstData.insert(std::pair<int, SwathCorData*>(iTraceNum, pData));
		}

		ifs.close();
		//
		return 0;
	}
	void addData(long iTraceNum, SwathCorData *pData)
	{
		m_lstData.insert(std::pair<long, SwathCorData*>(iTraceNum, pData));
	}
	
	/*
	* Fun:    ͨ��Trace�ţ���ȡһ��������Ϣ�����û����ȫ�Ǻϵ����꣬���ȡһ�����������
	* Param:  long iTraceNum Trace��
	*/
	SwathCorData *getData(long iTraceNum )
	{
		if (0 == m_lstData.size())
			return NULL;

		//������Trace�ľ���
		int iBestDistance = 5000;
		//��������Trace
		int iBestTrace    = 0;

		std::map<long, SwathCorData*>::iterator oIter = m_lstData.begin();
		while (oIter != m_lstData.end())
		{
			SwathCorData *pData = oIter->second;
			int iTmpNum = pData->getTraceNum();

			if (abs(iTraceNum - iTmpNum) < iBestDistance)
			{
				iBestTrace = iTmpNum;
				iBestDistance = abs(iTraceNum - iTmpNum);
			}

			oIter++;
		}

		std::map<long, SwathCorData*>::iterator iterDest = m_lstData.find(iBestTrace);
		if (iterDest == m_lstData.end())
			return NULL;
		else
			return (SwathCorData*)iterDest->second;
	}

	//��Cor�������
	int saveCor( const char *pathFile )
	{
		FILE * m_pFile = fopen(pathFile, "wt+");
		if (nullptr == m_pFile)
			return -2;

		//��ȡ��ʼλ��Ϊ0
		fseek(m_pFile, 0, SEEK_SET);
		//ѭ����ȡĿ��������
		for (int i = 0; i < m_lstData.size(); i++)
		{
			std::map<long, SwathCorData*>::iterator  iter = m_lstData.find(i);
			SwathCorData *corData = iter->second;
			if (!corData)
				continue;

			char data[256] = { 0 };
			sprintf(data, "%d\t%s\t%s\t%f\tN\t%f\tE\r\n", corData->getTraceNum(), corData->getDateString(), corData->getTimeString(),corData->getLat(), corData->getLon());

			//����д���ļ�
			fread(data, 1, strlen(data), m_pFile);
		}

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}
		return 0;
	};

private:
	std::map<long,SwathCorData*> m_lstData;
};

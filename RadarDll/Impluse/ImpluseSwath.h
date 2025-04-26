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
		//释放所有分配的空间
		for (std::map<int, SwathChannel*>::iterator iter = m_lstSwathChannel.begin(); iter != m_lstSwathChannel.end(); iter++)
		{
			SwathChannel* p = iter->second;
			delete p;
		}
		m_lstSwathChannel.clear();
	}

	/*
	* Fun:   初始化测线
	* Param: char* pPath   测线文件路径
	*        char *pSwathID测线ID，测线名
	* Return:成功返回0,失败返回错误码
	*/
	int init(char* pPath, char *pSwathID)
	{
		int iResult = 0;
		char szPathFile[512]     = { 0 };
		char szPathFileIPRB[512] = { 0 };
		char szPathFileIPRH[512] = { 0 };

		//记录测线ID
		if (128 <= strlen(pSwathID))
			strncpy(m_szSwathID, pSwathID, 128);
		else
			strcpy(m_szSwathID, pSwathID);

		//组装cor文件的路径和文件名，然后初始化Cor对象
		sprintf(szPathFile, "%s\\%s.cor", pPath, pSwathID);
		iResult = m_oSwathCor.init( szPathFile );
		if (0 != iResult)
			printf( "init cor failed" );
		//	return iResult;

		//组装time文件的路径和文件名，然后初始化Time对象
		sprintf(szPathFile, "%s\\%s.time", pPath, pSwathID);
		iResult = m_oSwathTime.init( szPathFile );
		if (0 != iResult)
			printf("init time failed");
			//return iResult;

		//逐个初始化对应的SwathChannel，然后将得到的SwathChannel加入map
		int i = 1;
		while( true )
		{
			//组装目录下的.iprb
			sprintf(szPathFileIPRB, "%s\\%s_A%02d.iprb", pPath, pSwathID , i );
			if (!Utils::checkExistFile(szPathFileIPRB))
				break;

			//组装目录下的.iprh
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

			//设置通道ID
			pSwathChannel->setNo( i );

			m_lstSwathChannel.insert(std::pair<int, SwathChannel*>( i , pSwathChannel ));

			i++;
		}

		return 0;
	}

	/*
	* Fun:   通过通道ID获取通道数据
	* Param: int iChannelID 通道ID
	* Return:成功返回通道对象，失败返回NULL
	*/
	SwathChannel*getChannel( int iChannelID )
	{
		return m_lstSwathChannel[iChannelID];
	}

	/*
	* Fun:   获取通道数量
	* Param: int& iCount 输出参数，通道数量
	* Return:成功返回0,失败返回错误码
	*/
	int getChannelCount( int &iCount )
	{
		iCount = ( int )m_lstSwathChannel.size();
		return 0;
	}

	/*
	* Fun:   获取测线ID
	* Param: 无
	* Return:成功返回0,失败返回错误码
	*/
	char* getSwathID()
	{
		return m_szSwathID;
	}

	/*
	* Fun:获取测线的时间间隔信息
	* Param:无
	* Return:成功返回SwathTime对象，失败返回NULL
	*/
	SwathTime * getSwathTime()
	{
		return &m_oSwathTime;
	}

	/*
	* Fun:获取测线的位置间隔信息
	* Param:无
	* Return:成功返回SwathCor对象，失败返回NULL
	*/
	SwathCor * getSwathCor()
	{
		return &m_oSwathCor;
	}

	//通过Trace发生的时间 获取 Trace号
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

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
		//释放所有分配的空间--16位
		for (std::map<long, MalaTrace16*>::iterator iter16 = m_lstData16.begin(); iter16 != m_lstData16.end(); iter16++)
		{
			MalaTrace16* p = iter16->second;
			delete p;
		}
		m_lstData16.clear();

		//释放所有分配的空间--32位
		for (std::map<long, MalaTrace32*>::iterator iter32 = m_lstData32.begin(); iter32 != m_lstData32.end(); iter32++)
		{
			MalaTrace32* p = iter32->second;
			delete p;
		}
		m_lstData32.clear();
	};

	//采用通道数据文件初始化通道数据对象
	int init( char *pathFile, int bitFlag, int iSample, int iTraceCount )
	{
		//参数合法性判断
		if (!pathFile)
			return 1;
		if (!strlen(pathFile))
			return 1;

		//记录blob文件名
		strcpy(m_FileName, pathFile);

		m_iDataVersion = bitFlag;		//blob是32位还是16位
		m_iSample      = iSample;       //每Trace的采样点数
		m_iTraceCount  = iTraceCount;   //Trace数量

		//根据位数加载trace数据
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
	* Fun:设置通道数据参数
	* Param:
	*     int iVersion     trace数据的位数,16位或32位
	*     int iSample      每个trace的样本数量
	*     int iTraceCount  Trace数量
	*/
	void setChannelParam(int iVersion, int iSample, int iTraceCount)
	{
		m_iDataVersion = iVersion;
		m_iSample      = iSample;
		m_iTraceCount  = iTraceCount;
	};

	//获取trace数据--16位
	std::map<long, MalaTrace16*> *getTrace16List()
	{
		return &m_lstData16;
	};
	//获取trace数据--32位
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
	//读取一个通道中所有的16位Trace
	int loadTrace16()
	{
		FILE * m_pFile = fopen(m_FileName, "rb");
		if (nullptr == m_pFile)
			return -2;

		//读取开始位置为0
		fseek(m_pFile, 0, SEEK_SET);
		//循环读取目标数据量
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
	//读取一个通道中所有的32位Trace
	int loadTrace32()
	{
		FILE * m_pFile = fopen(m_FileName, "rb");
		if (nullptr == m_pFile)
			return -2;

		//读取开始位置为0
		fseek(m_pFile, 0, SEEK_SET);
		//循环读取目标数据量
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
	int m_iDataVersion = 16;   //trace数据的位数
	int m_iSample      = 0;    //一个trace中的样本数量
	int m_iTraceCount  = 0;    //一个通道中Trace的数量

	std::map<long, MalaTrace16*> m_lstData16;
	std::map<long, MalaTrace32*> m_lstData32;
	char m_FileName[1024] = { 0 };
};
 
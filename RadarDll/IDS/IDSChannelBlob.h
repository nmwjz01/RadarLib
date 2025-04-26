#pragma once

#include "IDSTrace16.h"

class IDSChannelBlob
{
public:
	IDSChannelBlob()
	{
		m_iDataVersion = 16;
		m_iSample      = 512;
		m_iTraceCount  = 0;
	};
	~IDSChannelBlob()
	{
		//释放所有分配的空间
		for (std::map<long, IDSTrace16*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			IDSTrace16* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	};

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

	//添加一个16位Trace
	int addTrace16(IDSTrace16 *trace)
	{
		m_iTraceCount++;
		m_lstData.insert(std::pair<long, IDSTrace16*>(m_iTraceCount, trace));

		return 0;
	};

	//获取trace数据
	std::map<long, IDSTrace16*> *getTraceList()
	{
		return &m_lstData;
	};

private:
	int m_iDataVersion = 16;   //trace数据的位数
	int m_iSample      = 0;    //一个trace中的样本数量
	int m_iTraceCount  = 0;    //一个通道中Trace的数量

	std::map<long, IDSTrace16*> m_lstData;
	char m_FileName[1024] = { 0 };
};
 
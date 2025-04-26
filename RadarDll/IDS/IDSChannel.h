#pragma once

#include "IDSChannelHeader.h"
#include "IDSChannelBlob.h"

class IDSChannel
{
public:
	IDSChannel() {};
	~IDSChannel() {};

	int setChannelParam(int iVersion, int iSample, int iTraceCount)
	{
		//设置块参数
		m_oBlob.setChannelParam(iVersion, iSample, iTraceCount);

		return 0;
	};

	//设置ID
	void setID( int id )
	{
		m_iNo = id;
	};
	int getID()
	{
		return m_iNo;
	}

	//向通道中添加trace数据
	void appendData(IDSTrace16 *trace)
	{
		//Trace数量加一
		int traceCount = m_oHeader.GetTraceCount();
		traceCount++;
		m_oHeader.SetTraceCount(traceCount);

		//添加Trace数据
		m_oBlob.addTrace16(trace);
	};

	//返回通道头信息
	IDSChannelHeader * getHeader()
	{
		return &m_oHeader;
	};

	//返回通道数据块
	IDSChannelBlob * getBlob()
	{
		return &m_oBlob;
	};

private:
	IDSChannelHeader m_oHeader;
	IDSChannelBlob   m_oBlob;
	int m_iNo      = 0;

};

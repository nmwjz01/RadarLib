#pragma once

#include "MalaChannelHeader.h"
#include "MalaChannelBlob.h"

class MalaChannel
{
public:
	MalaChannel() {};
	~MalaChannel() {};

	int setChannelParam(int iVersion, int iSample, int iTraceCount)
	{
		//设置块参数
		m_oBlob.setChannelParam(iVersion, iSample, iTraceCount);

		return 0;
	};

	//采用通道头文件和数据文件，初始化一个通道对象
	int init(char *channelHeader, char *channelBlob, int bitFlag)
	{
		//参数合法性校验
		if ((!channelHeader) || (!channelBlob))
			return 1;
		if ((0 == strlen(channelHeader)) || (0 == strlen(channelBlob)))
			return 2;

		//从文件中读取头信息
		int result = m_oHeader.init(channelHeader);
		if (result)
			return result;

		//从文件中读取数据信息
		return m_oBlob.init(channelBlob, bitFlag, m_oHeader.GetSample(), m_oHeader.GetTraceCount());
	}

	//设置ID
	void setID(int id)
	{
		m_iNo = id;
	};
	int getID()
	{
		return m_iNo;
	};

	//返回通道头信息
	MalaChannelHeader * getHeader()
	{
		return &m_oHeader;
	};

	//返回通道数据块
	MalaChannelBlob * getBlob()
	{
		return &m_oBlob;
	};

private:
	MalaChannelHeader m_oHeader;
	MalaChannelBlob   m_oBlob;
	int m_iNo = 0;
};

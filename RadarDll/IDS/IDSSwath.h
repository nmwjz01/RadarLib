#pragma once

#include "IDSChannel.h"

class IDSSwath
{
public:
	IDSSwath()
	{
		//为测线片段，初始化IDS的15个通道
		for (int i = 1; i <= 15; i++)
		{
			IDSChannel *channel = new IDSChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, IDSChannel*>(i, channel));
		}
	};
	IDSSwath(int size)
	{
		//为测线片段，初始化IDS的15个通道
		for (int i = 1; i <= size; i++)
		{
			IDSChannel *channel = new IDSChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, IDSChannel*>(i, channel));
		}
	};
	~IDSSwath()
	{
		int size = ( int )m_lstData.size();

		//测线，数据释放
		for (int i = 1; i <= size; i++)
		{
			IDSChannel* channel = m_lstData[i];
			delete channel;
			m_lstData.clear();
		}
	};

	//将通道列表数据，添加到测线中
	void swathDataAdd(std::map<long, IDSChannel*> *lstData)
	{
		//释放所有分配的空间
		for (std::map<long, IDSChannel*>::iterator iter = lstData->begin(); iter != lstData->end(); iter++)
		{
			IDSChannel* p = iter->second;

			//数据合法性保护
			if (!p)
				continue;
			if ((p->getID() <= 0) || (p->getID() >= 16))
				continue;

			//取得通道对象
			IDSChannel* channel = m_lstData[p->getID()];
			if (!channel)
				continue;

			//获取通道对应的数据头和数据块
			IDSChannelHeader * header = channel->getHeader();
			IDSChannelBlob   * blob   = channel->getBlob();

			if ((!header) || (!blob))
				continue;

			//更新通道中trace的数量
			//int traceCount = header->GetTraceCount() + p->getHeader()->GetTraceCount();
			//header->SetTraceCount(traceCount);

			//添加通道中的trace数据
			IDSChannelBlob * blobNew = p->getBlob();
			std::map<long, IDSTrace16*> * blobList = blobNew->getTraceList();
			for (std::map<long, IDSTrace16*>::iterator itBlob = blobList->begin(); itBlob != blobList->end(); itBlob++)
			{
				//添加通道中的trace数据
				channel->appendData(itBlob->second);
			}
		}
	};

	//清除测线中的数据，并且释放空间
	void swathDataClear()
	{
		//释放原有分配的空间
		for (std::map<long, IDSChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			IDSChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//重新分配空间
		for (int i = 1; i <= 15; i++)
		{
			IDSChannel *channel = new IDSChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, IDSChannel*>(i, channel));
		}

		swathID = 0;
	};

	//返回测线ID
	int getID()
	{
		return swathID;
	};
	//设置测线ID
	void setID( int iID )
	{
		swathID = iID;
	};

	//获取所有通道列表
	std::map<long, IDSChannel*> *getData()
	{
		return &m_lstData;
	};

private:
	std::map<long, IDSChannel*> m_lstData;
	int swathID;
};

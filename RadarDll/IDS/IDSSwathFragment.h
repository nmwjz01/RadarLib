#pragma once

#include <map>
#include "IDSChannel.h"

class IDSSwathFragment
{
public:
	IDSSwathFragment()
	{};
	~IDSSwathFragment() { /*clear();*/ };

	/*
	* Fun:使用IDS的RAW文件初始化类对象，初始化完成后，可以取得Raw文件的各个结构化数据
	*     使用一个RAW_00000X.scan初始化一个IDS测线片段
	* Para:带有路径的Raw文件
	* Return:成功返回0。失败返回错误码
	*/
	int init(const char *file)
	{
		//为测线片段，初始化IDS的15个通道
		for (int i = 1; i <= 15; i++)
		{
			IDSChannel *channel = new IDSChannel();
			m_lstData.insert(std::pair<long, IDSChannel*>(i, channel));
		}

		//打开文件
		FILE *fpIDS = fopen(file, "rb");
		fseek(fpIDS, 6, SEEK_SET);

		char buff[128] = { 0 };
		//读取测线号
		fread(buff, 2, 1, fpIDS);
		iID = buff[0] + buff[1] * 256;

		//文件固定位置
		int start = 0x01ec;
		//定位到数据位置
		//fseek(fpIDS, start, SEEK_CUR);
		fseek(fpIDS, start, SEEK_SET);

		//循环读取每个Trace，并且添加到对应的通道中
		while (!feof(fpIDS))
		{
			//下面要循环，读取数据头的标志位置
			fread(buff, 1, 2, fpIDS);
			if ((0x44 != buff[0]) || (0 != buff[1]))
			{
				//已经读取完成
				break;
			}
			//读取通道号信息
			fread(buff, 1, 2, fpIDS);
			int channelNum = buff[0];

			//数据合法性检测
			if ((channelNum <= 0) || (channelNum >= 16))
				continue;
			//记录trace数量
			if (1 == channelNum)
				traceCount++;

			//取得通道对象
			IDSChannel *channel = m_lstData[channelNum];

			//暂时跳过8字节
			fread(buff, 1, 8, fpIDS);

			//读取实际数据
			int length = 1024;
			unsigned char *pData = (unsigned char *)malloc(length);
			int len = (int)fread(pData, 1, length, fpIDS);

			//while ((!feof(fpIDS)) && (length>0) )
			//{
			//	int len = (int)fread(pData, 1, length, fpIDS);
			//	pData  = pData + len;
			//	length = length - len;
			//}

			//生成一个trace
			IDSTrace16 *trace = new IDSTrace16();
			trace->setTrace16((short *)pData, 512);
			trace->setTraceNum(traceCount);

			//把新的数据添加到通道中
			channel->appendData(trace);
			channel->setID(channelNum);
		}

		return 0;
	};

	/*
	//清除测线片段中的数据
	int clear()
	{
		//释放所有分配的空间
		for (std::map<long, IDSChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			IDSChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	};
	*/

	//返回测线片段中的数据列表
	std::map<long, IDSChannel*> *getDataList()
	{
		return &m_lstData;
	};

	//返回测线ID
	int getID()
	{
		return iID;
	};

	//返回测线片段中trace数量
	int getTraceCount()
	{
		return traceCount;
	};
private:
	std::map<long, IDSChannel*> m_lstData;

	//测线片段中的测线ID
	int iID = 0;

	//测线片段中的Trace数量
	int traceCount = 0;
};


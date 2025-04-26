#pragma once
#include <cstring>
#include <map>

#include "SegyFileHeaderText.h"
#include "SegyFileHeaderBinary.h"
#include "SegyChannel.h"

class SegySwathFile
{
public:
	SegySwathFile()
	{
		segyFileHeaderText   = NULL;
		segyFileHeaderBinary = NULL;

		memset(szPathFile, 0 , 512);
		iSmaple  = 0;
		iBitType = 0;
	};
	~SegySwathFile()
	{
		//释放所有分配的空间
		for (std::map<int, SegyChannel*>::iterator iter = m_lstChannel.begin(); iter != m_lstChannel.end(); iter++)
		{
			SegyChannel* p = iter->second;
			delete p;
		}
		m_lstChannel.clear();
	}

	void setHeaderText(SegyFileHeaderText *fileHeaderText)
	{
		segyFileHeaderText = fileHeaderText;
	}
	void setHeaderBinary(SegyFileHeaderBinary *fileHeaderBinary)
	{
		segyFileHeaderBinary = fileHeaderBinary;

		iSmaple  = segyFileHeaderBinary->getSamples();
		iBitType = segyFileHeaderBinary->getBitType();
	}
	SegyFileHeaderText *getSegyFileHeaderText()
	{
		return segyFileHeaderText;
	}
	SegyFileHeaderBinary *getSegyFileHeaderBinary()
	{
		return segyFileHeaderBinary;
	}

	int getChannelCount()
	{
		return (int)m_lstChannel.size();
	}

	void addChannel(SegyChannel *segyChannel)
	{
		m_lstChannel.insert(std::map<int, SegyChannel*>::value_type(segyChannel->getChannelNum(), segyChannel));
	}
	SegyChannel *getChannel( int iChannelNum )
	{
		return m_lstChannel.at(iChannelNum);
	}

	void setPathFile( char *pathFile )
	{
		strncpy(szPathFile, pathFile, 512);
	}
	char *getPathFile()
	{
		return szPathFile;
	}

	//将对象中的数据转存到文件
	int saveData2File()
	{
		if (strlen(szPathFile) == 0)
			return -1;

		//打开一个文件，用于写入segy文件
		FILE *fp = fopen(szPathFile, "wb");

		if (!segyFileHeaderText)
		{
			//退处前关闭文件
			fclose(fp);
			return -2;
		}
		//将Segy文本文件头写入文件
		unsigned char *headerText = (unsigned char *)segyFileHeaderText->getBuff();
		int iIndex = 0;
		while (iIndex < 3200)
		{
			iIndex = iIndex + (int)fwrite(headerText + iIndex, 1, 3200 - iIndex, fp);
		}

		if (!segyFileHeaderBinary)
		{
			//退处前关闭文件
			fclose(fp);
			return -3;
		}
		//将Segy二进制头写入文件
		unsigned char *headerBinary = segyFileHeaderBinary->getBuff();
		iIndex = 0;
		while (iIndex < 400)
		{
			iIndex = iIndex + (int)fwrite(headerBinary + iIndex, 1, 400 - iIndex, fp);
		}

		//将每个通道顺序存入
		for (std::map<int, SegyChannel*>::iterator iter = m_lstChannel.begin(); iter != m_lstChannel.end(); iter++)
		{
			SegyChannel* p = iter->second;
			std::map<int, SegyTrace*>* lstTrace = p->getTraceList();

			//将每个通道的每个Trace顺序存入
			for (std::map<int, SegyTrace*>::iterator iter = lstTrace->begin(); iter != lstTrace->end(); iter++)
			{
				SegyTrace* pTrace = iter->second;
				unsigned char *pDataHeader = pTrace->getHeaderBuff();
				unsigned char *pData = (unsigned char *)pTrace->getData();

				//将Trace头写入文件
				iIndex = 0;
				while (iIndex < 240)
				{
					iIndex = iIndex + (int)fwrite(pDataHeader + iIndex, 1, 240 - iIndex, fp);
				}

				//将Trace数据
				int iLength = iBitType / 8 * iSmaple;
				iIndex = 0;
				while (iIndex < iLength)
				{
					iIndex = iIndex + (int)fwrite(pData + iIndex, 1, iLength - iIndex, fp);
				}
			}
		}

		//写入完成就关闭文件
		fclose(fp);

		return 0;
	}
	//从文件中读取数据到对象
	int loadFile2Data( const char *pathFile )
	{
		if (strlen(pathFile) == 0)
			return -1;

		strncpy(szPathFile, pathFile, 512);

		//打开文件
		FILE *fp = fopen(pathFile, "rb");
		if (!fp)
		{
			fclose(fp);
			return -2;
		}

		//读取文本文件头，将数据存入文本文件头对象
		char *szBuff = (char *)malloc(3200); memset(szBuff, 0, 3200);
		int iIndex = 0;
		while (iIndex < 3200)
		{
			iIndex = iIndex + (int)fread(szBuff + iIndex, 1, 3200 - iIndex, fp);
		}
		if (segyFileHeaderText)
			delete segyFileHeaderText;
		segyFileHeaderText = new SegyFileHeaderText();
		segyFileHeaderText->setBuff(szBuff);

		//读取二进制文件头
		szBuff = (char *)malloc(400); memset(szBuff, 0, 400);
		iIndex = 0;
		while (iIndex < 400)
		{
			iIndex = iIndex + (int)fread(szBuff + iIndex, 1, 400 - iIndex, fp);
		}
		if (segyFileHeaderBinary)
			delete segyFileHeaderBinary;
		segyFileHeaderBinary = new SegyFileHeaderBinary();
		segyFileHeaderBinary->setBuff((unsigned char *)szBuff);

		//获取采样点数
		iSmaple  = segyFileHeaderBinary->getSamples();
		iBitType = segyFileHeaderBinary->getBitType();

		//如果已经存在数据，则需要清除
		if (m_lstChannel.size())
		{
			clearChannelData(m_lstChannel);
		}

		//构造一个Channel
		SegyChannel *segyChannel = new SegyChannel();
		segyChannel->setChannelNum(1);

		int iChannelNum = 1;
		//int iTraceNum   = 1;
		//循环读取全部Trace数据
		while (!feof(fp))
		{
			//读取Trace头
			szBuff = (char *)malloc(240); memset(szBuff, 0, 240);
			iIndex = 0;
			while((iIndex < 240) && (!feof(fp)))
			{
				iIndex = iIndex + (int)fread(szBuff + iIndex, 1, 240 - iIndex, fp);
			}
			if (0 == iIndex)
			{
				//已经读取到结尾了
				free(szBuff);
				break;
			}
			SegyTrace *segyTrace = new SegyTrace();
			segyTrace->setHeaderBuff((unsigned char *)szBuff);

			//读取Trace数据
			int iTraceLenth = iBitType / 8 * iSmaple;
			szBuff = (char *)malloc(iTraceLenth); memset(szBuff, 0, iTraceLenth);
			iIndex = 0;
			while (iIndex < iTraceLenth)
			{
				iIndex = iIndex + (int)fread(szBuff + iIndex, 1, iTraceLenth - iIndex, fp);
			}
			segyTrace->setData((void *)szBuff);
			//segyTrace->setTraceNum(iTraceNum);

			//读取得到的Trace还在当前通道
			if (segyTrace->getChannelNum() > iChannelNum)
			{
				//得到的Trace出现在后续通道，所以需要将通道放到通道列表
				m_lstChannel.insert(std::map<int, SegyChannel*>::value_type(iChannelNum, segyChannel));

				//这里需要增加一个通道
				iChannelNum++;
				segyChannel = new SegyChannel();
				segyChannel->setChannelNum(iChannelNum);
			}

			segyChannel->addTrace(segyTrace);
		}

		//需要将最后一个通道放到通道列表
		m_lstChannel.insert(std::map<int, SegyChannel*>::value_type(iChannelNum, segyChannel));

		return 0;
	}

private :
	void clearChannelData(std::map<int, SegyChannel*> m_lst)
	{
		//释放所有分配的空间
		for (std::map<int, SegyChannel*>::iterator iter = m_lst.begin(); iter != m_lst.end(); iter++)
		{
			SegyChannel* p = iter->second;
			delete p;
		}
		m_lst.clear();
	}
private:
	SegyFileHeaderText   *segyFileHeaderText;       //3200字节文本头
	SegyFileHeaderBinary *segyFileHeaderBinary;     //400字节二进制头

	//对于Segy文件，一个文件就是一个Swath，这个测线可能包含多个通道，也可能包含一个通道
	std::map<int, SegyChannel*> m_lstChannel;


	char szPathFile[512];    //测线文件
	int iSmaple;             //采样点数
	int iBitType;            //单个数据点的位数
};

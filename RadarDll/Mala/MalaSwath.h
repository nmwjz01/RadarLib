#pragma once

#include "MalaChannel.h"

class MalaSwath
{
public:
	MalaSwath()
	{
		//为测线片段，初始化Mala的15个通道
		for (int i = 1; i <= 15; i++)
		{
			MalaChannel *channel = new MalaChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, MalaChannel*>(i, channel));
		}
	};
	~MalaSwath()
	{
		//释放所有分配的空间
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	};

	//清除测线中的数据，并且释放空间
	void swathDataClear()
	{
		//释放原有分配的空间
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//重新分配空间
		for (int i = 1; i <= 15; i++)
		{
			MalaChannel *channel = new MalaChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, MalaChannel*>(i, channel));
		}
	};

	/*
	* Fun:   初始化测线
	* Param: char* pPath   测线文件路径
	*        char *pSwathID测线ID，测线名
	*        int bitFlag   标志数据是32位还是16位
	* Return:成功返回0,失败返回错误码
	*/
	int init(char* pPath, char *pSwathID, int bitFlag)
	{
		//参数合法性判断
		if ((!pPath) || (!pSwathID))
		{
			return 1;
		}
		if ((0 == strlen(pPath)) || (0 == strlen(pSwathID)))
		{
			return 2;
		}

		//记录数据类型：16位 or 32位
		bitType = bitFlag;
		//记录测线名称
		strncpy(swathName, pSwathID, 128);

		//判断是不是32进制
		if (32 == bitFlag)
			return init32(pPath, pSwathID);
		else if (33 == bitFlag)
			return init32Ex(pPath, pSwathID);
		else if (16 == bitFlag)
			return init16(pPath, pSwathID);
		else
			return 3;
	}
	/*
	* Fun:   初始化测线,Mala16二维雷达
	* Param: char* pPath   测线文件路径
	*        char *pSwathID测线ID，测线名
	*        int bitFlag   标志数据是32位还是16位
	* Return:成功返回0,失败返回错误码
	*/
	int init16Ex(char* pPath, char *pSwathID, int bitFlag)
	{
		//参数合法性判断
		if ((!pPath) || (!pSwathID))
		{
			return 1;
		}
		if ((0 == strlen(pPath)) || (0 == strlen(pSwathID)))
		{
			return 2;
		}

		//记录数据类型：16位 or 32位
		bitType = bitFlag;
		//记录测线名称
		strncpy(swathName, pSwathID, 128);

		//判断是不是32进制
		return init16Ex(pPath, pSwathID);
	}

	/*
	* Fun:   初始化测线--32位
	* Param: char* pPath   测线文件路径
	*        char *pSwathID测线ID，测线名
	* Return:成功返回0,失败返回错误码
	*/
	int init32(char* pPath, char *pSwathID)
	{
		//释放所有分配的空间
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//通道数量清空
		channelCount = 0;

		//处理测线下面的通道信息
		while (true)
		{
			channelCount++;

			//搜索雷达目录下，指定文件,rd7数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRd7[512] = { 0 };
			sprintf(szPathRd7, "%s\\%s_%03d.rd7", pPath, pSwathID, channelCount);
			intptr_t hFileRd7;
			struct _finddata_t oFileInfoRd7;

			//如果没有找到rd7，则终止
			if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) == -1L)
			{
				_findclose(hFileRd7);
				printf("没有发现rd7的测线数据\n");
				break;
			}
			//关闭rd7文件搜索器
			_findclose(hFileRd7);

			//搜索雷达目录下，指定文件,rad数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRad[512] = { 0 };
			sprintf(szPathRad, "%s\\%s_%03d.RAD", pPath, pSwathID, channelCount);
			intptr_t hFileRad;
			struct _finddata_t oFileInfoRad;

			//如果没有找到rad，则终止
			if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
			{
				_findclose(hFileRad);
				printf("没有发现rad的测线数据\n");
				break;
			}
			//关闭rd7文件搜索器
			_findclose(hFileRad);

			//构造头文件名和数据文件名
			char szPathFileHeader[512] = { 0 };
			char szPathFileBlob[512]   = { 0 };
			sprintf(szPathFileHeader,"%s\\%s", pPath, oFileInfoRad.name);
			sprintf(szPathFileBlob,  "%s\\%s", pPath, oFileInfoRd7.name);

			//初始化一个Channel
			MalaChannel *channel = new MalaChannel();
			channel->init(szPathFileHeader, szPathFileBlob, 32 );
			channel->setID(channelCount );

			m_lstData.insert(std::pair<int, MalaChannel*>(channelCount, channel));
		}

		channelCount--;

		return 0;
	}

	/*
	* Fun:   初始化测线--32位
	* Param: char* pPath   测线文件路径
	*        char *pSwathID测线ID，测线名
	* Return:成功返回0,失败返回错误码
	*/
	int init32Ex(char* pPath, char *pSwathID)
	{
		//释放所有分配的空间
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//搜索雷达目录下，指定文件,rd7数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRd7[512] = { 0 };
		sprintf(szPathRd7, "%s\\%s.rd7", pPath, pSwathID);
		intptr_t hFileRd7;
		struct _finddata_t oFileInfoRd7;

		//如果没有找到rd7，则终止
		if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) == -1L)
		{
			_findclose(hFileRd7);
			printf("没有发现rd7的测线数据\n");
			return 1;
		}
		//关闭rd7文件搜索器
		_findclose(hFileRd7);

		//搜索雷达目录下，指定文件,rad数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRad[512] = { 0 };
		sprintf(szPathRad, "%s\\%s.RAD", pPath, pSwathID);
		intptr_t hFileRad;
		struct _finddata_t oFileInfoRad;

		//如果没有找到rad，则终止
		if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
		{
			_findclose(hFileRad);
			printf("没有发现rad的测线数据\n");
			return 2;
		}
		//关闭rd7文件搜索器
		_findclose(hFileRad);

		//构造头文件名和数据文件名
		char szPathFileHeader[512] = { 0 };
		char szPathFileBlob[512] = { 0 };
		sprintf(szPathFileHeader, "%s\\%s", pPath, oFileInfoRad.name);
		sprintf(szPathFileBlob, "%s\\%s", pPath, oFileInfoRd7.name);

		//初始化一个Channel
		MalaChannel *channel = new MalaChannel();
		channel->init(szPathFileHeader, szPathFileBlob, 32);
		channel->setID(1);   //通道数量只有1个，所以第一个通道ID为1

		m_lstData.insert(std::pair<int, MalaChannel*>(1, channel));

		return 0;
	}

	/*
	* Fun:   初始化测线--16位
	* Param: char* pPath   测线文件路径
	*        char *pSwathID测线ID，测线名
	* Return:成功返回0,失败返回错误码
	*/
	int init16(char* pPath, char *pSwathID)
	{
		//释放所有分配的空间
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//通道数量清空
		channelCount = 0;

		//处理测线下面的通道信息
		while (true)
		{
			//搜索雷达目录下，指定文件,rd3数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRd3[512] = { 0 };
			sprintf(szPathRd3, "%s\\%s_A%03d.rd3", pPath, pSwathID, channelCount);
			intptr_t hFileRd3;
			struct _finddata_t oFileInfoRd3;

			//如果没有找到rd7，则终止
			if ((hFileRd3 = (intptr_t)_findfirst(szPathRd3, &oFileInfoRd3)) == -1L)
			{
				_findclose(hFileRd3);
				printf("没有发现rd3的测线数据\n");
				break;
			}
			//关闭rd7文件搜索器
			_findclose(hFileRd3);

			//搜索雷达目录下，指定文件,rad数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRad[512] = { 0 };
			sprintf(szPathRad, "%s\\%s_A%03d.RAD", pPath, pSwathID, channelCount);
			intptr_t hFileRad;
			struct _finddata_t oFileInfoRad;

			//如果没有找到rad，则终止
			if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
			{
				_findclose(hFileRad);
				printf("没有发现rad的测线数据\n");
				break;
			}
			//关闭rd7文件搜索器
			_findclose(hFileRad);

			//构造头文件名和数据文件名
			char szPathFileHeader[512] = { 0 };
			char szPathFileBlob[512] = { 0 };
			sprintf(szPathFileHeader, "%s\\%s", pPath, oFileInfoRad.name);
			sprintf(szPathFileBlob, "%s\\%s", pPath, oFileInfoRd3.name);

			//初始化一个Channel
			MalaChannel *channel = new MalaChannel();
			channel->init(szPathFileHeader, szPathFileBlob, 16);
			channel->setID(channelCount);

			m_lstData.insert(std::pair<int, MalaChannel*>(channelCount, channel));

			channelCount++;
		}

		return 0;
	}

	/*
	* Fun:   初始化测线--16位
	* Param: char* pPath   测线文件路径
	*        char *pSwathID测线ID，测线名
	* Return:成功返回0,失败返回错误码
	*/
	int init16Ex(char* pPath, char *pSwathID)
	{
		//释放所有分配的空间
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//处理测线下面的通道信息
		{
			//搜索雷达目录下，指定文件,rd3数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRd3[512] = { 0 };
			sprintf(szPathRd3, "%s\\%s*.rd3", pPath, pSwathID);
			intptr_t hFileRd3;
			struct _finddata_t oFileInfoRd3;

			//如果没有找到rd7，则终止
			if ((hFileRd3 = (intptr_t)_findfirst(szPathRd3, &oFileInfoRd3)) == -1L)
			{
				printf("没有发现rd3的测线数据\n");
				_findclose(hFileRd3);
				return 0;
			}
			//关闭rd7文件搜索器
			_findclose(hFileRd3);

			//搜索雷达目录下，指定文件,rad数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRad[512] = { 0 };
			sprintf(szPathRad, "%s\\%s*.rad", pPath, pSwathID);
			intptr_t hFileRad;
			struct _finddata_t oFileInfoRad;

			//如果没有找到rad，则终止
			if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
			{
				printf("没有发现rad的测线数据\n");
				_findclose(hFileRad);
				return 0;
			}
			//关闭rd7文件搜索器
			_findclose(hFileRad);

			//构造头文件名和数据文件名
			char szPathFileHeader[512] = { 0 };
			char szPathFileBlob[512] = { 0 };
			sprintf(szPathFileHeader, "%s\\%s", pPath, oFileInfoRad.name);
			sprintf(szPathFileBlob, "%s\\%s", pPath, oFileInfoRd3.name);

			//初始化一个Channel
			MalaChannel *channel = new MalaChannel();
			channel->init(szPathFileHeader, szPathFileBlob, 16);
			channel->setID(0);

			m_lstData.insert(std::pair<int, MalaChannel*>(0, channel));
		}

		return 0;
	}

	//返回测线Name
	char * getName()
	{
		return swathName;
	};
	//设置测线ID
	void setID( char * name )
	{
		strncpy(swathName, name, 128);
	};
	//获取所有通道列表
	std::map<long, MalaChannel*> *getData()
	{
		return &m_lstData;
	};

private:
	std::map<long, MalaChannel*> m_lstData;
	char swathName[128] = { 0 };
	int channelCount = 0;
	int bitType = 16;
};

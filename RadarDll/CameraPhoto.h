#pragma once

#include <io.h>
#include <vector>

class CameraPhoto
{
public:
	CameraPhoto() {}
	~CameraPhoto()
	{
		//释放空间
		for ( int i=0; i< (int)m_lstFileName.size(); i++)
		{
			char *p = m_lstFileName[i];
			free(p);
		}
		m_lstFileName.clear();
	}

	int init(char *pPath)
	{
		if (nullptr == pPath)
			return -1;
		if (0 >= strlen(pPath))
			return -1;

		if (512 <= strlen(pPath))
			strncpy(szPath, pPath, 512);
		else
			strcpy(szPath, pPath);

		//检查目录是否存在
		struct stat oDirState;
		if (-1 == stat(szPath, &oDirState))
		{
			printf("视频抓拍目录%s不存在\n", szPath);
			return -2;
		}

		//
		char szPathFile[512] = { 0 };
		sprintf(szPathFile, "%s\\*.*", szPath);
		//搜索抓拍目录下，所有视频文件名，用于匹配雷达数据      //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		intptr_t hFile;
		struct _finddata_t oFileInfo;
		if ((hFile = (intptr_t)_findfirst(szPathFile, &oFileInfo)) == -1L)
		{
			printf("没有发现视频文件数据\n");
			return 0;
		}

		//初始化所有文件名
		char *pFileName = ( char * )malloc( 32 );
		strncpy(pFileName, oFileInfo.name, 30 );
		m_lstFileName.push_back(pFileName);

		while (_findnext(hFile, &oFileInfo) == 0)
		{
			pFileName = (char *)malloc(32);
			strncpy(pFileName, oFileInfo.name, 30);
			m_lstFileName.push_back(pFileName);
		}
		_findclose(hFile);

		//TestStub();
		return 0;
	}

	//逆初始化
	void unInit()
	{
		//释放空间
		for (int i = 0; i < (int)m_lstFileName.size(); i++)
		{
			char *p = m_lstFileName[i];
			free(p);
		}
		m_lstFileName.clear();
	}

	//通过时间获取最满足时间的图片 
	int getPictureByTime(char* pDateTime, char szDestFile[])
	{
		if (NULL == pDateTime)
			return -1;
		if (0 >= strlen(pDateTime))
			return  -1;

		char szTmp[64] = { 0 };
		strcpy(szTmp, pDateTime);

		transferDateTime(szTmp);

		int iBegin  = 0;
		int iEnd    = ( int )m_lstFileName.size();
		if (0 >= iEnd)
			return 0;

		int iDest = 0;
		//循环比较
		while ( true )
		{
			if (1 == (iEnd - iBegin))
			{
				iDest = iBegin;
				break;
			}

			int iMidlle = (iBegin + iEnd) / 2;

			char * pMidlleTime = m_lstFileName[iMidlle];
			//*(pMidlleTime + 19) = 0;

			int iResult = strncmp(szTmp, pMidlleTime, 19);
			if (0 == iResult)
			{
				iDest = iMidlle;
				break;
			}
			else if (0 > iResult)
			{
				iEnd = iMidlle;
				continue;
			}
			else
			{
				iBegin = iMidlle;
				continue;
			}
		}

		sprintf( szDestFile , "%s" , m_lstFileName[iDest] );
		if ('.' == szDestFile[0])
			szDestFile[0] = 0;
		return 0;
	}

	//返回路径
	char *getPath()
	{
		return szPath;
	}

private:
	void transferDateTime(char * pDateTme)
	{
		char *pTmp = pDateTme;
		/*
		//月
		*(pTmp + 4) = *(pTmp + 5);
		*(pTmp + 5) = *(pTmp + 6);

		//日
		*(pTmp + 6) = *(pTmp + 8);
		*(pTmp + 7) = *(pTmp + 9);

		*(pTmp + 8) = '_';

		//时
		*(pTmp + 9)  = *(pTmp + 11);
		*(pTmp + 10) = *(pTmp + 12);

		//分
		*(pTmp + 11) = *(pTmp + 14);
		*(pTmp + 12) = *(pTmp + 15);

		//秒
		*(pTmp + 13) = *(pTmp + 17);
		*(pTmp + 14) = *(pTmp + 18);

		*(pTmp + 15) = '.';

		//毫秒
		*(pTmp + 16) = *(pTmp + 20);
		*(pTmp + 17) = *(pTmp + 21);
		*(pTmp + 18) = *(pTmp + 22);

		*(pTmp + 19) = 0;
		*/

		//分
		*(pTmp + 13) = *(pTmp + 14);
		*(pTmp + 14) = *(pTmp + 15);

		//秒
		*(pTmp + 15) = *(pTmp + 17);
		*(pTmp + 16) = *(pTmp + 18);

		*(pTmp + 17) = '.';

		//毫秒
		*(pTmp + 18) = *(pTmp + 20);
		*(pTmp + 19) = *(pTmp + 21);
		*(pTmp + 20) = *(pTmp + 22);
		*(pTmp + 21) = 0;

		strcpy(pDateTme, pTmp);
	}

	void tranferTimeZone(char * pDateTme)
	{
	}

private:
	//抓拍图片存放目录
	char szPath[512] = { 0 };
	std::vector<char *> m_lstFileName;
};

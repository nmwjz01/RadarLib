#pragma once

#include <vector>

//#include "Swath.h"
#include "CameraPhoto.h"

#include <iostream>  
#include <exception>  
using namespace std;


class Project
{
public:
	Project() {}
	~Project()
	{
		//清除所有Swath数据
		clearAllSwath();
	}

	/*
	* Fun:   初始化工程
	* Param: char *pPath 工程目录
	* Return:成功返回0，失败返回错误码
	*/
	int init( char *pPath )
	{
		if (nullptr == pPath)
			return -1;
		if (0 >= strlen(pPath))
			return -1;

		//保存工程目录
		if (512 <= strlen(pPath))
			strncpy(szPathProject, pPath,512);
		else
			strcpy(szPathProject, pPath);

		//如果结尾是一个"\",就去掉
		if ('\\' == *(szPathProject + strlen(szPathProject) - 1))
			*(szPathProject + strlen(szPathProject) - 1) = 0;

		//获取工程的目录
		int i = ( int )strlen(szPathProject) - 1;
		for (; i > 0; i--)
		{
			if ('\\' == *(szPathProject + i))
				break;
		}

		//组装数据目录
		sprintf(szPathRadar     , "%s\\%s", szPathProject, DATA_PATH_RADAR);
		sprintf(szPathVideoFront, "%s\\%s", szPathProject, DATA_PATH_VIDEO_FRONT);
		sprintf(szPathVideoBack,  "%s\\%s", szPathProject, DATA_PATH_VIDEO_BACK);
		sprintf(szPathVideoLeft,  "%s\\%s", szPathProject, DATA_PATH_VIDEO_LEFT);
		sprintf(szPathVideoRight, "%s\\%s", szPathProject, DATA_PATH_VIDEO_RIGHT);

		//检查目录是否存在
		if (!Utils::checkExistDir(szPathProject) )
		{
			printf("工程目录%s不存在\n", szPathProject);
			return -2;
		}
		//检查雷达数据目录是否存在
		if (!Utils::checkExistDir(szPathRadar))
		{
			printf("工程目录%s中，不存在雷达数据目录%s\n", szPathProject, szPathRadar);
			return -3;
		}
		//检查抓拍视频目录是否存在(前置 抓拍机)
		if (!Utils::checkExistDir(szPathVideoFront))
		{
			printf("工程目录%s中，不存在前置抓拍视频目录%s\n", szPathProject, szPathVideoFront);
		}
		//检查抓拍视频目录是否存在(前置 抓拍机)
		if (!Utils::checkExistDir(szPathVideoBack))
		{
			printf("工程目录%s中，不存在后置抓拍视频目录%s\n", szPathProject, szPathVideoBack);
		}
//		=====================

		//检查抓拍视频目录是否存在(左置 抓拍机)
		if (!Utils::checkExistDir(szPathVideoLeft))
		{
			printf("工程目录%s中，不存在左置抓拍视频目录%s\n", szPathProject, szPathVideoLeft);
		}
		//检查抓拍视频目录是否存在(右置 抓拍机)
		if (!Utils::checkExistDir(szPathVideoRight))
		{
			printf("工程目录%s中，不存在右置抓拍视频目录%s\n", szPathProject, szPathVideoRight);
		}

		//清除所有已經存在的Swath数据
		clearAllSwath();

		//获取所有测线名，并且放到vector中
		findAllSwath(szPathRadar);

		int iResult = 0;
		//初始化每个测线
		std::vector<char *>::iterator  it_pos;
		//遍历，每个测线名，然后初始化
		for (it_pos = m_lstSwathName.begin(); it_pos != m_lstSwathName.end(); it_pos++)
		{
			char * pSwathName = *it_pos;
			if ((NULL == pSwathName) && (strlen(pSwathName) <= 0))
				continue;

			Swath * pSwath = new Swath();
			iResult = pSwath->init(szPathRadar, pSwathName);
			if (0 != iResult)
			{
				delete pSwath;
				return iResult;
			}
			m_lstSwath.push_back(pSwath);
		}

		//初始化视频对象，前置抓拍机
		if (0 != m_oCameraFront.init(szPathVideoFront))
		{
			printf("初始化视频目录出错\n");
		}
		//初始化视频对象，后置抓拍机
		if (0 != m_oCameraBack.init(szPathVideoBack))
		{
			printf("初始化视频目录出错\n");
		}
		//初始化视频对象，左置抓拍机
		if (0 != m_oCameraLeft.init(szPathVideoLeft))
		{
			printf("初始化视频目录出错\n");
		}
		//初始化视频对象，右置抓拍机
		if (0 != m_oCameraRight.init(szPathVideoRight))
		{
			printf("初始化视频目录出错\n");
		}

		return 0;
	}

	/*
	* Fun:   初始化工程
	* Param: char *pPath 工程目录
	* Return:成功返回0，失败返回错误码
	*/
	int initEx(char *pPath)
	{
		if (nullptr == pPath)
			return -1;
		if (0 >= strlen(pPath))
			return -1;

		//保存工程目录
		if (512 <= strlen(pPath))
			strncpy(szPathProject, pPath, 512);
		else
			strcpy(szPathProject, pPath);

		//如果结尾是一个"\",就去掉
		if ('\\' == *(szPathProject + strlen(szPathProject) - 1))
			*(szPathProject + strlen(szPathProject) - 1) = 0;

		//获取工程的目录
		int i = (int)strlen(szPathProject) - 1;
		for (; i > 0; i--)
		{
			if ('\\' == *(szPathProject + i))
				break;
		}

		//组装数据目录
		sprintf(szPathRadar, "%s\\%s", szPathProject, DATA_PATH_RADAR);
		sprintf(szPathVideoFront, "%s\\%s", szPathProject, DATA_PATH_VIDEO_FRONT);
		sprintf(szPathVideoBack, "%s\\%s", szPathProject, DATA_PATH_VIDEO_BACK);
		sprintf(szPathVideoLeft, "%s\\%s", szPathProject, DATA_PATH_VIDEO_LEFT);
		sprintf(szPathVideoRight, "%s\\%s", szPathProject, DATA_PATH_VIDEO_RIGHT);

		//检查目录是否存在
		if (!Utils::checkExistDir(szPathProject))
		{
			printf("工程目录%s不存在\n", szPathProject);
			return -2;
		}
		//检查雷达数据目录是否存在
		if (!Utils::checkExistDir(szPathRadar))
		{
			printf("工程目录%s中，不存在雷达数据目录%s\n", szPathProject, szPathRadar);
			return -3;
		}
		//检查抓拍视频目录是否存在(前置 抓拍机)
		if (!Utils::checkExistDir(szPathVideoFront))
		{
			printf("工程目录%s中，不存在前置抓拍视频目录%s\n", szPathProject, szPathVideoFront);
		}
		//检查抓拍视频目录是否存在(前置 抓拍机)
		if (!Utils::checkExistDir(szPathVideoBack))
		{
			printf("工程目录%s中，不存在后置抓拍视频目录%s\n", szPathProject, szPathVideoBack);
		}
		//		=====================

				//检查抓拍视频目录是否存在(左置 抓拍机)
		if (!Utils::checkExistDir(szPathVideoLeft))
		{
			printf("工程目录%s中，不存在左置抓拍视频目录%s\n", szPathProject, szPathVideoLeft);
		}
		//检查抓拍视频目录是否存在(右置 抓拍机)
		if (!Utils::checkExistDir(szPathVideoRight))
		{
			printf("工程目录%s中，不存在右置抓拍视频目录%s\n", szPathProject, szPathVideoRight);
		}

		char szPathTmp[512] = { 0 };
		sprintf(szPathTmp, "%s\\*.ord", szPathRadar);
		//搜索雷达目录下，测线数据，用于初始化所有测线      //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		intptr_t hFile;
		struct _finddata_t oFileInfo;
		if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
		{
			printf("没有发现工程的测线数据\n");
			return -4;
		}

		//清除所有已經存在的Swath数据
		clearAllSwath();

		//初始化所有新的测线数据
		Swath * pSwath = new Swath();
		char szSwathName[128] = { 0 };
		parseSwathName(szSwathName, oFileInfo.name);

		int iResult = pSwath->init(szPathRadar, szSwathName);
		if (0 != iResult)
		{
			delete pSwath;
			return iResult;
		}

		m_lstSwath.push_back(pSwath);
		while (_findnext(hFile, &oFileInfo) == 0)
		{
			pSwath = new Swath();
			//dropExtName(oFileInfo.name);
			parseSwathName(szSwathName, oFileInfo.name);
			iResult = pSwath->init(szPathRadar, szSwathName);
			if (0 != iResult)
			{
				//char szOut[1024] = { 0 };
				//sprintf(szOut, "初始化Swath失败，该Swath未加载，szSwathName:%s; FileInfo:%s; iResult:%d; szPathProject:%s\n", szSwathName, oFileInfo.name, iResult, szPathProject);
				//MessageBox(NULL, szOut, szSwathName, MB_OK);
				printf("初始化Swath失败，该Swath未加载，Swath：%s\n", oFileInfo.name);
				delete pSwath;
				continue;
			}
			m_lstSwath.push_back(pSwath);
		}

		_findclose(hFile);

		//初始化视频对象，前置抓拍机
		if (0 != m_oCameraFront.init(szPathVideoFront))
		{
			printf("初始化视频目录出错\n");
		}
		//初始化视频对象，后置抓拍机
		if (0 != m_oCameraBack.init(szPathVideoBack))
		{
			printf("初始化视频目录出错\n");
		}
		//初始化视频对象，左置抓拍机
		if (0 != m_oCameraLeft.init(szPathVideoLeft))
		{
			printf("初始化视频目录出错\n");
		}
		//初始化视频对象，右置抓拍机
		if (0 != m_oCameraRight.init(szPathVideoRight))
		{
			printf("初始化视频目录出错\n");
		}

		return 0;
	}

	/*
	* Fun :逆初始化工程
	* Para:無
	* Return:無
	*/
	void unInit()
	{
		//清除所有Swath数据
		clearAllSwath();

		m_oCameraFront.unInit();
		m_oCameraBack.unInit();
		m_oCameraLeft.unInit();
		m_oCameraRight.unInit();
	}

	/*
	* Fun:   读取Swath信息
	* Param: char *pSwathName 测线名
	* Return:成功返回Swath对象，失败返回NULL；
	*/
	Swath* getSwath( char *pSwathName )
	//int getSwath(char *pSwathName)
	{
		std::vector<Swath*>::iterator  it_pos;
		//遍历，释放
		for (it_pos = m_lstSwath.begin(); it_pos != m_lstSwath.end(); it_pos++)
		{
			Swath* pSwath = *it_pos;
			if (NULL == pSwath)
			{
				//return 223;
				continue;
			}

			if (0 == strcmp(pSwathName, pSwath->getSwathID()))
			{
				//return 123;
				return pSwath;
			}
		}
		//return 323;
		return NULL;
	}

	/*
	* Fun:   读取所有测线列表
	* Return:成功返回0，失败返回错误码
	*/
	int getAllSwathName(std::vector<char*> &lstSwath)
	{
		std::vector<Swath*>::iterator  it_pos;
		//遍历，释放
		for (it_pos = m_lstSwath.begin(); it_pos != m_lstSwath.end(); it_pos++)
		{
			Swath* pSwath = *it_pos;
			if (NULL == pSwath)
				continue;

			lstSwath.push_back( pSwath->getSwathID() );
		}

		return 0;
	}

	/*
	* Fun:获取视频查询对象，前置抓拍机
	* Param:无
	* Return:成功返回视频查询对象，失败返回空
	*/
	CameraPhoto* getCameraFront()
	{
		return &m_oCameraFront;
	}

	/*
	* Fun:获取视频查询对象，后置抓拍机
	* Param:无
	* Return:成功返回视频查询对象，失败返回空
	*/
	CameraPhoto* getCameraBack()
	{
		return &m_oCameraBack;
	}

	/*
	* Fun:获取视频查询对象，左置抓拍机
	* Param:无
	* Return:成功返回视频查询对象，失败返回空
	*/
	CameraPhoto* getCameraLeft()
	{
		return &m_oCameraLeft;
	}

	/*
	* Fun:获取视频查询对象，右置抓拍机
	* Param:无
	* Return:成功返回视频查询对象，失败返回空
	*/
	CameraPhoto* getCameraRight()
	{
		return &m_oCameraRight;
	}

	/*
	* Fun:获取工程路径
	* Param:无
	* Return:返回工程路径
	*/
	char * getProjectPath()
	{
		return szPathProject;
	}
private:
	//删除文件名中的扩展名，取得文件名作为作为SwathID
	void dropExtName(char *pFileName)
	{
		int i = ( int )strlen(pFileName) - 1;
		for (; i >= 0; i--)
		{
			if ('.' == *(pFileName + i))
			{
				*(pFileName + i) = 0;
				break;
			}
		}
	}
	//使用文件名，解析测线名称
	void parseSwathName(char* pSwathName , char* pFileName)
	{
		int i = (int)strlen(pFileName) - 1;
		for (; i >= 0; i--)
		{
			if ('_' == *(pFileName + i))
			{
				break;
			}
		}
		strncpy(pSwathName, pFileName, i);
	}

	//清除所有已經存在的Swath数据
	void clearAllSwath()
	{
		std::vector<Swath*>::iterator  it_pos;
		//遍历，释放
		for (it_pos = m_lstSwath.begin(); it_pos != m_lstSwath.end(); it_pos++)
		{
			Swath * pSwath = *it_pos;
			if (NULL != pSwath)
			{
				delete pSwath;
				pSwath = NULL;
			}
		}
		m_lstSwath.clear();
	}

	void findAllSwath(const char * szPathRadar)
	{
		//清除所有已經存在的Swath名称
		m_lstSwathName.clear();

		char szPathTmp[512] = { 0 };
		sprintf(szPathTmp, "%s\\*_A01.iprh", szPathRadar);
		//搜索雷达目录下，测线数据，用于初始化所有测线      //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		intptr_t hFile;
		struct _finddata_t oFileInfo;
		if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
		{
			printf("没有发现工程的测线数据\n");
			return;
		}

		//初始化所有新的测线数据
		char *szSwathName = (char *)malloc(64);
		memset(szSwathName, 0 , 64);
		parseSwathName(szSwathName, oFileInfo.name);
		if (szSwathName && (strlen(szSwathName) > 0))
			m_lstSwathName.push_back(szSwathName);

		while (_findnext(hFile, &oFileInfo) == 0)
		{
			szSwathName = (char *)malloc(64);
			memset(szSwathName, 0, 64);
			parseSwathName(szSwathName, oFileInfo.name);

			if (szSwathName && (strlen(szSwathName) > 0))
				m_lstSwathName.push_back(szSwathName);
        }

		_findclose(hFile);
	}
private:
	const char *DATA_PATH_RADAR       = "RadarData";
	const char *DATA_PATH_VIDEO_FRONT = "CameraFront";
	const char *DATA_PATH_VIDEO_BACK  = "CameraBack";
	const char *DATA_PATH_VIDEO_LEFT  = "CameraLeft";
	const char *DATA_PATH_VIDEO_RIGHT = "CameraRight";
	const char *DATA_PATH_RADAR_PIC   = "RadarPic";

	char szPathProject[512]    = { 0 };
	char szPathRadar[512]      = { 0 };
	char szPathVideoFront[512] = { 0 };
	char szPathVideoBack[512]  = { 0 };
	char szPathVideoLeft[512]  = { 0 };
	char szPathVideoRight[512] = { 0 };

	std::vector<Swath *> m_lstSwath;
	CameraPhoto m_oCameraFront;
	CameraPhoto m_oCameraBack;
	CameraPhoto m_oCameraLeft;
	CameraPhoto m_oCameraRight;

	std::vector<char *> m_lstSwathName;
};

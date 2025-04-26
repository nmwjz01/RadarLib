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
		//�������Swath����
		clearAllSwath();
	}

	/*
	* Fun:   ��ʼ������
	* Param: char *pPath ����Ŀ¼
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	int init( char *pPath )
	{
		if (nullptr == pPath)
			return -1;
		if (0 >= strlen(pPath))
			return -1;

		//���湤��Ŀ¼
		if (512 <= strlen(pPath))
			strncpy(szPathProject, pPath,512);
		else
			strcpy(szPathProject, pPath);

		//�����β��һ��"\",��ȥ��
		if ('\\' == *(szPathProject + strlen(szPathProject) - 1))
			*(szPathProject + strlen(szPathProject) - 1) = 0;

		//��ȡ���̵�Ŀ¼
		int i = ( int )strlen(szPathProject) - 1;
		for (; i > 0; i--)
		{
			if ('\\' == *(szPathProject + i))
				break;
		}

		//��װ����Ŀ¼
		sprintf(szPathRadar     , "%s\\%s", szPathProject, DATA_PATH_RADAR);
		sprintf(szPathVideoFront, "%s\\%s", szPathProject, DATA_PATH_VIDEO_FRONT);
		sprintf(szPathVideoBack,  "%s\\%s", szPathProject, DATA_PATH_VIDEO_BACK);
		sprintf(szPathVideoLeft,  "%s\\%s", szPathProject, DATA_PATH_VIDEO_LEFT);
		sprintf(szPathVideoRight, "%s\\%s", szPathProject, DATA_PATH_VIDEO_RIGHT);

		//���Ŀ¼�Ƿ����
		if (!Utils::checkExistDir(szPathProject) )
		{
			printf("����Ŀ¼%s������\n", szPathProject);
			return -2;
		}
		//����״�����Ŀ¼�Ƿ����
		if (!Utils::checkExistDir(szPathRadar))
		{
			printf("����Ŀ¼%s�У��������״�����Ŀ¼%s\n", szPathProject, szPathRadar);
			return -3;
		}
		//���ץ����ƵĿ¼�Ƿ����(ǰ�� ץ�Ļ�)
		if (!Utils::checkExistDir(szPathVideoFront))
		{
			printf("����Ŀ¼%s�У�������ǰ��ץ����ƵĿ¼%s\n", szPathProject, szPathVideoFront);
		}
		//���ץ����ƵĿ¼�Ƿ����(ǰ�� ץ�Ļ�)
		if (!Utils::checkExistDir(szPathVideoBack))
		{
			printf("����Ŀ¼%s�У������ں���ץ����ƵĿ¼%s\n", szPathProject, szPathVideoBack);
		}
//		=====================

		//���ץ����ƵĿ¼�Ƿ����(���� ץ�Ļ�)
		if (!Utils::checkExistDir(szPathVideoLeft))
		{
			printf("����Ŀ¼%s�У�����������ץ����ƵĿ¼%s\n", szPathProject, szPathVideoLeft);
		}
		//���ץ����ƵĿ¼�Ƿ����(���� ץ�Ļ�)
		if (!Utils::checkExistDir(szPathVideoRight))
		{
			printf("����Ŀ¼%s�У�����������ץ����ƵĿ¼%s\n", szPathProject, szPathVideoRight);
		}

		//��������ѽ����ڵ�Swath����
		clearAllSwath();

		//��ȡ���в����������ҷŵ�vector��
		findAllSwath(szPathRadar);

		int iResult = 0;
		//��ʼ��ÿ������
		std::vector<char *>::iterator  it_pos;
		//������ÿ����������Ȼ���ʼ��
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

		//��ʼ����Ƶ����ǰ��ץ�Ļ�
		if (0 != m_oCameraFront.init(szPathVideoFront))
		{
			printf("��ʼ����ƵĿ¼����\n");
		}
		//��ʼ����Ƶ���󣬺���ץ�Ļ�
		if (0 != m_oCameraBack.init(szPathVideoBack))
		{
			printf("��ʼ����ƵĿ¼����\n");
		}
		//��ʼ����Ƶ��������ץ�Ļ�
		if (0 != m_oCameraLeft.init(szPathVideoLeft))
		{
			printf("��ʼ����ƵĿ¼����\n");
		}
		//��ʼ����Ƶ��������ץ�Ļ�
		if (0 != m_oCameraRight.init(szPathVideoRight))
		{
			printf("��ʼ����ƵĿ¼����\n");
		}

		return 0;
	}

	/*
	* Fun:   ��ʼ������
	* Param: char *pPath ����Ŀ¼
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	int initEx(char *pPath)
	{
		if (nullptr == pPath)
			return -1;
		if (0 >= strlen(pPath))
			return -1;

		//���湤��Ŀ¼
		if (512 <= strlen(pPath))
			strncpy(szPathProject, pPath, 512);
		else
			strcpy(szPathProject, pPath);

		//�����β��һ��"\",��ȥ��
		if ('\\' == *(szPathProject + strlen(szPathProject) - 1))
			*(szPathProject + strlen(szPathProject) - 1) = 0;

		//��ȡ���̵�Ŀ¼
		int i = (int)strlen(szPathProject) - 1;
		for (; i > 0; i--)
		{
			if ('\\' == *(szPathProject + i))
				break;
		}

		//��װ����Ŀ¼
		sprintf(szPathRadar, "%s\\%s", szPathProject, DATA_PATH_RADAR);
		sprintf(szPathVideoFront, "%s\\%s", szPathProject, DATA_PATH_VIDEO_FRONT);
		sprintf(szPathVideoBack, "%s\\%s", szPathProject, DATA_PATH_VIDEO_BACK);
		sprintf(szPathVideoLeft, "%s\\%s", szPathProject, DATA_PATH_VIDEO_LEFT);
		sprintf(szPathVideoRight, "%s\\%s", szPathProject, DATA_PATH_VIDEO_RIGHT);

		//���Ŀ¼�Ƿ����
		if (!Utils::checkExistDir(szPathProject))
		{
			printf("����Ŀ¼%s������\n", szPathProject);
			return -2;
		}
		//����״�����Ŀ¼�Ƿ����
		if (!Utils::checkExistDir(szPathRadar))
		{
			printf("����Ŀ¼%s�У��������״�����Ŀ¼%s\n", szPathProject, szPathRadar);
			return -3;
		}
		//���ץ����ƵĿ¼�Ƿ����(ǰ�� ץ�Ļ�)
		if (!Utils::checkExistDir(szPathVideoFront))
		{
			printf("����Ŀ¼%s�У�������ǰ��ץ����ƵĿ¼%s\n", szPathProject, szPathVideoFront);
		}
		//���ץ����ƵĿ¼�Ƿ����(ǰ�� ץ�Ļ�)
		if (!Utils::checkExistDir(szPathVideoBack))
		{
			printf("����Ŀ¼%s�У������ں���ץ����ƵĿ¼%s\n", szPathProject, szPathVideoBack);
		}
		//		=====================

				//���ץ����ƵĿ¼�Ƿ����(���� ץ�Ļ�)
		if (!Utils::checkExistDir(szPathVideoLeft))
		{
			printf("����Ŀ¼%s�У�����������ץ����ƵĿ¼%s\n", szPathProject, szPathVideoLeft);
		}
		//���ץ����ƵĿ¼�Ƿ����(���� ץ�Ļ�)
		if (!Utils::checkExistDir(szPathVideoRight))
		{
			printf("����Ŀ¼%s�У�����������ץ����ƵĿ¼%s\n", szPathProject, szPathVideoRight);
		}

		char szPathTmp[512] = { 0 };
		sprintf(szPathTmp, "%s\\*.ord", szPathRadar);
		//�����״�Ŀ¼�£��������ݣ����ڳ�ʼ�����в���      //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		intptr_t hFile;
		struct _finddata_t oFileInfo;
		if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
		{
			printf("û�з��ֹ��̵Ĳ�������\n");
			return -4;
		}

		//��������ѽ����ڵ�Swath����
		clearAllSwath();

		//��ʼ�������µĲ�������
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
				//sprintf(szOut, "��ʼ��Swathʧ�ܣ���Swathδ���أ�szSwathName:%s; FileInfo:%s; iResult:%d; szPathProject:%s\n", szSwathName, oFileInfo.name, iResult, szPathProject);
				//MessageBox(NULL, szOut, szSwathName, MB_OK);
				printf("��ʼ��Swathʧ�ܣ���Swathδ���أ�Swath��%s\n", oFileInfo.name);
				delete pSwath;
				continue;
			}
			m_lstSwath.push_back(pSwath);
		}

		_findclose(hFile);

		//��ʼ����Ƶ����ǰ��ץ�Ļ�
		if (0 != m_oCameraFront.init(szPathVideoFront))
		{
			printf("��ʼ����ƵĿ¼����\n");
		}
		//��ʼ����Ƶ���󣬺���ץ�Ļ�
		if (0 != m_oCameraBack.init(szPathVideoBack))
		{
			printf("��ʼ����ƵĿ¼����\n");
		}
		//��ʼ����Ƶ��������ץ�Ļ�
		if (0 != m_oCameraLeft.init(szPathVideoLeft))
		{
			printf("��ʼ����ƵĿ¼����\n");
		}
		//��ʼ����Ƶ��������ץ�Ļ�
		if (0 != m_oCameraRight.init(szPathVideoRight))
		{
			printf("��ʼ����ƵĿ¼����\n");
		}

		return 0;
	}

	/*
	* Fun :���ʼ������
	* Para:�o
	* Return:�o
	*/
	void unInit()
	{
		//�������Swath����
		clearAllSwath();

		m_oCameraFront.unInit();
		m_oCameraBack.unInit();
		m_oCameraLeft.unInit();
		m_oCameraRight.unInit();
	}

	/*
	* Fun:   ��ȡSwath��Ϣ
	* Param: char *pSwathName ������
	* Return:�ɹ�����Swath����ʧ�ܷ���NULL��
	*/
	Swath* getSwath( char *pSwathName )
	//int getSwath(char *pSwathName)
	{
		std::vector<Swath*>::iterator  it_pos;
		//�������ͷ�
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
	* Fun:   ��ȡ���в����б�
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	int getAllSwathName(std::vector<char*> &lstSwath)
	{
		std::vector<Swath*>::iterator  it_pos;
		//�������ͷ�
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
	* Fun:��ȡ��Ƶ��ѯ����ǰ��ץ�Ļ�
	* Param:��
	* Return:�ɹ�������Ƶ��ѯ����ʧ�ܷ��ؿ�
	*/
	CameraPhoto* getCameraFront()
	{
		return &m_oCameraFront;
	}

	/*
	* Fun:��ȡ��Ƶ��ѯ���󣬺���ץ�Ļ�
	* Param:��
	* Return:�ɹ�������Ƶ��ѯ����ʧ�ܷ��ؿ�
	*/
	CameraPhoto* getCameraBack()
	{
		return &m_oCameraBack;
	}

	/*
	* Fun:��ȡ��Ƶ��ѯ��������ץ�Ļ�
	* Param:��
	* Return:�ɹ�������Ƶ��ѯ����ʧ�ܷ��ؿ�
	*/
	CameraPhoto* getCameraLeft()
	{
		return &m_oCameraLeft;
	}

	/*
	* Fun:��ȡ��Ƶ��ѯ��������ץ�Ļ�
	* Param:��
	* Return:�ɹ�������Ƶ��ѯ����ʧ�ܷ��ؿ�
	*/
	CameraPhoto* getCameraRight()
	{
		return &m_oCameraRight;
	}

	/*
	* Fun:��ȡ����·��
	* Param:��
	* Return:���ع���·��
	*/
	char * getProjectPath()
	{
		return szPathProject;
	}
private:
	//ɾ���ļ����е���չ����ȡ���ļ�����Ϊ��ΪSwathID
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
	//ʹ���ļ�����������������
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

	//��������ѽ����ڵ�Swath����
	void clearAllSwath()
	{
		std::vector<Swath*>::iterator  it_pos;
		//�������ͷ�
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
		//��������ѽ����ڵ�Swath����
		m_lstSwathName.clear();

		char szPathTmp[512] = { 0 };
		sprintf(szPathTmp, "%s\\*_A01.iprh", szPathRadar);
		//�����״�Ŀ¼�£��������ݣ����ڳ�ʼ�����в���      //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		intptr_t hFile;
		struct _finddata_t oFileInfo;
		if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
		{
			printf("û�з��ֹ��̵Ĳ�������\n");
			return;
		}

		//��ʼ�������µĲ�������
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

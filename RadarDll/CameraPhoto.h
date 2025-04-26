#pragma once

#include <io.h>
#include <vector>

class CameraPhoto
{
public:
	CameraPhoto() {}
	~CameraPhoto()
	{
		//�ͷſռ�
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

		//���Ŀ¼�Ƿ����
		struct stat oDirState;
		if (-1 == stat(szPath, &oDirState))
		{
			printf("��Ƶץ��Ŀ¼%s������\n", szPath);
			return -2;
		}

		//
		char szPathFile[512] = { 0 };
		sprintf(szPathFile, "%s\\*.*", szPath);
		//����ץ��Ŀ¼�£�������Ƶ�ļ���������ƥ���״�����      //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		intptr_t hFile;
		struct _finddata_t oFileInfo;
		if ((hFile = (intptr_t)_findfirst(szPathFile, &oFileInfo)) == -1L)
		{
			printf("û�з�����Ƶ�ļ�����\n");
			return 0;
		}

		//��ʼ�������ļ���
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

	//���ʼ��
	void unInit()
	{
		//�ͷſռ�
		for (int i = 0; i < (int)m_lstFileName.size(); i++)
		{
			char *p = m_lstFileName[i];
			free(p);
		}
		m_lstFileName.clear();
	}

	//ͨ��ʱ���ȡ������ʱ���ͼƬ 
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
		//ѭ���Ƚ�
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

	//����·��
	char *getPath()
	{
		return szPath;
	}

private:
	void transferDateTime(char * pDateTme)
	{
		char *pTmp = pDateTme;
		/*
		//��
		*(pTmp + 4) = *(pTmp + 5);
		*(pTmp + 5) = *(pTmp + 6);

		//��
		*(pTmp + 6) = *(pTmp + 8);
		*(pTmp + 7) = *(pTmp + 9);

		*(pTmp + 8) = '_';

		//ʱ
		*(pTmp + 9)  = *(pTmp + 11);
		*(pTmp + 10) = *(pTmp + 12);

		//��
		*(pTmp + 11) = *(pTmp + 14);
		*(pTmp + 12) = *(pTmp + 15);

		//��
		*(pTmp + 13) = *(pTmp + 17);
		*(pTmp + 14) = *(pTmp + 18);

		*(pTmp + 15) = '.';

		//����
		*(pTmp + 16) = *(pTmp + 20);
		*(pTmp + 17) = *(pTmp + 21);
		*(pTmp + 18) = *(pTmp + 22);

		*(pTmp + 19) = 0;
		*/

		//��
		*(pTmp + 13) = *(pTmp + 14);
		*(pTmp + 14) = *(pTmp + 15);

		//��
		*(pTmp + 15) = *(pTmp + 17);
		*(pTmp + 16) = *(pTmp + 18);

		*(pTmp + 17) = '.';

		//����
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
	//ץ��ͼƬ���Ŀ¼
	char szPath[512] = { 0 };
	std::vector<char *> m_lstFileName;
};

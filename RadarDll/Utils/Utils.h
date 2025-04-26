#pragma once

#include <time.h>
#include <strstream>

class Utils
{
public:
	//�������ַ���ת��Ϊ����
	static int string2int(char * pText)
	{
		int iNum = 0;
		int iLen = ( int )strlen(pText);

		for (int i = 0; i < iLen; i++)
		{
			iNum = iNum * 10 + pText[i] - '0';
		}
		return iNum;
	}

	/*
	* Fun:���ַ�����ʱ��(��ʽ HH:MM:SS:sss)ת��Ϊ����ʱ�䣬��ȷ������
	* Param: 
	*      char * pDateTime ��������� �ַ�����ʽ��ʱ��
	*      long &lTime      ��������� ���뼶���ʱ��
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	static int time2long(char * pDateTime, long &lTime)
	{
		//���Ȳ�����ʱ��
		if (12 != strlen( pDateTime ))
			return -1;

		//�����ַ���ʱ���ÿ������
		char szHour[2]     = { 0 };
		char szMinute[2]   = { 0 };
		char szSecond[2]   = { 0 };
		char szMilliSec[3] = { 0 };

		//�����봮�н�ȡʱ���ÿ������
		strncpy(szHour    , pDateTime, 2);  pDateTime = pDateTime + 3;
		strncpy(szMinute  , pDateTime, 2);  pDateTime = pDateTime + 3;
		strncpy(szSecond  , pDateTime, 2);  pDateTime = pDateTime + 3;
		strncpy(szMilliSec, pDateTime, 3);

		//����ʱ���ÿ������
		int iHour     = string2int(szHour    );
		int iMinute   = string2int(szMinute  );
		int iSecond   = string2int(szSecond  );
		int iMilliSec = string2int(szMilliSec);

		//���㷵��ʱ��
		lTime = (((iHour * 60) + iMinute) * 60 + iSecond) * 1000 + iMilliSec;

		return 0;
	}

	/*
	* Fun:������ʱ��ת��Ϊ �ַ�����ʱ��(��ʽ YYYY-MM-DD HH:MM:SS:sss)����ȷ������
	* Param:
	*      long lTime      ��������� ���뼶���ʱ��
	*      char * pDateTime ��������� �ַ�����ʽ������ʱ��
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	static int long2date(long long lTime, char * pDateTime)
	{
		if (!pDateTime)
			return -1;

		int msecond = lTime % 1000;

		lTime = lTime/1000;
		tm *tm_ = localtime(&lTime);
		sprintf(pDateTime, "%4d-%02d-%02d	%02d:%02d:%02d:%03d", tm_->tm_year + 1900, tm_->tm_mon + 1, tm_->tm_mday, tm_->tm_hour, tm_->tm_min, tm_->tm_sec, msecond);

		return 0;
	}

	/*
	* Fun:���ַ�����ʱ��(��ʽ YYYY-MM-DD HH:MM:SS:sss)ת��Ϊ����ʱ�䣬��ȷ������
	* Param:
	*      char * pDateTime ��������� �ַ�����ʽ������ʱ��
	*      long &lTime      ��������� ���뼶���ʱ��
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	static int date2long(char * pDateTime, long long &lTime)
	{
		if (!pDateTime)
			return -1;
		struct tm tm_;
		int year, month, day, hour, minute, second, msecond;
		if (-1 == sscanf(pDateTime, "%d-%d-%d	%d:%d:%d:%d", &year, &month, &day, &hour, &minute, &second, &msecond))
			return -1;
		tm_.tm_year = year - 1900;
		tm_.tm_mon  = month - 1;
		tm_.tm_mday = day;
		tm_.tm_hour = hour;
		tm_.tm_min  = minute;
		tm_.tm_sec  = second;
		tm_.tm_isdst = 0;

		//time_t t_ = mktime(&tm_); //�Ѿ�����8��ʱ�� 
		lTime  = mktime(&tm_) * 1000 + msecond; //�Ѿ�����8��ʱ��
		return 0; //����ʱ��
	}

	/**
	* Fun:���������������ڴ��ַ����ж�ȡͨ��������Ϣ
	*     ��ȡ�ַ���������Ϣ    ReadParameterStr
	*     ��ȡ����������Ϣ      ReadParameterInt
	*     ��ȡdouble��������Ϣ  ReadParameterDouble
	*/
	//��ȡһ���ַ���������Ϣ
	static int ReadParameterStr(char* s, const char* sp, char szData[] )
	{
		char s1[200];
		std::istrstream iss(s + strlen(sp));
		if (strncmp(s, sp, strlen(sp)) == 0)
		{
			iss.getline(s1, 255);
			strncpy(szData, s1, strlen(s)-strlen(sp));
		}
		//�����ͷ�ǿո�ð�š�tab��������
		if ((' ' == szData[0]) || ('	' == szData[0]) || (':' == szData[0]))
		{
			char *tmp = szData + 1;
			strcpy(szData, tmp);
		}

		return 0;
	}
	//��ȡһ������������Ϣ
	static int ReadParameterInt(char* s, char* sp, int& t)
	{
		std::istrstream iss(s + strlen(sp));
		if (strncmp(s, sp, strlen(sp)) == 0)
		{
			iss >> t;
		}

		return 0;
	}
	//��ȡһ��double������Ϣ
	static int ReadParameterDouble(char* s, char* sp, double& t)
	{
		std::istrstream iss(s + strlen(sp));
		if (strncmp(s, sp, strlen(sp)) == 0)
		{
			iss >> t;
		}

		return 0;
	}

	/**
	* Fun:�ж�Ŀ¼�Ƿ����
	* Param: pPath  Ŀ¼·��
	* Return: ���ڷ���true�������ڷ���false
	*/
	static bool checkExistDir( char *pPath )
	{
		//���Ŀ¼�Ƿ����
		struct stat oDirState;
		if (-1 == stat(pPath, &oDirState))
			return false;
		else
			return true;
	}
	/**
	* Fun:�ж��ļ��Ƿ����
	* Param: pPath  �ļ�·��������
	* Return: ���ڷ���true�������ڷ���false
	*/
	static bool checkExistFile(char* pPathName)
	{
		FILE* pFP = fopen(pPathName, "r");
		if (nullptr == pFP)
			return false;
		else
		{
			fclose(pFP);
			return true;
		}
	}

	/**
	* Fun:�������ļ��е�һ�У�����Ŀ������
	* Param: src       һ������������
	*        fN        Ŀ������������
	*        separator ������֮��ķָ���
	*        res       �������
	* Return:�����ɹ�����0
	*/
	static int findField(const char* src, int fN, char separator, char* res)
	{
		int i, j = 0;
		res[0] = 0;
		//
		for (i = 0; i < fN; i++)
		{
			while (src[j] && src[j] != separator) j++;
			if (src[j] == 0) return -1;
			j++;
		}
		i = 0;
		while (src[j] && src[j] != separator)
		{
			res[i++] = src[j];
			j++;
		}
		if (i == 0) return -2;
		res[i] = 0;
		return 0;
	}


	/*
	* Fun:���ַ����е������D��Ϊ����
	* Param: str ����Ď������ֵ��ַ���
	*        out �D���������
	*/
	static void char2int(char *str, long long &out)
	{
		if (!str)
			return;

		int i = 0;
		//ѭ�h�D��ÿһ������
		while ( true )
		{
			if ((str[i] > '9') || (str[i] < '0') )
				break;

			out = out * 10 + str[i] - '0';
			i++;
		}
	}

   /*
   * Fun:��Ŀ¼����Ѱ�����һ��Ŀ¼
   * Param: path �����ԭʼĿ¼
   * Return: Ŀ��Ŀ¼
   */
	static char * findLastPath(char * path)
	{
		if (!path)
			return NULL;
		if (0 == strlen(path))
			return path;

		//������һ���ַ���б��(������б�ܺ���б��)������Ҫȥ��
		int end =(int) strlen(path) - 1;
		if( ( 92 == *(path+end) ) || ( 47 == *(path+end) ) )
		{
			*(path + end) = 0;
		}

		//��б�ܿ�ʼλ��
		char *start1 = path;

		char *tmp   = path;
		//Ѱ�����һ����б��
		while (true)
		{
			tmp = strstr(tmp, "/");
			if (tmp)
			{
				tmp++;
				start1 = tmp;
			}

			if (!tmp)
				break;
		}

		//��б�ܿ�ʼλ��
		char *start2 = path;

		tmp = path;
		//Ѱ�����һ����б��
		while (true)
		{
			tmp = strstr(tmp, "\\");
			if (tmp)
			{
				tmp++;
				start2 = tmp;
			}

			if (!tmp)
				break;
		}
		char *ret = start2 > start1? start2 : start1;
		return ret;
	}

	/*
	* Fun:��һ���򿪵��ļ�����ȡһ���ı�
	* Param:  fd   �ļ����
	*         buff ��ŷ������ݵ�Buff
	*         len  Buff�ĳ���
	* Return: �ɹ��������ݳ��ȣ�ʧ�ܷ���0��û�ж�ȡ�����ݷ���0��
	*/
	static int findLastPath2(FILE * fd, char * buff, int len)
	{
		if ((NULL == fd) || (NULL == buff))
			return -1;
		if (len <= 0)
			return -2;

		//��¼�ַ����Ŀ�ʼ
		char * start = buff;
		//��¼��ȡ�ĳ���
		int count = 0;
		//ѭ����ȡ
		while (!feof(fd))
		{
			if (count >= len)
				break;

			fread(buff, 1, 1, fd);

			buff++;
			*buff = 0;
			count++;

			if (strstr(start, "\n"))
				break;
		}

		return count;
	}
};

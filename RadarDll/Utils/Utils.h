#pragma once

#include <time.h>
#include <strstream>

class Utils
{
public:
	//将数字字符串转化为整数
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
	* Fun:将字符串的时间(格式 HH:MM:SS:sss)转化为整数时间，精确到毫秒
	* Param: 
	*      char * pDateTime 输入参数， 字符串格式的时间
	*      long &lTime      输出参数， 毫秒级别的时间
	* Return:成功返回0，失败返回错误码
	*/
	static int time2long(char * pDateTime, long &lTime)
	{
		//长度不符合时间
		if (12 != strlen( pDateTime ))
			return -1;

		//定义字符串时间的每个部分
		char szHour[2]     = { 0 };
		char szMinute[2]   = { 0 };
		char szSecond[2]   = { 0 };
		char szMilliSec[3] = { 0 };

		//从输入串中截取时间的每个部分
		strncpy(szHour    , pDateTime, 2);  pDateTime = pDateTime + 3;
		strncpy(szMinute  , pDateTime, 2);  pDateTime = pDateTime + 3;
		strncpy(szSecond  , pDateTime, 2);  pDateTime = pDateTime + 3;
		strncpy(szMilliSec, pDateTime, 3);

		//计算时间的每个部分
		int iHour     = string2int(szHour    );
		int iMinute   = string2int(szMinute  );
		int iSecond   = string2int(szSecond  );
		int iMilliSec = string2int(szMilliSec);

		//计算返回时间
		lTime = (((iHour * 60) + iMinute) * 60 + iSecond) * 1000 + iMilliSec;

		return 0;
	}

	/*
	* Fun:将整数时间转化为 字符串的时间(格式 YYYY-MM-DD HH:MM:SS:sss)，精确到毫秒
	* Param:
	*      long lTime      输入参数， 毫秒级别的时间
	*      char * pDateTime 输出参数， 字符串格式的日期时间
	* Return:成功返回0，失败返回错误码
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
	* Fun:将字符串的时间(格式 YYYY-MM-DD HH:MM:SS:sss)转化为整数时间，精确到毫秒
	* Param:
	*      char * pDateTime 输入参数， 字符串格式的日期时间
	*      long &lTime      输出参数， 毫秒级别的时间
	* Return:成功返回0，失败返回错误码
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

		//time_t t_ = mktime(&tm_); //已经减了8个时区 
		lTime  = mktime(&tm_) * 1000 + msecond; //已经减了8个时区
		return 0; //毫秒时间
	}

	/**
	* Fun:下面三个方法用于从字符串中读取通道配置信息
	*     读取字符串配置信息    ReadParameterStr
	*     读取整型配置信息      ReadParameterInt
	*     读取double型配置信息  ReadParameterDouble
	*/
	//读取一行字符串配置信息
	static int ReadParameterStr(char* s, const char* sp, char szData[] )
	{
		char s1[200];
		std::istrstream iss(s + strlen(sp));
		if (strncmp(s, sp, strlen(sp)) == 0)
		{
			iss.getline(s1, 255);
			strncpy(szData, s1, strlen(s)-strlen(sp));
		}
		//如果开头是空格、冒号、tab，则跳过
		if ((' ' == szData[0]) || ('	' == szData[0]) || (':' == szData[0]))
		{
			char *tmp = szData + 1;
			strcpy(szData, tmp);
		}

		return 0;
	}
	//读取一行整数配置信息
	static int ReadParameterInt(char* s, char* sp, int& t)
	{
		std::istrstream iss(s + strlen(sp));
		if (strncmp(s, sp, strlen(sp)) == 0)
		{
			iss >> t;
		}

		return 0;
	}
	//读取一行double配置信息
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
	* Fun:判断目录是否存在
	* Param: pPath  目录路径
	* Return: 存在返回true，不存在返回false
	*/
	static bool checkExistDir( char *pPath )
	{
		//检查目录是否存在
		struct stat oDirState;
		if (-1 == stat(pPath, &oDirState))
			return false;
		else
			return true;
	}
	/**
	* Fun:判断文件是否存在
	* Param: pPath  文件路径和名称
	* Return: 存在返回true，不存在返回false
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
	* Fun:从配置文件中的一行，解析目标数据
	* Param: src       一行完整的输入
	*        fN        目标数据所在列
	*        separator 列与列之间的分隔符
	*        res       输出数据
	* Return:解析成功返回0
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
	* Fun:将字符串中的数字轉化为整数
	* Param: str 输入的帶有数字的字符串
	*        out 轉化后的整数
	*/
	static void char2int(char *str, long long &out)
	{
		if (!str)
			return;

		int i = 0;
		//循環轉化每一个数字
		while ( true )
		{
			if ((str[i] > '9') || (str[i] < '0') )
				break;

			out = out * 10 + str[i] - '0';
			i++;
		}
	}

   /*
   * Fun:在目录串中寻找最后一段目录
   * Param: path 输入的原始目录
   * Return: 目标目录
   */
	static char * findLastPath(char * path)
	{
		if (!path)
			return NULL;
		if (0 == strlen(path))
			return path;

		//如果最后一个字符是斜杠(包括左斜杠和右斜杠)，则需要去掉
		int end =(int) strlen(path) - 1;
		if( ( 92 == *(path+end) ) || ( 47 == *(path+end) ) )
		{
			*(path + end) = 0;
		}

		//右斜杠开始位置
		char *start1 = path;

		char *tmp   = path;
		//寻找最后一个右斜杠
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

		//左斜杠开始位置
		char *start2 = path;

		tmp = path;
		//寻找最后一个左斜杠
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
	* Fun:从一个打开的文件，读取一行文本
	* Param:  fd   文件句柄
	*         buff 存放返回数据的Buff
	*         len  Buff的长度
	* Return: 成功返回数据长度，失败返回0，没有读取到数据返回0；
	*/
	static int findLastPath2(FILE * fd, char * buff, int len)
	{
		if ((NULL == fd) || (NULL == buff))
			return -1;
		if (len <= 0)
			return -2;

		//记录字符串的开始
		char * start = buff;
		//记录读取的长度
		int count = 0;
		//循环读取
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

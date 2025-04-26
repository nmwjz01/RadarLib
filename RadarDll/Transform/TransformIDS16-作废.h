#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\\pch.h"
#include "..\\framework.h"
#include <atlimage.h>

class TransferIDSData16
{
public:
	TransferIDSData16()
	{
		//下面一组提供通道属性定义
		headVersion = 10;        // header version: 10
		dataVersion = 16;        // data version: 16-16b; 32-32b 
		strcpy(date     , "2023-10-22");    //测线操作日期
		strcpy(startTime, "00:00:00"  );    //测线开始时间
		strcpy(stopTime , "23:59:59"  );    //测线结束时间
		strcpy(ANTENNA  , "200 MHz"   );    //频率？
		separation  = 0.08;
		samples     = 512;        //一个trace中，采样数量
		runs        = 0;
		maxStacks   = 0;
		frequency   = 200;        // Sampling Frequency (MHz)又是一个频率？
		timeWindow  = 60;         //时间窗，波從發出返回的时间
		lastTrace   = 0;
		strcpy(trigSource, "wheel");    //
		intervalTime = 0.05;       // time interval (seconds)
		intervalDist = 0.2;        // real distance interval (meters)
		intervalUser = 0.05;       // user distance interval (meters)
		zeroLevel    = 0;
		soilVel      = 100;        // soil velocity (m/us)
		positioning  = 0;          // it is used for combined files only
		channels     = 0;
		memset(chanName, 0 , 16);      // channel name (like "T1-R1")
	}
	~TransferIDSData16()
	{
		printf("转化第一个测线文件出错\n");
	}

	/*
	 * Fun:转化data扩展名的数据为iprh/iprb
	 * Param: pathSrc文件存放目录
	 *        pathDst iprb文件存放目录
	 *        swathName 测线名
	 *        swathID   测线ID
	 * 返回:成功返回true，失败返回false
	 */
	bool transferData(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
	{
		char swathFile[128] = { 0 };
		//处理雷达目录下，指定文件(第一个IDS数据文件)
		sprintf(swathFile, "%s_Array01.data", swathName);
		//这里第一个data文件转化为iprb文件
		if (false == transferFirst(pathSrc, pathDst, swathFile, swathName))
		{
			printf("转化第一个测线文件出错\n");
			return false;
		}

		/* --------------------Array01和Array02处理交界--------------------*/
		//处理雷达目录下，指定文件(第二个IDS数据文件)
		sprintf(swathFile, "%s_Array02.data", swathName);
		//这里第一个data文件转化为iprb文件
		if (false == transferSecond(pathSrc, pathDst, swathFile, swathName))
		{
			printf("转化第二个测线文件出错\n");
			return false;
		}

		/* --------------------生成cor文件--------------------*/
		//生成cor文件
		if (false == makeFileCor(pathDst, swathName, swathID))
		{
			printf("产生cor文件出错\n");
			return false;
		}

		/* --------------------生成time文件-------------------*/
		if (false == makeFileTime(pathSrc, pathDst, swathName, swathID))
		{
			printf("产生time文件出错\n");
			return false;
		}

		return true;
	}


private:

	/*
	 * Fun:将Data格式的IDS文件转化为iprh和iprb
	 * Param: pathSrc文件存放目录
	 *        pathDst iprb文件存放目录
	 *        swathFile文件名称 ... ... 例如:Swath001_Array01.data
	 *        swathName测线名   ... ... 例如:Swath001
	 *        sample每个trace的样本数量
	 * 返回:成功返回true，失败返回false
	 * Example:Swath001_Array01.data -->Swath001_Array01_A01.iprb、Swath001_A02.iprb ... ... Swath001_A10.iprb
	 */
	bool transferFirst(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
	{
		//参数合法性判断
		if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
			return false;
		if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
			return false;

		//200M hz的道间距是8
		separation = 0.08;
		//200M hz的时间窗是60ns
		timeWindow = 60;

		//trace数量计数器
		lastTrace = 0;
		//输入的ids文件
		char pathIDSFile[1024] = { 0 };
		sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
		FILE *fpSrc = fopen(pathIDSFile, "rb");
		if (!fpSrc)
		{
			printf( "原文件(%s)不存在", pathIDSFile);
			return false;
		}

		char pathDstFile[1024] = { 0 };
		FILE *fpDst[11] = { NULL };
		for (int index = 1; index <= 10; index++)
		{
			//打开输出的文件
			sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
			fpDst[index] = fopen(pathDstFile, "wb");
		}

		int traceSize = samples * 2;
		char *szBuff = (char *)malloc(traceSize);

		//跳过36字节头信息
		fread(szBuff, 1, 36, fpSrc);

		while (!feof(fpSrc))
		{
			int buffSize = 0;         //记录读取/写入每个Trace时以及读取的总字节数字
			char *tmpStart = szBuff;  //记录读取/写入每个Trace时的开头位置

			buffSize = 0;
			//读取第一个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第一个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[1]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第二个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第二个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[2]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第三个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第三个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[3]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第四个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第四个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[4]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第五个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第五个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[5]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第六个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第六个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[6]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第七个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第七个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[7]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第八个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第八个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[8]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第九个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第九个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[9]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第十个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第十个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[10]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//测线数量计数
			lastTrace++;
		}

		free(szBuff);

		//关闭数据输入文件和数据输出文件(iprb)
		fclose(fpSrc);
		for (int index = 1; index <= 10; index++)
		{
			fclose(fpDst[index]);
		}

		//记录到Trace数量
		lastTrace--;

		//写入iprh头文件
		for (int index = 1; index <= 10; index++)
		{
			//写入iprh
			saveHeader( pathDst, swathName, index);
		}

		return true;
	}

	/*
	 * Fun:将Data格式的IDS文件转化为iprh和iprb
	 * Param: pathSrc文件存放目录
	 *        pathDst iprb文件存放目录
	 *        swathFile文件名称 ... ... 例如:Swath001_Array02.data
	 *        swathName测线名   ... ... 例如:Swath002
	 *        sample每个trace的样本数量
	 * 返回:成功返回true，失败返回false
	 * Example:Swath001_Array02.data -->Swath001_Array02_A01.iprb、Swath001_A11.iprb ... ... Swath001_A29.iprb
	 */
	bool transferSecond(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
	{
		//参数合法性判断
		if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
			return false;
		if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
			return false;

		//600M hz的道间距是8
		separation = 0.04;
		//600M hz的时间窗是80ns
		timeWindow = 80;

		//trace数量计数器
		lastTrace = 0;
		//输入的ids文件
		char pathIDSFile[1024] = { 0 };
		sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
		FILE *fpSrc = fopen(pathIDSFile, "rb");
		if (!fpSrc)
		{
			printf("原文件(%s)不存在", pathIDSFile);
			return false;
		}

		char pathDstFile[1024] = { 0 };
		FILE *fpDst[30] = { NULL };
		for (int index = 11; index <= 29; index++)
		{
			//打开输出的文件
			sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
			fpDst[index] = fopen(pathDstFile, "wb");
		}

		int traceSize = samples * 2;
		char *szBuff = (char *)malloc(traceSize);

		//跳过36字节头信息
		fread(szBuff, 1, 36, fpSrc);

		while (!feof(fpSrc))
		{
			int buffSizeOld = 0;
			int buffSize    = 0;         //记录读取/写入每个Trace时以及读取的总字节数字
			char *tmpStart  = szBuff;  //记录读取/写入每个Trace时的开头位置

			buffSize = 0;
			//读取第11个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第11个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[11]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[11]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第12个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第12个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[12]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[12]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第13个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第13个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[13]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[13]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第14个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第14个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[14]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[14]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第15个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第15个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[15]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[15]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第16个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第16个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[16]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[16]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第17个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第17个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[17]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[17]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第18个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第18个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[18]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[18]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第19个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第19个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[19]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[19]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}


			//读取第20个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第20个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[20]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[20]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第21个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第21个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[21]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[21]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第22个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第22个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[22]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[22]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第23个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第23个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[23]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[23]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第24个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第24个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[24]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[24]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第25个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第25个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[25]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[25]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第26个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第26个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[26]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[26]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第27个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第27个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[27]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[27]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第28个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第28个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[28]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[28]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//读取第29个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第29个通道的数据
			buffSizeOld = buffSize;
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[29]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
			//再次写入一次，因为11-29通道的数据是1-10的一半
			tmpStart = szBuff;
			while (buffSizeOld > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[29]);
				buffSizeOld = buffSizeOld - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

			//测线数量计数
			lastTrace++;
		}

		free(szBuff);

		//关闭
		fclose(fpSrc);
		for (int index = 11; index <= 29; index++)
		{
			fclose(fpDst[index]);
		}

		//记录到Trace数量
		lastTrace--;
		lastTrace = lastTrace * 2;

		strcpy(ANTENNA, "600 MHz");    //频率？
		frequency = 600;        // Sampling Frequency (MHz)又是一个频率？

		//写入iprh头文件
		for (int index = 11; index <= 29; index++)
		{
			//写入iprh
			saveHeader(pathDst, swathName, index);
		}

		return true;
	}

	/*
	 * Fun:产生cor文件
	 * Param: pathDst文件存放目录
	 *        swathName 测线名
	 *        swathID 测线ID
	 * 返回:成功返回true，失败返回false
	 */
	bool makeFileCor(const char * pathDst, const char *swathName, int swathID)
	{
		char szSwathFile[512] = { 0 };
		sprintf(szSwathFile, "%s\\Swath%03d.cor", pathDst, swathID);

		FILE *fp = fopen(szSwathFile, "w+");

		char szContext[1024] = { 0 };
		sprintf(szContext, "%s", "1	2022-09-28	07:20:45:601	22.57101145217	N	113.93479769317	E	31.531	M	1\n");

		fwrite(szContext, 1, strlen(szContext), fp);

		fclose(fp);

		return true;
	}

	/*
	 * Fun:产生time文件
	 * Param: pathDst文件存放目录
	 *        swathName 测线名
	 *        swathID 测线ID
	 * 返回:成功返回true，失败返回false
	 */
	bool makeFileTime(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
	{
		//===========下面为转化IDS的Index文件到Impluse的Time文件==========//
		//取得起始时间
		char fileSurvey[1024] = {}; sprintf(fileSurvey, "%s\\Survey", pathSrc);
		//定义并且打开IDS的Survey文件
		FILE *fpSurvey = fopen(fileSurvey, "r");
		if (!fpSurvey)
			return false;
		char szSurvey[256] = { 0 };
		fread(szSurvey, 1, 250, fpSurvey);
		fclose(fpSurvey);
		char *tmp = strstr(szSurvey, "Begin=\"");   //时间精确到秒
		tmp = tmp + 7;

		long long timeStart = 0;
		//将字符串转化为整数
		Utils::char2int(tmp, timeStart);

		//写入time文件
		char fileTimeIDS[1024]     = { 0 }; sprintf(fileTimeIDS    , "%s\\%s_Array01.index", pathSrc, swathName);
		char fileTimeImpluse[1024] = { 0 }; sprintf(fileTimeImpluse, "%s\\Swath%03d.time", pathDst, swathID);
		//定义并且打开IDS的index文件
		FILE *fpTimeIDS = fopen(fileTimeIDS, "rb");
		if (!fpTimeIDS)
			return false;
		//定义并且打开impluse的time文件
		FILE *fpTimeImpluse = fopen(fileTimeImpluse, "wb");
		if (!fpTimeImpluse)
		{
			fclose(fpTimeIDS);
			return false;
		}

		//用于存储每行时间的读取数据
		unsigned char buffTime[20] = { 0 };
		static long long offset = 0;
		//如果是第一个测线，则记录测线起始时间偏移
		if (1 == swathID)
		{
			//跳过IDS index的文件头(120字节，包含trace号为0的数据)
			fseek(fpTimeIDS, 100, SEEK_SET);
			fread(buffTime, 1, 20, fpTimeIDS);

			offset = buffTime[7] * 256;   //取得Trace时间（相对开始的时间--毫秒）
			offset = (offset + buffTime[6]) * 256;
			offset = (offset + buffTime[5]) * 256;
			offset = (offset + buffTime[4]);
		}
		else
            //跳过IDS index的文件头(120字节，包含trace号为0的数据)
            fseek(fpTimeIDS, 120, SEEK_SET);

		//下面循环处理后续trace,一直到文件结尾
		while (!feof(fpTimeIDS))
		{
			fread(buffTime, 1, 20, fpTimeIDS);
			int traceNum         = *((int *)buffTime);        //取得Trace号
			long long traceTimeMsecond = buffTime[7] * 256;   //取得Trace时间（相对开始的时间--毫秒）
                      traceTimeMsecond = (traceTimeMsecond + buffTime[6]) * 256;
                      traceTimeMsecond = (traceTimeMsecond + buffTime[5]) * 256;
                      traceTimeMsecond = (traceTimeMsecond + buffTime[4]) - offset;
			long long  traceTime = traceTimeMsecond/1000 + timeStart * 1000;    //Trace时间，绝对时间--毫秒
			//long long  traceTime = timeStart * 1000 + 900;    //Trace时间，绝对时间--毫秒
			char szTmp[128] = { 0 };
			char szLine[128] = { 0 };

			Utils::long2date(traceTime, szTmp);               //将时间转化为字符串格式
			sprintf(szLine, "%d	%s\r\n", traceNum, szTmp);
			fwrite(szLine, 1, strlen(szLine), fpTimeImpluse);
		}
		fclose(fpTimeIDS);
		fclose(fpTimeImpluse);

		return true;
	}

	/*
	 * Fun:存储iprh文件头信息
	 * Param:
	 * Return:成功返回true，失败返回false
	 */
	int saveHeader(const char * pathDst, const char *swathName, int channelNum)
	{
		//参数合法性校验
		if ((nullptr == pathDst) || (nullptr == swathName) )
			return -1;
		if ((0 >= strlen(pathDst)) || (0 >= strlen(swathName)))
			return -1;

		char pathFile[1024] = { 0 };
		sprintf(pathFile, "%s\\%s_A%02d.iprh", pathDst, swathName, channelNum);

		FILE * m_pFile = fopen(pathFile, "wt+");
		if (nullptr == m_pFile)
			return -2;

		//循环读取各个配置参数
		char s[300];

		sprintf(s, "HEADER VERSION: %d\n"    , headVersion     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DATA VERSION: %d\n"      , dataVersion     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DATE: %s\n"              , date            );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "START TIME: %s\n"        , startTime       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "STOP TIME: %s\n"         , stopTime        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA: %s\n"           , ANTENNA         );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA SEPARATION: %f\n", separation      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SAMPLES: %d\n"           , samples         );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SIGNAL POSITION: 12\n"                     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CLIPPED SAMPLES: 0\n"                      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "RUNS: %d\n"              , runs            );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "MAX STACKS: %d\n"        , maxStacks       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "AUTOSTACKS: 1\n"                           );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "FREQUENCY: %d\n"         , (int)frequency  );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TIMEWINDOW: %d\n"        , (int)timeWindow );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "LAST TRACE: %d\n"        , lastTrace       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TRIG SOURCE: %s\n"       , trigSource      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TIME INTERVAL: %f\n"     , intervalTime    );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DISTANCE INTERVAL: %f\n" , intervalDist    );  fwrite(s, 1, strlen(s), m_pFile);

		if (channelNum < 11)
		{
			sprintf(s, "USER DISTANCE INTERVAL: %f\n", 0.05);  fwrite(s, 1, strlen(s), m_pFile);
		}
		else
		{
			sprintf(s, "USER DISTANCE INTERVAL: %f\n", 0.1);  fwrite(s, 1, strlen(s), m_pFile);
		}

		sprintf(s, "STOP POSITION: %f\n"     , 1.0             );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "WHEEL NAME: WENDE\n"                       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "WHEEL CALIBRATION: 1.0\n"                  );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ZERO LEVEL: %d\n"        , zeroLevel       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SOIL VELOCITY: %f\n"     , soilVel         );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "PREPROCESSING: 0\n"                        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "OPERATOR COMMENT: _\n"                     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA F/W: 48001262\n"                   );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA H/W: 0\n"                          );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA FPGA: WENDE\n"                     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA SERIAL: 001\n"                     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SOFTWARE VERSION: 1.0.0\n"                 );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "POSITIONING: %d\n"       , positioning     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CHANNELS: %d\n"          , channels        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CHANNEL CONFIGURATION: T2 - R2\n"          );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CH_X_OFFSET: 0.000000\n"                   );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CH_Y_OFFSET: 0.000000\n"                   );  fwrite(s, 1, strlen(s), m_pFile);
		fclose(m_pFile);

		return 0;
	}


private:
	//下面一组提供通道属性定义
	int headVersion     = 0;        // header version: 10
	int dataVersion     = 0;        // data version: 16-16b; 32-32b 
	char date[16]       = { 0 };    //测线操作日期
	char startTime[9]   = { 0 };    //测线开始时间
	char stopTime[9]    = { 0 };    //测线结束时间
	char ANTENNA[10]    = { 0 };    //频率？
	double separation   = 0;
	int samples         = 0;        //一个trace中，采样数量
	int runs            = 0;
	int maxStacks       = 0;
	double frequency    = 0;        // Sampling Frequency (MHz)又是一个频率？
	double timeWindow   = 0;        //时间窗，波從發出返回的时间
	int lastTrace       = 0;        //一个测线中trace的数量
	char trigSource[16] = { 0 };
	double intervalTime = 0;        // time interval (seconds)
	double intervalDist = 0;        // real distance interval (meters)
	double intervalUser = 0;        // user distance interval (meters)
	int    zeroLevel    = 0;
	double soilVel      = 0;        // soil velocity (m/us)
	int    positioning  = 0;        // it is used for combined files only
	int    channels     = 0;
	char   chanName[16] = { 0 };    // channel name (like "T1-R1")

};

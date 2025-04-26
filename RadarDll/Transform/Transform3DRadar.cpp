/*
* Fun:将Segy数据转化为iprb、iprh格式
*/

//#include "pch.h"
//#include "framework.h"
#include <io.h>
#include <atlimage.h>

//#include "resource.h"		// 主符号

#include "..\\Utils\\PalSET.h"
#include "..\\Utils\\Utils.h"
#include "..\\Utils\\RadarConst.h"

#include "..\\Impluse\\ImpluseTrace16.h"
#include "..\\Impluse\\ImpluseTrace32.h"
#include "..\\Impluse\\ImpluseCor.h"
#include "..\\Impluse\\ImpluseTime.h"
#include "..\\Impluse\\ImpluseChannelHeader.h"
#include "..\\Impluse\\ImpluseChannelBlob.h"
#include "..\\Impluse\\ImpluseChannel.h"
#include "..\\Impluse\\ImpluseSwath.h"

//#include "FSize.h"

#include "..\\IDS\\IDSChannel.h"
#include "..\\IDS\\IDSChannelBlob.h"
#include "..\\IDS\\IDSChannelHeader.h"
#include "..\\IDS\\IDSSwath.h"
#include "..\\IDS\\IDSSwathFragment.h"
#include "..\\IDS\\IDSTrace16.h"
#include "..\\IDS\\IDSTrace32.h"

#include "..\\Mala\\MalaChannel.h"
#include "..\\Mala\\MalaChannelBlob.h"
#include "..\\Mala\\MalaChannelHeader.h"
#include "..\\Mala\\MalaSwath.h"
#include "..\\Mala\\MalaTime.h"
#include "..\\Mala\\MalaTrace16.h"
#include "..\\Mala\\MalaTrace32.h"

//#include "Project.h"
#include "TransformBase.h"
#include "Transform3DRadar.h"



/*
* Fun：将指定目录下的3DRadar数据转化为IPRH、IPRD，存储到指定目录下
* Param：
*      swathPath3DRadar    3DRadar数据目录(txt格式数据所在目录)
*      pathDst             IPRH、IPRD目标目录
* Return：成功返回TRUE，失败返回FALSE
*/
int Transform3DRadar::transform3DRadar(const char * swathPath3DRadar, const char * pathDst)
{
	//搜索雷达目录下，指定文件,Rd数据
	char szPathTxt[512] = { 0 };

	//首先寻找Mala16的标志文件
	sprintf(szPathTxt, "%s\\*.txt", swathPath3DRadar);
	intptr_t hFileTxt;
	struct _finddata_t oFileInfoTxt;
	if ((hFileTxt = (intptr_t)_findfirst(szPathTxt, &oFileInfoTxt)) == -1L)
	{
		_findclose(hFileTxt);
		printf("没有发现3DRadar的测线数据\n");
		return ERROR_NOFILE_3DRADAR;
	}

	int iResult = ERROR_CODE_SUCCESS;
	int swathIndex = 0;

	//1、循环加载3DRadar的所有测线
	while (true)
	{
		//获取测线名称
		char swathName[256] = { 0 };
		if (strlen(oFileInfoTxt.name) > 4)
			strncpy(swathName, oFileInfoTxt.name, strlen(oFileInfoTxt.name) - 4);

		//这里得到测线文件名，将该文件转化位Impluse
		//转化一个测线文件
		iResult = transform3DRadarSwath(swathPath3DRadar, pathDst, swathName);
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		swathIndex++;

		//搜索下一个，没有搜索到就返回
		if (_findnext(hFileTxt, &oFileInfoTxt) == -1L)
			break;
	}
	_findclose(hFileTxt);

	return iResult;
}

//将一个3Dradar文件转化为一个iprh\iprb测线
int Transform3DRadar::transform3DRadarSwath(const char * swathPath3DRadar, const char * pathDst, const char * swathName)
{
	char swathPathFile3DRadar[512] = { 0 };
	sprintf(swathPathFile3DRadar, "%s\\%s.txt", swathPath3DRadar, swathName);

	//将
	FILE * srcFile = fopen(swathPathFile3DRadar, "r");
	if (!srcFile)
	{
		//MessageBox("打开文件失败", "提示", MB_OK);
		return ERROR_NOFILE_3DRADAR;
	}

	//测线中Trace的数量
	int lastTrace = 0;
	//测线中通道的数量
	int inLines = 0;
	//一个Trace中采样的点数
	int samples = 0;
	//读取每个测线的Trace数量，通道数量，以及每个Trace的采样点数
	{
		//存放一行文本
		char buffLline[512] = { 0 };
		//读取的文件行数
		int countLine = 0;

		//循环读取
		while (!feof(srcFile))
		{
			//初始化缓存
			memset(buffLline, 0, 512);

			char * lineIndex = buffLline;
			//记录字符串的开始
			char * lineStart = buffLline;
			//记录读取的长度()
			int countChar = 0;
			//循环读取
			while (!feof(srcFile))
			{
				if (countChar >= 510)
					break;

				fread(lineIndex, 1, 1, srcFile);

				//如果得到完整行
				if ('\n' == lineIndex[0])
					break;

				lineIndex++;
				countChar++;
			}

			//得到一行数据，并且判断是否包含目标字符串
			if (strstr(lineStart, "#Volume: X-lines="))
			{
				//读取一个测线中的Trace数量
				lastTrace = getLastTraceByTxt(lineStart);
			}
			if (strstr(lineStart, "In-lines="))
			{
				//读取通道数量
				inLines = getInLinesByTxt(lineStart);
			}
			if (strstr(lineStart, "Samples="))
			{
				//读取每个Trace的采样点数
				samples = getSamplesByTxt(lineStart);
			}

			//文件头的最后一行
			if (strstr(lineStart, "\n") && strstr(lineStart, "#RegionGainData:"))
				break;
			//在文件开头已经读取10行，但没有发现文件头的最后一行
			if (countLine >= 10)
				break;

			countLine++;
		}
	}

	//Impluse cor文件
	char swathPathFileImpluseCor[512] = { 0 };
	sprintf(swathPathFileImpluseCor, "%s\\%s.cor", pathDst, swathName);
	//Impluse time文件
	char swathPathFileImpluseTime[512] = { 0 };
	sprintf(swathPathFileImpluseTime, "%s\\%s.time", pathDst, swathName);

	FILE * dstFileCor = fopen(swathPathFileImpluseCor, "wb+");
	FILE * dstFileTime = fopen(swathPathFileImpluseTime, "wb+");
	fclose(dstFileCor);
	fclose(dstFileTime);

	//循环每个通道，读取数据，写入目标
	for (int i = 0; i < inLines; i++)
	{
		char swathPathFileImpluseIprb[512] = { 0 };
		char swathPathFileImpluseIprh[512] = { 0 };
		sprintf(swathPathFileImpluseIprb, "%s\\%s_A%02d.iprb", pathDst, swathName, i + 1);
		sprintf(swathPathFileImpluseIprh, "%s\\%s_A%02d.iprh", pathDst, swathName, i + 1);   //like "C:\\3D_TestData\\RadarData\\20231011-TEST_001_A01.iprb";

		FILE * dstFileIprb = fopen(swathPathFileImpluseIprb, "wb+");
		FILE * dstFileIprh = fopen(swathPathFileImpluseIprh, "wb+");

		//读取一个通道中所有Trace，写入iprb文件
		for (int j = 0; j < lastTrace; j++)
		{
			char szLatitude[64] = { 0 };
			char szLongitude[64] = { 0 };
			fscanf(srcFile, "%s	%s", szLatitude, szLongitude);

			//读取一个Trace的所有sample数据
			unsigned char szBuffDst[1324] = { 0 };
			for (int k = 0; k < samples; k++)
			{
				unsigned int iTmp = 0;
				fscanf(srcFile, "%d", &iTmp);
				//iTmp = htonl(iTmp);

				unsigned char *pTmp = szBuffDst + 2 * k;
				memcpy(pTmp, &iTmp, 2);
			}
			//将一个Trace中的所有sample写入目标
			int lenDst = (int)fwrite(szBuffDst, 1, samples * 2, dstFileIprb);
			if (samples * 2 != lenDst)
			{
				printf("Error");
			}
		}
		fclose(dstFileIprb);

		//这里要构建iprh文件
		{
			//=======写文件========//
			char szBuff[64] = { 0 };
			sprintf(szBuff, "HEADER VERSION: 10\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "DATA VERSION: 16\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "DATE: 2022-11-10\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "START TIME: 12:08:46\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "STOP TIME: 12:09:48\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA: %d MHz\n", 500);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA SEPARATION: %f\n", 1.1);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "SAMPLES: %d\n", samples);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "SIGNAL POSITION: 12\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "CLIPPED SAMPLES: 0\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "RUNS: 32\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "MAX STACKS: 512\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "AUTOSTACKS: 1\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "FREQUENCY: %d\n", 500);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "TIMEWINDOW: %d\n", 80);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "LAST TRACE: %d\n", lastTrace);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "TRIG SOURCE: wheel\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "TIME INTERVAL: 0.050000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "DISTANCE INTERVAL: %f\n", 1.1);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "USER DISTANCE INTERVAL: 0.050000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "STOP POSITION: 368.635412\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "WHEEL NAME: New_Wheel123\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "WHEEL CALIBRATION:477.8000000000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ZERO LEVEL: 40\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "SOIL VELOCITY: 100.000000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "PREPROCESSING: 0\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "OPERATOR COMMENT: _\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA F/W: 48001262\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA H/W: 0\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA FPGA: DA74\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA SERIAL: 1579\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "SOFTWARE VERSION: T 1.2.20\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "POSITIONING: 2\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);


			sprintf(szBuff, "CHANNELS: 18\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "CHANNEL CONFIGURATION: T2 - R2\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);


			sprintf(szBuff, "CH_X_OFFSET: 0.000000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "CH_Y_OFFSET: 0.000000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			fclose(dstFileIprh);
		}
	}

	return ERROR_CODE_SUCCESS;
}

int Transform3DRadar::getLastTraceByTxt(char * lineStart)
{
	char * tmp = strstr(lineStart, "X-lines=");
	if (NULL == tmp)
		return 0;
	else
	{
		tmp = tmp + 8;
		char * tmp1 = strstr(tmp, ",");

		char strNum[20] = { 0 };
		strncpy(strNum, tmp, tmp1 - tmp);

		return atoi(strNum);
	}
}
int Transform3DRadar::getInLinesByTxt(char * lineStart)
{
	char * tmp = strstr(lineStart, "In-lines=");
	if (NULL == tmp)
		return 0;
	else
	{
		tmp = tmp + 9;
		char * tmp1 = strstr(tmp, ",");

		char strNum[20] = { 0 };
		strncpy(strNum, tmp, tmp1 - tmp);

		return atoi(strNum);
	}
}
int Transform3DRadar::getSamplesByTxt(char * lineStart)
{
	char * tmp = strstr(lineStart, "Samples=");
	if (NULL == tmp)
		return 0;
	else
	{
		tmp = tmp + 8;
		char * tmp1 = strstr(tmp, "\n");

		char strNum[20] = { 0 };
		strncpy(strNum, tmp, tmp1 - tmp);

		return atoi(strNum);
	}
}

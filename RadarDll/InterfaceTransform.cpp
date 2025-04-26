/*
 * Fun:将各种不同的数据格式转化为iprb、iprh
 */

#include "framework.h"
#include <io.h>
//#include <atlimage.h>

/*
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <winnls.h>
*/

#include "Utils\\PalSET.h"
#include "Utils\\Utils.h"
#include "Utils\\RadarConst.h"

#include "Impluse\\ImpluseTrace16.h"
#include "Impluse\\ImpluseTrace32.h"
#include "Impluse\\ImpluseCor.h"
#include "Impluse\\ImpluseTime.h"
#include "Impluse\\ImpluseChannelHeader.h"
#include "Impluse\\ImpluseChannelBlob.h"
#include "Impluse\\ImpluseChannel.h"
#include "Impluse\\ImpluseSwath.h"

//#include "FSize.h"

#include "IDS\\IDSChannel.h"
#include "IDS\\IDSChannelBlob.h"
#include "IDS\\IDSChannelHeader.h"
#include "IDS\\IDSSwath.h"
#include "IDS\\IDSSwathFragment.h"
#include "IDS\\IDSTrace16.h"
#include "IDS\\IDSTrace32.h"

#include "Mala\\MalaChannel.h"
#include "Mala\\MalaChannelBlob.h"
#include "Mala\\MalaChannelHeader.h"
#include "Mala\\MalaSwath.h"
#include "Mala\\MalaTime.h"
#include "Mala\\MalaTrace16.h"
#include "Mala\\MalaTrace32.h"

#include "Transform\\TransformSegy.h"
#include "Transform\\Transform3DRadar.h"
#include "Transform\\TransformMala.h"
#include "Transform\\TransformIDS.h"
#include "Transform\\TransformDT.h"

#include "Transform2Segy\\Transform2SegyBase.h"

/*
 * Fun:将一个DT文件转化为IPRH和IPRB文件-------这种转化后续作废，这种转化可以被下一个函数替代
 *     该方法支持的文件为LA010001.DT
 */
extern "C" __declspec(dllexport) int tranfersDT(const char * swathPath, int freq, float separation, const char * swathPathDst)
{
	int length = (int)wcslen((wchar_t*)swathPath);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDT[256] = { 0 };
	memset(swathPathDT, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathPath, length, swathPathDT, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//配置文件的路径
	char configFile[256] = { 0 };
	memset(configFile, 0, 256);

	//char configFile[256] = { 0 };
	sprintf(configFile, "%s\\Stream X1.xml", swathPathDT);
	//AfxMessageBox(configFile);
	//读取last trace和sample
	FILE * config = fopen(configFile, "r");
	if (!config)
		return ERROR_DT_XML;

	//目标存儲文件的路径
	char pathDst[256] = { 0 };
	memset(pathDst, 0, 256);

	length = (int)wcslen((wchar_t*)swathPathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathPathDst, length, pathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//存放配置文件信息
	char * configContent = (char *)malloc(1024 * 8);

	//读取全部配置文件
	char * tmp = configContent;
	size = 1024 * 8;
	while (true)
	{
		//读取部分数据
		size = size - (int)fread(configContent, 1, size, config);
		if (size < 1024)
			break;
		if (feof(config))
			break;
	}
	//已經读取完畢，可以關閉
	fclose(config);

	//計算最終的XStep
	float distance = 0.0;
	//读取下step
	{
		char *tmp = strstr(configContent, "<EncoderStack");
		if (!tmp)
		{
			free(configContent);
			return ERROR_DT_NOCHANNEL;
		}
		char *xstep = strstr(tmp, "\"");
		if (!xstep)
		{
			free(configContent);
			return ERROR_DT_ERRCHANNEL;
		}

		int istep = 0;
		xstep[0] = ' ';
		sscanf(xstep, "%d", &istep);

		tmp = strstr(configContent, "<XStep value");
		if (!tmp)
		{
			tmp = "";
			//free(configContent);
			//return false;
		}
		xstep = strstr(tmp, "\"");
		if (!xstep)
		{
			xstep = "000";
			//free(configContent);
			//return false;
		}

		float fstep = (float)0.002;
		if (0 != strcmp(xstep, "000"))
		{
			xstep[0] = ' ';
			sscanf(xstep, "%f", &fstep);
		}

		//計算最終的XStep
		distance = fstep * istep;
	}

	//循環处理每个测线
	int iSwathNum = 1;
	while (true)
	{
		//查找目标测线
		char szDTSwathFile[512] = { 0 };
		sprintf(szDTSwathFile, "%s\\RAW_%06d.scan", swathPathDT, iSwathNum);

		//如果不能找到目标测线，则跳出
		FILE *fpTmp = fopen(szDTSwathFile, "r");
		if (!fpTmp)
			break;
		fclose(fpTmp);

		//测线名稱
		char szDTSwathName[64] = { 0 };
		sprintf(szDTSwathName, "%04d", iSwathNum);
		iSwathNum++;

		//循環读取每个通道
		tmp = strstr(configContent, "<Channels Count=");
		if (!tmp)
			break;

		//如果找到目标测线，则处理测线信息
		while (true)
		{
			//读取每行通道
			tmp = strstr(tmp, "<Channel Id");
			if (!tmp)
				break;
			tmp++;

			char *channelID = strstr(tmp, "Id=\""); channelID = strstr(channelID, "\""); channelID++;
			char *channelNs = strstr(tmp, "Range_ns=\""); channelNs = strstr(channelNs, "\""); channelNs++;
			char *channelSamples = strstr(tmp, "Samples=\""); channelSamples = strstr(channelSamples, "\""); channelSamples++;

			int samples = 0;
			int iChannelID = 0;
			int ns = 0;
			sscanf(channelNs, "%d", &ns);
			sscanf(channelID, "%d", &iChannelID);
			sscanf(channelSamples, "%d", &samples);

			char szDTH[512] = { 0 };
			sprintf(szDTH, "%s\\GRED_HD\\LA%02d%s.HDR_00", swathPathDT, iChannelID, szDTSwathName);
			char szDTD[512] = { 0 };
			sprintf(szDTD, "%s\\GRED_HD\\LA%02d%s.DT", swathPathDT, iChannelID, szDTSwathName);
			char szIPRH[512] = { 0 };
			sprintf(szIPRH, "%s\\%s_A%02d.iprh", pathDst, szDTSwathName, iChannelID);
			char szIPRB[512] = { 0 };
			sprintf(szIPRB, "%s\\%s_A%02d.iprb", pathDst, szDTSwathName, iChannelID);

			int tracecount = 0;

			if (9 <= iChannelID)
				freq = 200;
			else
				freq = 600;

			TransformDT oTransformDT;
			int iResult = oTransformDT.dt2iprh(szDTH, freq, ns, distance, separation, szIPRH, tracecount);
			iResult = oTransformDT.dt2iprb(szDTD, szIPRB, samples, tracecount);
		}

		//创建cor文件
		char szCor[512] = { 0 };
		sprintf(szCor, "%s\\%s.cor", pathDst, szDTSwathName);
		FILE * pFileCor = fopen(szCor, "w+");
		fclose(pFileCor);

		//创建time文件
		char szTime[512] = { 0 };
		sprintf(szTime, "%s\\%s.time", pathDst, szDTSwathName);
		FILE * pFileTime = fopen(szTime, "w+");
		fclose(pFileTime);
	}

	//釋放空间
	free(configContent);

	return ERROR_CODE_SUCCESS;
}

/*
 * Fun:将IDS采集数据(整个工程)转化为IPRH/IPRB
 *     该方法支持RAW_000001.scan/Stream X.xml文件格式.
 */
extern "C" __declspec(dllexport) int tranfersIDS(const char * pathIDS, const char * pathDst, int freq, float separation)
{
	//1、取得IDS数据的目录
	int length = (int)wcslen((wchar_t*)pathIDS);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathIDS[256] = { 0 };
	memset(swathPathIDS, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathIDS, length, swathPathIDS, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;


	//2、取得存储目标的目录
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3、将ids转化为iprh
	TransformIDS oTransformIDS;
	return oTransformIDS.transformIDS(swathPathIDS, swathPathDst, separation);
}

/*
 * Fun:将IDS（16位）采集数据(整个工程)转化为IPRH/IPRB
 *     该方法支持将IDS的Swath001_Array01.data/Swath001_Array01.index/Swath001.swath/Survey文件转为iprb
 */
extern "C" __declspec(dllexport) int tranfersIDSData16(const char * pathIDS, const char * pathDst)
{
	//1、取得IDS数据的目录
	int length = (int)wcslen((wchar_t*)pathIDS);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathIDS[256] = { 0 };
	memset(swathPathIDS, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathIDS, length, swathPathIDS, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2、取得存储目标的目录
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3、将ids data格式的文件转化为iprh、iprb
	TransformIDS oTransformIDS;
	return oTransformIDS.transformIDS16(swathPathIDS, swathPathDst);
}

/*
 * Fun:将IDS（8位）采集数据(整个工程)转化为IPRH/IPRB ---------后续备用，用于Stream_DP雷达
 *     该方法支持将IDS的Swath001_Array01.data文件转为iprb
 */
extern "C" __declspec(dllexport) int tranfersIDSData08(const char * pathIDS, const char * pathDst, int iSample)
{
	//1、取得IDS数据的目录
	int length = (int)wcslen((wchar_t*)pathIDS);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathIDS[256] = { 0 };
	memset(swathPathIDS, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathIDS, length, swathPathIDS, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2、取得存储目标的目录
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3、将ids data格式的文件转化为iprh、iprb
	TransformIDS oTransformIDS;
	oTransformIDS.setSample(iSample);
	return oTransformIDS.transformIDS08(swathPathIDS, swathPathDst);
}

/*
 * Fun:将Mala数据转化为IPRH、IPRB格式
 * Param:  pathMala  Mala数据存放目录
 *         pathDst   目标数据存放目录
 */
extern "C" __declspec(dllexport) int tranfersMala(const char * pathMala, const char * pathDst)
{
	//1、取得Mala数据的目录
	int length = (int)wcslen((wchar_t*)pathMala);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathMala[256] = { 0 };
	memset(swathPathMala, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathMala, length, swathPathMala, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2、取得存储目标的目录
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//Mala数据转化对象
	TransformMala oTransformMala;

	//3、寻找Mala32
	char szPathRd7[512] = { 0 };
	//首先寻找Mala32的标志文件
	sprintf(szPathRd7, "%s\\*.rd7", swathPathMala);
	intptr_t hFileRd7;
	struct _finddata_t oFileInfoRd7;
	if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) == -1L)
	{
		printf("没有发现Mala32的测线数据\n");
		_findclose(hFileRd7);
	}
	else
	{
		_findclose(hFileRd7);

		//4、将Mala转化为iprh/iprb
		return oTransformMala.transformMala32(swathPathMala, swathPathDst);
	}

	//===================下面是首先监测16位雷达是三维或二维，然后根据实际情况进行转化=====================//
	char szPathRad[512] = { 0 };
	//寻找RAD文件，并且读取RAD文件中的Channels数据，通过RAD文件判断是3维数据还是2维数据
	sprintf(szPathRad, "%s\\*.rad", swathPathMala);
	intptr_t hFileRad;
	struct _finddata_t oFileInfoRad;
	if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
	{
		_findclose(hFileRad);
		printf("没有发现Mala16的测线数据\n");
		return 9;
	}
	//关闭原有的搜索句柄
	_findclose(hFileRad);

	//下面是16位Mala数据的处理，读取Rad文件中的Channels数据
	//设置后续雷达数据是3维或2维处理
	memset(szPathRad, 0, 512);
	sprintf(szPathRad, "%s\\%s", swathPathMala, oFileInfoRad.name);

	//读取Rad文件的所有内容
	FILE *fpRad = fopen(szPathRad, "r");
	char *szBuff = (char *)malloc(2048);
	int iIndex = 0;
	int iLength = 0;
	while (true)
	{
		iLength = (int)fread(szBuff + iIndex, 1, 2048 - iIndex, fpRad);
		iIndex = iIndex + iLength;
		if (!iLength)
			break;
	}
	fclose(fpRad);

	//搜索Channels
	char *szChannel = strstr(szBuff, "CHANNELS:");
	if (szChannel)
		//6、将Mala16三维数据转化为iprh
		return oTransformMala.transformMala16(swathPathMala, swathPathDst);
	else
		//8、将Mala16二维数据转化为iprh
		return oTransformMala.transformMala16Ex(swathPathMala, swathPathDst);
}

/*
 * Fun:将Mala数据转化为IPRH、IPRB格式
 * Param:  pathMala  一种特殊的32位Mala数据存放目录-----这个好像是Mala32的二维雷达
 *         pathDst   目标数据存放目录
 */
extern "C" __declspec(dllexport) int tranfersMala32Ex(const char * pathMala, const char * pathDst)
{
	//1、取得Mala数据的目录
	int length = (int)wcslen((wchar_t*)pathMala);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathMala[256] = { 0 };
	memset(swathPathMala, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathMala, length, swathPathMala, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2、取得存储目标的目录
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3、寻找Mala32
	char szPathMcor[512] = { 0 };
	//首先寻找Mala32的标志文件
	sprintf(szPathMcor, "%s\\*.cor", swathPathMala);
	intptr_t hFileMcor;
	struct _finddata_t oFileInfoMcor;
	if ((hFileMcor = (intptr_t)_findfirst(szPathMcor, &oFileInfoMcor)) == -1L)
	{
		printf("没有发现Mala32的测线数据\n");
		_findclose(hFileMcor);
	}
	else
	{
		_findclose(hFileMcor);

		//4、将Mala转化为iprh
		TransformMala oTransformMala;
		return oTransformMala.transformMala32Ex(swathPathMala, swathPathDst);
	}

	return ERROR_NOFILE_MALA;
}

/*
 * Fun:将3DRadar数据转化为IPRH、IPRB格式(欧美大地、康图)
 * Param:  path3DRadar  3DRadar数据存放目录
 *         pathDst   目标数据存放目录
 */
extern "C" __declspec(dllexport) int tranfers3DRadar(const char * path3DRadar, const char * pathDst)
{
	//1、取得3DRadar数据的目录
	int length = (int)wcslen((wchar_t*)path3DRadar);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPath3DRadar[256] = { 0 };
	memset(swathPath3DRadar, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)path3DRadar, length, swathPath3DRadar, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2、取得存储目标的目录
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3、寻找3DRadar
	char szPathTxt[512] = { 0 };
	//首先寻找3DRadar 32的标志文件
	sprintf(szPathTxt, "%s\\*.txt", swathPath3DRadar);
	intptr_t hFileTxt;
	struct _finddata_t oFileInfoMcor;
	if ((hFileTxt = (intptr_t)_findfirst(szPathTxt, &oFileInfoMcor)) == -1L)
	{
		printf("没有发现3DRadar的测线数据\n");
		_findclose(hFileTxt);
	}
	else
	{
		_findclose(hFileTxt);

		//4、将3DRadar转化为iprh
		Transform3DRadar oTransform3DRadar;
		return oTransform3DRadar.transform3DRadar(swathPath3DRadar, swathPathDst);
	}

	return ERROR_NOFILE_3DRADAR;
}

/*
 * Fun:将Segy数据转化为IPRH、IPRB格式(欧美大地、康图) --- 支持32位和16位
 * Param:  path      Segy数据存放目录
 *         pathDst   目标数据存放目录
 */
extern "C" __declspec(dllexport) int tranfersSegy(const char * path, const char * pathDst)
{
	//1、取得Segy数据的目录
	int length = (int)wcslen((wchar_t*)path);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathSegy[256] = { 0 };
	memset(swathPathSegy, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)path, length, swathPathSegy, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2、取得存储目标的目录
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3、寻找Segy
	char szPathTmp[512] = { 0 };
	//首先寻找Segy的标志文件
	sprintf(szPathTmp, "%s\\*.sgy", swathPathSegy);
	intptr_t hFileFd;
	struct _finddata_t oFileInfo;
	if ((hFileFd = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
	{
		printf("没有发现sgy的测线数据\n");
		_findclose(hFileFd);
	}
	else
	{
		_findclose(hFileFd);

		TransformSegy theAppTransformSegy;
		//4、将Segy转化为iprh
		return theAppTransformSegy.transformSegy(swathPathSegy, swathPathDst);
	}

	return ERROR_NOFILE_SEGY;
}

/*
 * Fun:将GSSI数据转化为IPRH、IPRB格式(欧美大地、康图)
 * Param:  pathGSSI  GSSI数据存放目录
 *         pathDst   目标数据存放目录
 */
extern "C" __declspec(dllexport) int tranfersGSSI(const char * pathGSSI, const char * pathDst)
{
	return -1;
}

/*
 * Fun:将一个多通道的Segy文件分解为，多个单通道的Segy文件
 * Param:  pathFile  多通道的Segy文件(包含路径)
 *         pathDst   目标目录，存放单通道的Segy文件
 */
extern "C" __declspec(dllexport) int tranfersSegySplit(const char * pathFile, const char * pathDst)
{
	//1、取得Segy数据的目录
	int length = (int)wcslen((wchar_t*)pathFile);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径--输入文件
	char swathPathSegySrc[256] = { 0 };
	memset(swathPathSegySrc, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathFile, length, swathPathSegySrc, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2、取得存储目标的目录
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//原始路径
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	Transform2SegyBase *pTransform2SegyBase = new Transform2SegyBase();

	int iResult = pTransform2SegyBase->SplitChannel(swathPathDst, swathPathSegySrc);

	delete pTransform2SegyBase;

	return iResult;
}

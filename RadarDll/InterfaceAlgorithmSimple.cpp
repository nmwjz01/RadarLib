/*
 * Fun:采用自我的算法库对图像数据进行处理
 */

#include "framework.h"
#include <io.h>
//#include <atlimage.h>

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

#include "Project.h"
#include "RadarDll.h"


//全局对象
extern CRadarDllApp theApp;

char szProjectPathC[PATH_MAX_LENGTH] = { 0 };    //工程目录
Project gProjectC;      //全局的工程对象

extern "C" __declspec(dllexport) int getVersion(const char *strPwd, char *strVersion)
{
	//参数合法性校验
	if (!strPwd)
		return ERROR_PARAM;
	if (!strVersion)
		return ERROR_PARAM;
	//输入参数长度
	int length = (int)wcslen((wchar_t*)strPwd);
	if (length <= 0)
		return ERROR_PARAM;
	if (length > 64)
		length = 64;

	char szPwd[65] = { 0 };
	char szVersion[65] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)strPwd, length, szPwd, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	if (strcmp(szPwd, "My name is alpha") == 0)
		strcpy(szVersion, "Make by:Wende on 2024-04-20");
	else
		strcpy(szVersion, "Make by:zgz&wjz&cf");

	wchar_t* buffer = (wchar_t*)malloc(128);
	memset((char *)buffer, 0, 128);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVersion, (int)strlen(szVersion) * 2 + 2, buffer, (int)strlen(szVersion) * 2 + 2);
	wcscpy((wchar_t *)strVersion, buffer);
	free(buffer);

	return ERROR_CODE_SUCCESS;
}

//设置雷达数据的存储目录，同时初始化雷达工程，后续对数据的操作都属于该工程
extern "C" __declspec(dllexport) int setProjectPath(const char *strpath)
{
	int length = (int)wcslen((wchar_t*)strpath);
	if (length <= 0)
		return ERROR_PARAM;

	memset(szProjectPathC, 0, PATH_MAX_LENGTH);

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)strpath, length, szProjectPathC, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	return ERROR_CODE_SUCCESS;
}

//初始化工程
extern "C" __declspec(dllexport) int initProject()
{
	if (0 >= strlen(szProjectPathC))
		return ERROR_CODE_PATH;

	//初始化工程（包含cor文件和time文件的数据集）
	int result = gProjectC.init(szProjectPathC);
	//如果第一种方式初始化失败，则采用第二种方式初始化
	if (result)
	{
		//（包含mrk文件和ord文件的数据集）
		result = gProjectC.initEx(szProjectPathC);
	}

	return result;
}

//逆初始化工程
extern "C" __declspec(dllexport) int unInitProject()
{
	gProjectC.unInit();
	return ERROR_CODE_SUCCESS;
}

//通过时间获取对应的Trace号
extern "C" __declspec(dllexport) int getTraceNumByTime(const char *  swathName, char *traceTime)
{
	int length = (int)wcslen((wchar_t*)traceTime);
	if (length <= 0)
		return ERROR_CODE_OTHER;

	//TraceTime
	char szTraceTime[64] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)traceTime, length, szTraceTime, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_CODE_OTHER;

	if (strlen(szTraceTime) <= 0)
		return ERROR_CODE_OTHER;

	length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_CODE_OTHER;
	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_CODE_OTHER;

	//获取目标测线
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_OTHER;

	SwathTime * swathTime = swath->getSwathTime();
	return swathTime->getTraceNumByTime(szTraceTime);
}

//通过Trace号获取对应的时间
extern "C" __declspec(dllexport) int getTraceTimeByNum(const char *  swathName, int traceNum, char *traceTime)
{
	if (!traceTime)
		return ERROR_PARAM;

	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_CODE_OTHER;
	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_CODE_OTHER;

	//获取目标测线
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_OTHER;

	char szDateTime[64] = { 0 };
	SwathTime * swathTime = swath->getSwathTime();
	swathTime->getTraceTimeByNum(traceNum, szDateTime);

	wchar_t* buffer = (wchar_t*)malloc(128);
	memset((char *)buffer, 0, 128);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szDateTime, (int)strlen(szDateTime) * 2 + 2, buffer, (int)strlen(szDateTime) * 2 + 2);
	wcscpy((wchar_t *)traceTime, buffer);
	free(buffer);

	return 0;
}


//设置计算雷达数据的参数--对比度，后续图像处理前需要根据需要设定该参数，否则使用默认数据或已经设定的参数
extern "C" __declspec(dllexport) int setParamContrast(BOOL autoEnable, int contrast)
{
	if (!autoEnable)
		//手动设置一个对比度
		theApp.SetContrast(contrast);
	else
		theApp.SetContrast(20);
	return ERROR_CODE_SUCCESS;
}

//设置计算雷达数据的参数--增益，后续图像处理前需要根据需要设定该参数，否则使用默认数据或已经设定的参数
extern "C" __declspec(dllexport) int setParamGain(BOOL autoEnable, int gain)
{
	if (!autoEnable)
		//手动设置增益
		theApp.SetGain(gain);
	else
		theApp.SetGain(10);

	return ERROR_CODE_SUCCESS;
}

//设置图像输出颜色
extern "C" __declspec(dllexport) int setColor(int iColor, int iMask)
{
	theApp.SetColor(iColor, iMask);
	return ERROR_CODE_SUCCESS;
}

//设置计算雷达数据的参数--深度，后续图像处理前需要根据需要设定该参数，否则使用默认数据或已经设定的参数
extern "C" __declspec(dllexport) int setParamDeep(BOOL autoEnable, int deep[])
{
	if (!autoEnable)
	{
		int iDeep[32] = { 10 };   //增益
		//手动设置z坐标找平参数
		for (int i = 0; i < 32; ++i)
		{
			iDeep[i] = deep[i];
		}

		theApp.SetDeep(iDeep);
	}
	else
	{
		int iDeep[32] = { 10 };   //增益
		theApp.SetDeep(iDeep);
	}
	return ERROR_CODE_SUCCESS;
}

//获取测线数量
extern "C" __declspec(dllexport) int getSwathCount()
{
	std::vector<char*> lstSwath;
	gProjectC.getAllSwathName(lstSwath);

	//取得测线数量
	int iSize = (int)lstSwath.size();

	return iSize;
}

//通过测线的索引，获取测线名称。如果总测线数量为3，则测线索引为0、1、2，以此类推
extern "C" __declspec(dllexport) int getSwathName(int index, char *swathName)
{
	std::vector<char*> lstSwath;
	gProjectC.getAllSwathName(lstSwath);

	//取得测线数量
	int iSize = (int)lstSwath.size();
	if (0 >= iSize)
		return ERROR_CODE_NOSWATH;

	//遍历，得到目标测线的名称
	int i = 0;
	char* pSwath = NULL;
	for (std::vector<char*>::iterator it_Swath = lstSwath.begin(); it_Swath != lstSwath.end(); it_Swath++)
	{
		if (NULL == *it_Swath)
			continue;

		//找到对应的测线名
		if (index == i)
		{
			pSwath = *it_Swath;
			//strcpy(swathName, *it_Swath);
			break;
		}

		i++;
	}
	if (!pSwath)
		return ERROR_CODE_SUCCESS;
	int slen = (int)strlen(pSwath);
	if (slen == 0)
		return ERROR_CODE_SUCCESS;

	int length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pSwath, slen, NULL, 0);
	wchar_t* buffer = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)buffer, 0, length * 2 + 2);

	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pSwath, slen, buffer, length);

	wcscpy((wchar_t *)swathName, buffer);
	free(buffer);
	return ERROR_CODE_SUCCESS;
}

//通过指定的测线名称，获取指定测线中的通道数量
extern "C" __declspec(dllexport) int getChannelCount(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//获取目标测线
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return 0;

	//从测线中获取通道数量
	int iCount = 0;
	swath->getChannelCount(iCount);

	return iCount;
}

//通过指定测线名称和通道索引，获取指定通道中的Trace数量。通道ID：为1、2、3 ... ...，这个是和目录下通道文件对应的。
extern "C" __declspec(dllexport) int getTraceCount(const char *  swathName, int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//获取目标测线
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return 0;

	//获取对应的通道对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return 0;

	//取得通道对象的头信息
	SwathChannelHeader * channelHeader = channel->getChannelHeader();
	if (!channelHeader)
		return 0;

	//直接返回Trace数量
	return channelHeader->getTraceCount();
}

//获取一个测线中，Trace之间的间隔
extern "C" __declspec(dllexport) double getDistanceInterval(const char *  swathName, int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return 0;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return 0;

	//获取目标测线
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return 0;

	//获取对应的通道对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return 0;

	//取得通道对象的头信息
	int iCount = 0;
	SwathChannelHeader * channelHeader = channel->getChannelHeader();
	if (!channelHeader)
		return 0;

	//得到Trace间隔
	double dist = channelHeader->getIntervalDist();
	return dist;
}

/*
Fun:根据测线名，通道号，Trace开始，Trace结束，导出垂直扫描图
PAram:
	picFile    目标图像路径(路径和文件名，如：C:\MyPath\深南大道2022-10-30\Pic\123.jpg)
	swathName  测线名
	withLine   是否带有标线
	channel    通道号
	traceBegin 开始Trace
	traceEnd   结束Trace
	deep       需要製圖的深度範圍
Return: 成功返回0，失败对应的错误码
*/
extern "C" __declspec(dllexport) int exportRadarPicV(const char * picFile, const char * swathName, BOOL withLine, int channelID, int traceBegin, int traceCount, int deep)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	length = (int)wcslen((wchar_t*)picFile);
	if (length <= 0)
		return ERROR_PARAM;
	char szPicFile[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)picFile, length, szPicFile, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//4、对存储路径和文件名做简单的校验
	if (strlen(szPicFile) <= 4)
		return ERROR_PARAM;

	//这个是总深度（cm）
	//int iAllDeep = ( int )( channel->getChannelHeader()->getTimeWindow() * channel->getChannelHeader()->getSoilVel() / 20 );
	//這个是垂直縂樣本数
	//int sampleCount = channel->getChannelHeader()->getSample();
	//通過深度範圍計算垂直樣本数
	//int height = sampleCount * deep / iAllDeep;

	//5、将测线数据存储为图片文件
	//int iResult = theApp.SaveAsPicV(szPicFile, channel, traceBegin, traceCount, height);
	int iResult = theApp.SaveAsPicV(szPicFile, channel, traceBegin, traceCount, deep);
	return iResult;
	if (0 != iResult)
		return ERROR_CODE_SYS;
	else
		return ERROR_CODE_SUCCESS;
}

/*
Fun:根据测线名，深度，Trace开始，Trace结束，导出水平扫描图
PAram:
	picFile    目标图像路径
	swathName  测线名
	withLine   是否带有标线
	deep       深度
	traceBegin 开始Trace
	traceEnd   结束Trace
Return: 成功返回0，失败对应的错误码
*/
extern "C" __declspec(dllexport) int exportRadarPicH(const char * picFile, const char * swathName, BOOL withLine, int deep, int traceBegin, int traceCount)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	length = (int)wcslen((wchar_t*)picFile);
	if (length <= 0)
		return ERROR_PARAM;
	char szPicFile[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)picFile, length, szPicFile, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3、对存储路径和文件名做简单的校验
	if (strlen(szPicFile) <= 4)
		return ERROR_PARAM;

	//4、将测线数据存储为图片文件
	int iResult = theApp.SaveAsPicH(szPicFile, swath, deep, traceBegin, traceCount);
	return iResult;
	if (0 != iResult)
		return ERROR_CODE_SYS;
	else
		return ERROR_CODE_SUCCESS;

	return 0;
}

/*
Fun:根据测线名，Trace号，获取当前的抓拍图像名
PAram:
	picFileFront    目标图像路径--前置摄像头
	picFileBack     目标图像路径--后置摄像头
	picFileLeft     目标图像路径--左置摄像头
	picFileRight    目标图像路径--右置摄像头
	swathName  测线名
	traceNum   Trace号
Return: 成功返回0，失败对应的错误码
*/
extern "C" __declspec(dllexport) int takePhotoPic(char * picFileFront, char * picFileBack, char * picFileLeft, char * picFileRight, const char * swathName, int traceNum, int iOffsetTime)
{
	//参数合法性判断
	if ((!picFileFront) || (!picFileBack) || (!picFileLeft) || (!picFileRight))
		return ERROR_PARAM;

	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	char szPicFileFront[512] = { 0 };
	char szPicFileBack[512] = { 0 };
	char szPicFileLeft[512] = { 0 };
	char szPicFileRight[512] = { 0 };
	//4、将测线数据存储为图片文件
	int iResult = theApp.TakePhotoPic(&gProjectC, szPicFileFront, szPicFileBack, szPicFileLeft, szPicFileRight, swath, traceNum, iOffsetTime);
	if (0 != iResult)
		return ERROR_CODE_SYS;

	//因爲.Net接受参数，长度大于25就会出錯，所以去掉擴展名
	szPicFileFront[21] = 0;
	szPicFileBack[21] = 0;
	szPicFileLeft[21] = 0;
	szPicFileRight[21] = 0;

	int slen = (int)strlen(szPicFileFront);
	if (0 != slen)
	{
		length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileFront, slen, NULL, 0);
		wchar_t* bufferFront = (wchar_t*)malloc(slen * 2 + 2);
		memset((char *)bufferFront, 0, slen * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileFront, slen, bufferFront, length);
		wcscpy((wchar_t *)picFileFront, bufferFront);
		free(bufferFront);
	}

	slen = (int)strlen(szPicFileBack);
	if (0 != slen)
	{
		length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileBack, slen, NULL, 0);
		wchar_t* bufferBack = (wchar_t*)malloc(slen * 2 + 2);
		memset((char *)bufferBack, 0, slen * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileBack, slen, bufferBack, length);
		wcscpy((wchar_t *)picFileBack, bufferBack);
		free(bufferBack);
	}

	slen = (int)strlen(szPicFileLeft);
	if (0 != slen)
	{
		length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileLeft, slen, NULL, 0);
		wchar_t* bufferLeft = (wchar_t*)malloc(slen * 2 + 2);
		memset((char *)bufferLeft, 0, slen * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileLeft, slen, bufferLeft, length);
		wcscpy((wchar_t *)picFileLeft, bufferLeft);
		free(bufferLeft);
	}

	slen = (int)strlen(szPicFileRight);
	if (0 != slen)
	{
		length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileRight, slen, NULL, 0);
		wchar_t* bufferRight = (wchar_t*)malloc(slen * 2 + 2);
		memset((char *)bufferRight, 0, slen * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileRight, slen, bufferRight, length);
		wcscpy((wchar_t *)picFileRight, bufferRight);
		free(bufferRight);
	}

	return 0;
}

/*获取整个深度，单位cm*/
extern "C" __declspec(dllexport) int getDeep(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(1);
	if (!channel)
		return 0;

	return (int)(channel->getChannelHeader()->getTimeWindow() * channel->getChannelHeader()->getSoilVel() / 20);
}

/*获取垂直分辨率*/
extern "C" __declspec(dllexport) int getSampleCount(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(1);
	if (!channel)
		return 0;

	return channel->getChannelHeader()->getSample();
}

/*----------------------------下面是MatGpr计算处理-------------------------*/
/*
Fun:初始化，读取指定测线和通道数据
Param:swathName 指定的测线名
*/
extern "C" __declspec(dllexport) int initSwath(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、初始化测线数据
	int iResult = theApp.InitSwath(swath);
	if (0 != iResult)
		return ERROR_CODE_SYS;
	else
		return ERROR_CODE_SUCCESS;
}

/*
Fun:去初始化，釋放指定测线的通道数据
Param:swathName 指定的测线名
*/
extern "C" __declspec(dllexport) int unInitSwath(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、去初始化测线数据
	int iResult = theApp.UnInitSwath(swath);
	if (0 != iResult)
		return ERROR_CODE_SYS;
	else
		return ERROR_CODE_SUCCESS;
}



/*
Fun:备份原有数据
*/
extern "C" __declspec(dllexport) int BackupData(const char * swathName, const int channelID)
{
	/*
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameDat; fileNameHeader; path");
	//MessageBox(NULL, szOut, szOut, 0);

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);

	//备份原有数据
	bool bReturn = theApp.BackupData(fileNameData, fileNameHeader, path);
	if (!bReturn)
		return 0;
	else
		return -1;
	*/

	return ERROR_CODE_SUCCESS;
}


/*
Fun:删除无效的trace
Param:const char *  swathName   目标测线
				int channelID   通道号
*/
extern "C" __declspec(dllexport) int deleteInvalidTrace(const char *  swathName, int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char path[512] = { 0 };
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//删除无效Trace，并且存入新的文件
	bool bReturn = theApp.deleteInvalidTrace(szSwathName, channelID, path, swath);
	if (bReturn)
		return ERROR_CODE_SUCCESS;
	else
		return ERROR_CODE_SYS;
}


/*
Fun:生成指定测线通道的直达波图像
Param:swathName      指定的测线名
	  channelID      通道号
	  sigPositionPic 目标直达波图像文件
	  traceNum       直达波对应的trace号，大于0表明直达波为对应的trace对应的直达波，小于0表示为整个图像的直达波
*/
extern "C" __declspec(dllexport) int SigPositionPicByVende(const char * swathName, const int channelID, const char * sigPositionPic, const int traceNum)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化测线名称
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4、对直达波图像存储路径和文件名做简单的校验
	length = (int)wcslen((wchar_t*)sigPositionPic);
	if (length <= 0)
		return ERROR_PARAM;

	char szSigPositionPic[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化直达波路径和名称
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)sigPositionPic, length, szSigPositionPic, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	/*
	//调试打印输出
	//char fileNameData[512]   = { "RadarData_001_A01.iprb" };
	//char fileNameHeader[512] = { "RadarData_001_A01.rad" };
	//char path[512] = { "C:\\3D地理信息系统\\来自张国忠\\2\\0928test_2\\RadarData\\" };

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	char szOut[512] = { 0 };
	sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	MessageBox(NULL, szOut, szOut, 0);
	*/

	//产生直达波图像
	return theApp.SigPosPicureByVende(channel, szSigPositionPic, traceNum);
}

/*
Fun:切除指定的直达波图像,同时返回切除的直达波位置-----用于自动切除直达波，并且返回切除深度
Param:swathName 指定的测线名
	  channelID 通道号
Return: 切除成功，返回切除的位置
		切除失败，返回0
*/
extern "C" __declspec(dllexport) int SigPositionCutEx2ByVende(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化测线名称
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	int traceCount = channel->getChannelHeader()->getTraceCount();
	//查找一个数据合法的Trace，进行直达波计算
	int traceNum = 1;
	for (; traceNum < traceCount; traceNum = traceNum + 10)
	{
		int traceValue = 0;
		theApp.TraceAvg(channel, traceNum, traceValue);

		//当Trace的平均值大于一定数值，说明数据是有效的
		if (traceValue > 100)
			break;
	}

	//获取直达波位置
	int iZero = theApp.MiniSigPosNum(channel, traceNum, 2);
	int iTmpDample = channel->getChannelHeader()->getSample();  //用于后续计算

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//产生去直达波图像
	bool bReturn = theApp.SimpleSigPositionCut(fileNameData, fileNameHeader, path, channel, iZero);
	if (bReturn)
	{
		//将切除的深度值，按比例进行放大（因为直达波图像的高度为1000像素）
		return iZero * 1000 / iTmpDample;
	}
	else
		return 0;
}

/*
Fun:切除指定深度的直达波图像
Param:swathName 指定的测线名
	  channelID 通道号
	  iZero     切除深度
*/
extern "C" __declspec(dllexport) int SigPositionCutEx3ByVende(const char * swathName, const int channelID, int iZero)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化测线名称
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//将直达波图像中的像素转化为实际深度像素
	int iTmpDample = channel->getChannelHeader()->getSample();  //用于后续计算
	iZero = iZero * iTmpDample / 1000;

	//切除直达波
	return theApp.SigPositionCutByVende(fileNameData, fileNameHeader, path, channel, iZero);
}

/*
Fun:对图像進行帶通滤波(高斯滤波器)
Param:swathName 指定的测线名
	  channelID 通道号
	  freqMax   通帶最大頻率
	  freqMin   通帶最小頻率
	  order     阶数，越大图像越精細，但運算速度越慢(取值範圍1-10)
*/
extern "C" __declspec(dllexport) int FilterGauss(const char * swathName, const int channelID, int freqMax, int freqMin, int order)
{
	//暫不支持
	return -1;
}

/*
Fun:对图像進行帶通滤波(理想滤波器)
Param:swathName 指定的测线名
	  channelID 通道号
	  freqMax   通帶最大頻率
	  freqMin   通帶最小頻率
	  order     阶数，越大图像越精細，但運算速度越慢(取值範圍1-10)
*/
extern "C" __declspec(dllexport) int FilterIdeal(const char * swathName, const int channelID, int freqMax, int freqMin, int order)
{
	//暫不支持
	return -1;

}



/*----------------------------下面是簡单的加解密处理-------------------------*/
/*
 * Fun:数据簡单加密处理
 * Param:
 *   src 待加密的原始数据
 *   dst 加密后的目标数据
 *   length 原始数据长度
 */
extern "C" __declspec(dllexport) int DataSimpleCrypt(const unsigned char *src, unsigned char *dst, int length)
{
	//参数合法性校驗
	if ((!src) || (!dst))
		return ERROR_PARAM;

	//每个字节循環取反
	for (int i = 0; i < length; i++)
	{
		dst[i] = src[i] ^ 0xff;
	}

	return 0;
}

/*
 * Fun:数据簡单解密处理
 * Param:
 *   src 待解密的原始数据
 *   dst 解密后的目标数据
 *   length 原始数据长度
 */
extern "C" __declspec(dllexport) int DataSimpleDecrypt(const unsigned char *src, unsigned char *dst, int length)
{
	//参数合法性校驗
	if ((!src) || (!dst))
		return ERROR_PARAM;

	//每个字节循環取反
	for (int i = 0; i < length; i++)
	{
		dst[i] = src[i] ^ 0xff;
	}

	return 0;
}
/*----------------------------上面是簡单的加解密处理-------------------------*/





//==========================下面一组接口：直流噪声去除：不使用Matlab的库，由C++直接实现====================//
/*
Fun:去除直流噪声
Param:swathName 指定的测线名
	  channelID 通道号
*/
/*
extern "C" __declspec(dllexport) int RemoveDC(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);
	//产生去直达波图像
	bool bReturn = theApp.RemoveDC(fileNameData, fileNameHeader, path);
	if (!bReturn)
		return 0;
	else
		return -1;
}
*/
//==========================上面一组接口：直流噪声去除：不使用Matlab的库，由C++直接实现====================//



//==========================下面一组接口：背景噪声去除：不使用Matlab的库，由C++直接实现====================//
//待实现

//==========================上面一组接口：背景噪声去除：不使用Matlab的库，由C++直接实现====================//

/*
Fun:对图像進行帶通滤波(理想滤波器)
Param:swathName 指定的测线名
	  channelID 通道号
	  freqMax   通帶最大頻率
	  freqMin   通帶最小頻率
	  order     阶数，越大图像越精細，但運算速度越慢(取值範圍1-10)
*/
extern "C" __declspec(dllexport) int FilterIdealSimple(const char * swathName, const int channelID, int freqMax, int freqMin, int order)
{
	//暫不支持
	return -1;

}


//==========================下面一组接口：直达波处理：不使用Matlab的库，由C++直接实现======================//

/*
Fun:生成指定测线通道的直达波图像
Param:swathName      指定的测线名
	  channelID      通道号
	  sigPositionPic 目标直达波图像文件
*/
extern "C" __declspec(dllexport) int SimpleSigPositionPic(const char * swathName, const int channelID, const char * sigPositionPic)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化测线名称
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4、对直达波图像存储路径和文件名做简单的校验
	length = (int)wcslen((wchar_t*)sigPositionPic);
	if (length <= 0)
		return ERROR_PARAM;

	char szSigPositionPic[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化直达波路径和名称
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)sigPositionPic, length, szSigPositionPic, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//产生直达波图像
	return theApp.SigPosPicureByVende(channel, szSigPositionPic, 10);
}

/*
Fun:获取指定测线通道的直达波位置
Param:swathName 指定的测线名
	  channelID 通道号
*/
extern "C" __declspec(dllexport) int SimpleSigPositionGet(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化测线名称
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//因为前几个Trace的数据容易出问题，所以跳过前几个Trace数据进行计算
	//获取直达波位置
	return theApp.SigPosNumByVende(channel, 0, 1);
}

/*
Fun:获取指定测线通道的直达波位置
Param:swathName 指定的测线名
	  channelID 通道号
	  direct    直达波方向 -1：左边直达波的波峰； 0：两边直达波的波峰； 1：右边直达波的波峰；
	  waveNum   指定从上向下的第几个波峰
*/
extern "C" __declspec(dllexport) int SimpleSigPositionGetEx(const char * swathName, const int channelID, int direct, const int waveNum)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化测线名称
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//因为前几个Trace的数据容易出问题，所以跳过前几个Trace数据进行计算
	//获取直达波位置
	return theApp.SigPosNumByVende(channel, direct, waveNum);
}

/*
Fun:切除指定的直达波图像
Param:swathName 指定的测线名
	  channelID 通道号
	  iZero     切除深度
*/
extern "C" __declspec(dllexport) int SimpleSigPositionCut(const char * swathName, const int channelID, int iZero)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化测线名称
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//切除直达波
	return theApp.SimpleSigPositionCut(fileNameData, fileNameHeader, path, channel, iZero);
}

/*
Fun:切除指定的直达波图像，自动计算直达波深度
Param:swathName 指定的测线名
	  channelID 通道号
*/
extern "C" __declspec(dllexport) int SimpleSigPositionCutEx(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串---此处转化测线名称
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//获取直达波位置，//因为前几个Trace的数据容易出问题，所以跳过前几个Trace数据进行计算
	int iZero = theApp.SigPosNumByVende(channel, 0, 1);

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//切除直达波
	return theApp.SimpleSigPositionCut(fileNameData, fileNameHeader, path, channel, iZero);
}


//==========================上面一组接口：直达波处理：不使用Matlab的库，由C++直接实现======================//


//==========================下面一组接口：逆振幅：不使用Matlab的库，由C++直接实现==========================//
/*
Fun:根据深度增加自动增益，
	信号增加的幅度由用户手动指定，底层在处理数据时，根据实际情况在原有数据的的基础上增加对应的偏移
	注意，使用增益处理前一定要完成直达波切除
Param:swathName 指定的测线名
	  channelID 通道号
	  order:    增益阶数(系数的数量)，介于1到最大采样点数之间，越大越平滑
	  offset:   偏移系数，采用由上到下进行排列
*/
extern "C" __declspec(dllexport) int SimpleGainInvDecayConst(const char * swathName, const int channelID, int order, int *offset)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4、校验增益参数
	if (order <= 0)
		return ERROR_PARAM;    //后续这里可以内置一组默认的增益参数
	if (!offset)
		return ERROR_PARAM;
	int iTmpSample = channel->getChannelHeader()->getSample();  //用于后续计算
	if (order > iTmpSample)
		order = iTmpSample;


	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5、实现逆振幅增益
	return theApp.GainInvDecayConst(fileNameData, path, channel, order, offset);
}

/*
Fun:根据深度增加自动增益，
	信号增加的幅度由用户手动指定，底层在处理数据时，根据实际情况在原有数据的的基础上增加对应的系数比例的值
	注意，使用增益处理前一定要完成直达波切除
	Param : swathName 指定的测线名
	channelID 通道号
	order : 增益阶数(系数的数量)，介于1到最大采样点数之间，越大越平滑
	coef  : 增益系数，采用由上到下进行排列(取值范围 1 ... ... 100)
*/
extern "C" __declspec(dllexport) int SimpleGainInvDecayCoef(const char * swathName, const int channelID, int order, int *coef)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4、校验增益参数
	if (order <= 0)
		return ERROR_PARAM;    //后续这里可以内置一组默认的增益参数
	if (!coef)
		return ERROR_PARAM;
	int iTmpSample = channel->getChannelHeader()->getSample();  //用于后续计算
	if (order > iTmpSample)
		order = iTmpSample;


	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5、实现逆振幅增益
	return theApp.GainInvDecayCoef(fileNameData, path, channel, order, coef);
}

/*
Fun:根据深度增加自动增益 ----- 正曲线
	信号增加的幅度由模型公式计算。
	信号衰减模型: y = b - b * (k/100) * (( x ^ n )/sample)
	注意，使用增益处理前一定要完成直达波切除
	Param : swathName 指定的测线名
	channelID 通道号
	k : 斜率，取值范围为1 ... ... 100
	n : 指数，取值范围为1 ... ... 10
*/
extern "C" __declspec(dllexport) int SimpleGainInvDecayCurve(const char * swathName, const int channelID, int k, int n)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4、校验增益参数
	if ((k <= 0) || (n <= 0))
	{
		//默认值
		k = 25;
		n = 19;
	}

	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5、实现逆振幅增益
	return theApp.GainInvDecayCurve(fileNameData, path, channel, k, n);
}

//==========================上面一组接口：逆振幅：不使用Matlab的库，由C++直接实现==========================//

//==========================下面一组接口：去背景噪声：不使用Matlab的库，由C++直接实现==========================//
/*
Fun:去除背景噪声
Param:swathName 指定的测线名
	  channelID 通道号
*/
extern "C" __declspec(dllexport) int SimpleRemoveBackgr(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5、删除背景噪声
	return theApp.SimpleRemoveBackgr(fileNameData, path, channel);
}
/*
Fun:去除背景噪声
Param:swathName 指定的测线名
	  channelID 通道号
	  offerset  背景噪声，消除偏移，浅层背景噪声消除要少一点，深层背景噪声要消除多一点。
*/
extern "C" __declspec(dllexport) int SimpleRemoveBackgrEx(const char * swathName, const int channelID, int offerset)
{
	return -1;
}
//==========================上面一组接口：去背景噪声：不使用Matlab的库，由C++直接实现==========================//



//==========================下面一组接口：删除直流噪声：不使用Matlab的库，由C++直接实现========================//
/*
*Fun:删除直流噪声
*Param:swathName 指定的测线名
*	  channelID 通道号
*/
extern "C" __declspec(dllexport) int SimpleRemoveDC(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1、判断输入参数长度是否合法
	if (strlen(szSwathName) <= 0)
		return 0;

	//2、根据测线名称，取得测线对象
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3、在测线对象中，根据通道ID，取得通道的对象
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5、删除背景噪声
	return theApp.SimpleRemoveDC(fileNameData, path, channel);
}

//==========================上面一组接口：删除直流噪声：不使用Matlab的库，由C++直接实现========================//


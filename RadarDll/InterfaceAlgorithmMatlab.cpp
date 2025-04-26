/*
 * Fun:采用Matlab的算法库对图像数据进行处理
 */

#include "framework.h"
#include <io.h>
//#include <atlimage.h>

#include "RadarMath.h"
#include "Utils\\RadarConst.h"

//工程目录
extern char szProjectPathC[];

/*
Fun:RadarMath库初始化
*/
extern "C" __declspec(dllexport) bool RadarMathInit()
{
	//AfxMessageBox(_T("init RadarMath Start !!!"));

	bool bInit = RadarMathInitialize();
	if (!bInit)
	{
		//AfxMessageBox(_T("init RadarMath NOK !!!"));
		return false;
	}
	else
	{
		//AfxMessageBox(_T("init RadarMath OK !!!"));
		return true;
	}
}

/*
Fun:RadarMath库初始化
*/
extern "C" __declspec(dllexport) bool RadarMathInitEx(const char * projectPath)
{
	strcpy(szProjectPathC, projectPath);

	bool bInit = 0;

	bInit = RadarMathInitialize();
	if (!bInit)
	{
		//AfxMessageBox(_T("init RadarMath NOK !!!"));
		return false;
	}
	else
	{
		//AfxMessageBox(_T("init RadarMath OK !!!"));
		return true;
	}
}

/*
Fun:RadarMath库去初始化
*/
extern "C" __declspec(dllexport) void RadarMathUninit()
{
	RadarMathTerminate();
}

/*
Fun:生成指定测线通道的直达波图像-----这个函数底层采用Matlab进行运算，该函数即将废除或改名为SigPositionPicByMatGPR
Param:swathName      指定的测线名
	  channelID      通道号
	  sigPositionPic 目标直达波图像文件
*/
extern "C" __declspec(dllexport) int SigPositionPic(const char * swathName, const int channelID, const char * sigPositionPic)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	length = (int)wcslen((wchar_t*)sigPositionPic);
	if (length <= 0)
		return ERROR_PARAM;

	char szSigPositionPic[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)sigPositionPic, length, szSigPositionPic, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//char fileNameData[512]   = { "RadarData_001_A01.iprb" };
	//char fileNameHeader[512] = { "RadarData_001_A01.rad" };
	//char path[512] = { "C:\\3D地理信息系统\\来自张国忠\\2\\0928test_2\\RadarData\\" };

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//AfxMessageBox(fileNameData);
	//AfxMessageBox(fileNameHeader);
	//AfxMessageBox(path);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray STR_FILE_PIC(szSigPositionPic);

	//产生直达波图像
	ISigPositionPic(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, STR_FILE_PIC);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:获取指定测线通道的直达波位置----底层调用Matlab实现
Param:swathName 指定的测线名
	  channelID 通道号
*/
extern "C" __declspec(dllexport) int SigPositionNum(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//char fileNameData[512]   = { "RadarData_001_A01.iprb" };
	//char fileNameHeader[512] = { "RadarData_001_A01.rad" };
	//char path[512] = { "C:\\3D地理信息系统\\来自张国忠\\2\\0928test_2\\RadarData\\" };

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray Result;
	int nargout = 1;
	//获取直达波位置
	ISigPositionNum(nargout, Result, STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	return Result.Get(1, 1);
}

/*
Fun:获取指定测线通道的直达波位置
Param:swathName 指定的测线名
	  channelID 通道号
	  direct    直达波方向 1：右边直达波的波峰； -1：左边直达波的波峰； 0：两边直达波的波峰
*/
extern "C" __declspec(dllexport) int SigPositionNumEx(const char * swathName, const int channelID, int direct)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//下面将Java传递的Unicode字符串转化为ASCII字符串
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//char fileNameData[512]   = { "RadarData_001_A01.iprb" };
	//char fileNameHeader[512] = { "RadarData_001_A01.rad" };
	//char path[512] = { "C:\\3D地理信息系统\\来自张国忠\\2\\0928test_2\\RadarData\\" };

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_DIRECT(direct);
	mwArray Result;
	int nargout = 1;
	//产生直达波图像
	ISigPositionNumEx(nargout, Result, STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_DIRECT);

	return Result.Get(1, 1);

}

/*
Fun:切除指定的直达波图像
Param:swathName 指定的测线名
	  channelID 通道号
	  iZero     直达波位置
*/
extern "C" __declspec(dllexport) int SigPositionCut(const char * swathName, const int channelID, int iZero)
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

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_DEEP(iZero);
	//切除直达波图像
	ISigPositionCut(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_DEEP);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:去除背景噪声
Param:swathName 指定的测线名
	  channelID 通道号
	  offerset  背景噪声，消除偏移
*/
extern "C" __declspec(dllexport) int RemoveBackgr(const char * swathName, const int channelID )
//extern "C" __declspec(dllexport) int RemoveBackgr(const char * swathName, const int channelID, const int offset)
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
	//int offerset   = 0;

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_OFFSET(5);

	//去背景噪声
	IRmBackGr(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_OFFSET);
	//IRmBackGr(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:去除背景噪声
Param:swathName 指定的测线名
	  channelID 通道号
	  scaleBase 背景去噪时，该值越大上层数据去掉的噪声越小。针对Impluse 450M雷达。取值0-3，0表示保持现有的处理程度。
*/
extern "C" __declspec(dllexport) int RemoveBackgr450M(const char * swathName, const int channelID, int scaleBase)
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

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_SCALE_BASE(scaleBase);

	//去背景噪声
	IRmBackGr450M(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_SCALE_BASE);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:去除直流噪声
Param:swathName 指定的测线名
	  channelID 通道号
*/
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

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);

	//去直流噪声
	IRemoveDC(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:根据深度增加自动增益
Param:swathName 指定的测线名
	  channelID 通道号
*/
extern "C" __declspec(dllexport) int GainInvDecay(const char * swathName, const int channelID)
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

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);

	//处理逆振幅衰減的自动增益
	IGainInvDecay(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:根据深度增加自动增益
Param:swathName 指定的测线名
	  channelID 通道号
	  curve     曲线類型，默認为1
	  order     阶数，越大图像越精細，但運算速度越慢(取值範圍1-10)
*/
extern "C" __declspec(dllexport) int GainInvDecayEx(const char * swathName, const int channelID, int curve, int order)
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

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);

	mwArray INT_CURVE(curve);
	mwArray INT_ORDER(order);

	//处理逆振幅衰減的自动增益
	IGainInvDecayEx(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_CURVE, INT_ORDER);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:对图像進行帶通滤波(巴特沃斯滤波器)
Param:swathName  指定的测线名
	  channelID  通道号
	  freqStart  通帶開始頻率  -- 最小
	  freqEnd    通帶結束頻率  -- 最大
Return: 成功：0； 失败：错误码
*/
extern "C" __declspec(dllexport) int FilterButterworth(const char * swathName, const int channelID, int freqStart, int freqEnd)
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

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);
	//return 0;

	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);

	mwArray INT_FreqStart(freqStart);
	mwArray INT_FreqEnd(freqEnd);


	//char szOut[512] = { 0 };
	//sprintf(szOut, "IPRB:%s; IPRH:%s; PATH:%s; freqStart:%d; freqEnd:%d; ", fileData, fileHeader, path, freqStart, freqEnd );
	//MessageBox(NULL, szOut, szOut, 0);
	//return true;

	//帶通滤波
	IButterworthPassBand(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_FreqStart, INT_FreqEnd);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:对图像進行KL变换
Param:swathName 指定的测线名
	  channelID 通道号
	  tr        变换值1---无穷大，值越小效果越明显
Return: 成功：0； 失败：错误码
*/
extern "C" __declspec(dllexport) int TransferKL(const char * swathName, const int channelID, int tr)
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

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);

	//图像KL变换
	//bool bReturn = theApp.TransferKL(fileNameData, fileNameHeader, path, tr);
	//输入参数
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_TR(tr);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "IPRB:%s; IPRH:%s; PATH:%s; freqSample:%d; freqStart:%d; freqEnd:%d; ", fileData, fileHeader, path, freq, freqStart, freqEnd );
	//MessageBox(NULL, szOut, szOut, 0);

	//KL变换
	IKarhunenLoeve(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_TR);

	return ERROR_CODE_SUCCESS;
}


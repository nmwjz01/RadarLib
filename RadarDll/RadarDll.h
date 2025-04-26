// RadarDll.h: RadarDll DLL 的主标头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// 主符号


// CRadarDllApp
// 有关此类实现的信息，请参阅 RadarDll.cpp
//

class CRadarDllApp : public CWinApp
{
public:
	CRadarDllApp();
	~CRadarDllApp();

// 重写
public:
	virtual BOOL InitInstance();

	int SaveAsPicV(CString strPathFile, SwathChannel * pChannel, int iStart, int iCount, int iHeight);
	int SaveAsPicH(CString strPathFile, Swath * pSwath, int iDeep, int iStart, int iCount);
	int TakePhotoPic(Project *pProject, char * picFileFront, char * picFileBack, char * picFileLeft, char * picFileRight, Swath* swath, int traceNum, int iOffsetTime);
	int InitSwath(Swath * pSwath);
	int UnInitSwath(Swath * pSwath);

	void SetContrast(int iContrast) { m_Contrast = iContrast; }

	void SetGain(int iGain) { m_Gain = iGain; }
	void SetColor(int iColor, int iMask) { m_Color = iColor; m_ColorMask = iMask; }
	void SetDeep(int iDeep[32])
	{
		for (int i = 0; i < 32; i++)
			m_Deep[i] = iDeep[i];
	}

	//备份原有的测线通道数据
	//bool BackupData(char *fileData, char *fileHeader, char *path);

	//删除无效Trace
	bool deleteInvalidTrace(const char * pSwathName, int channelID, const char *path, Swath * pSwath);

	//去直达波--产生直达波图形
	int SigPosPicureByVende(SwathChannel * pChannel, char *filePic, int traceNum);
	//取直达波--获取直达波位置
	int SigPosNumByVende(SwathChannel * pChannel, int direct, int waveNum);
	//取直达波--获取直达波位置
	int SigPosNumExByVende(SwathChannel * pChannel, int traceNum, int direct);
	//去直达波--裁剪直达波图形
	bool SimpleSigPositionCut(char *fileData, char *fileHeader, char *path, SwathChannel * pChannel, int iZero);
	//获取直达波位置
	int MiniSigPosNum(SwathChannel * pChannel, int traceNum, int waveNum);
	int MiniSigPosNum2(SwathChannel * pChannel, int traceNum, int waveNum);
	bool SigPositionCutByVende(char *fileData, char *fileHeader, char *path, SwathChannel * pChannel, int iZero);

	//逆振幅衰减，按照参数进行增大偏移
	int GainInvDecayConst(char *fileData, char *path, SwathChannel * channel, int order, int *coef);
	int GainInvDecayCoef(char *fileData, char *path, SwathChannel * channel, int order, int *coef);
	int GainInvDecayCurve(char *fileData, char *path, SwathChannel * channel, int k, int n);

	//去背景噪声
	int SimpleRemoveBackgr(char *fileData, char *path, SwathChannel * channel);
	//去直流噪声
	int SimpleRemoveDC(char *fileData, char *path, SwathChannel * channel);

	//计算Trace中数据的平均值，用于数据合法性判断
	int TraceAvg(SwathChannel * pChannel, int traceNum, int &traceValue);

	DECLARE_MESSAGE_MAP()

private:
	//转化UTC时间到北京时间
	void TransferTime(char * pTime, int iOffsetTime);

	//删除无效Trace
	int DeleteInvalidTrace(SwathChannelBlob * channelBlob, int dataType);

private:
	int m_Contrast;    //对比度
	int m_Gain;        //增益
	int m_Deep[32];    //深度
	int m_Color;       //显示颜色
	int m_ColorMask;   //显示颜色亮暗取反。

	BOOL m_SwathLoaded;   //测线数据是否已經被加載
};

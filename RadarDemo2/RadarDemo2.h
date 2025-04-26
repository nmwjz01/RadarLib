
// RadarDemo2.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// 主符号


// CRadarDemo2App:
// 有关此类的实现，请参阅 RadarDemo2.cpp
//

class CRadarDemo2App : public CWinApp
{
public:
	CRadarDemo2App();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CRadarDemo2App theApp;

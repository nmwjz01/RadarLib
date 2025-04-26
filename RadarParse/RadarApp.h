
// RadarParse.h: RadarParse 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号

#include "Project.h"

// RadarApp:
// 有关此类的实现，请参阅 RadarParse.cpp
//

class RadarApp : public CWinAppEx
{
public:
	RadarApp() noexcept;

// 重写
public:
	virtual BOOL InitInstance();

// 实现
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileExport();

	afx_msg void OnOpenLeft();
	afx_msg void OnFileRight();
	afx_msg void OnSettingZLevel();
	afx_msg void OnOpenSwath();

	DECLARE_MESSAGE_MAP()

private:
	Project * m_pProject  = NULL;

	void CfgFileRead();       //读取配置文件
	void CfgFileWrite();      //写入配置文件

	CString m_Path;           //工程根目录
	CString m_PathRadar;     //雷达目录
	CString m_PathCamera1;    //摄像头1目录
	CString m_PathCamera2;    //摄像头2目录
	CString m_PathCamera3;    //摄像头3目录
	CString m_PathCamera4;    //摄像头4目录

};

extern RadarApp theApp;

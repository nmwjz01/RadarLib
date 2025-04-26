#pragma once

#include "stdafx.h"

// 用于应用程序“关于”菜单项的 AboutDlg 对话框
class AboutDlg : public CDialogEx
{
public:
	AboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
	{}

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV 支持
	{
		CDialogEx::DoDataExchange(pDX);
	}

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(AboutDlg, CDialogEx)
END_MESSAGE_MAP()
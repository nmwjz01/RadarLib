#pragma once

#include "stdafx.h"

// ����Ӧ�ó��򡰹��ڡ��˵���� AboutDlg �Ի���
class AboutDlg : public CDialogEx
{
public:
	AboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
	{}

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV ֧��
	{
		CDialogEx::DoDataExchange(pDX);
	}

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(AboutDlg, CDialogEx)
END_MESSAGE_MAP()
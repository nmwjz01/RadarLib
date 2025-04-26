#pragma once


// PicProcess 对话框

class PicProcess : public CDialogEx
{
	DECLARE_DYNAMIC(PicProcess)

public:
	PicProcess(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~PicProcess();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_PIC_PROCESS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPicBrowser();
	afx_msg void OnBnClickedPicSplitV();
	afx_msg void OnBnClickedPicSplitH();
};

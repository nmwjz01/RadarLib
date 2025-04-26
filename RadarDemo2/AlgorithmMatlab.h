#pragma once


// AlgorithmMatlab 对话框

class AlgorithmMatlab : public CDialogEx
{
	DECLARE_DYNAMIC(AlgorithmMatlab)

public:
	AlgorithmMatlab(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~AlgorithmMatlab();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_ALGORITHM_MATLAB };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMatlabSigpositionPic();
	afx_msg void OnBnClickedMatlabSigpositionCut();
	afx_msg void OnBnClickedMatlabSigpositionGet();
	afx_msg void OnBnClickedMatlabSigpositionBrowser();
	afx_msg void OnBnClickedMatlabBackground();
	afx_msg void OnBnClickedMatlabGaininvdecay();
	afx_msg void OnBnClickedMatlabFilterBandpass();
};

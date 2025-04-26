#pragma once


// AlgorithmSimple 对话框

class AlgorithmSimple : public CDialogEx
{
	DECLARE_DYNAMIC(AlgorithmSimple)

public:
	AlgorithmSimple(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~AlgorithmSimple();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_ALGORITHM_SIMPLE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSimpleSigpositionBrowser();
	afx_msg void OnBnClickedSimpleSigpositionGet();
	afx_msg void OnBnClickedSimpleSigpositionPic();
	afx_msg void OnBnClickedSimpleSigpositionCut();
	afx_msg void OnBnClickedSimpleGaininvdecay();
	afx_msg void OnBnClickedSimpleBackground();
	afx_msg void OnBnClickedSimpleRemovedc();

};

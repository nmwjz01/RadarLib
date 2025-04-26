#pragma once


// DataTransfer 对话框

class DataTransfer : public CDialogEx
{
	DECLARE_DYNAMIC(DataTransfer)

public:
	DataTransfer(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DataTransfer();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_TRANSFER_IMPLUSE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBrowserInput();
	afx_msg void OnBnClickedBrowserOutput();
	afx_msg void OnBnClickedTransferSegy();
	afx_msg void OnBnClickedTransferDt();
	afx_msg void OnBnClickedTransferIds();
	afx_msg void OnBnClickedTransferIds16();
	afx_msg void OnBnClickedTransferMala();
	afx_msg void OnBnClickedTransferMalaEx();
	afx_msg void OnBnClickedTransfer3dradar();
	afx_msg void OnBnClickedTransferGssi();
	afx_msg void OnBnClickedTransferIds08();
};

#pragma once


// DataTransferToSegy 对话框

class DataTransferToSegy : public CDialogEx
{
	DECLARE_DYNAMIC(DataTransferToSegy)

public:
	DataTransferToSegy(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DataTransferToSegy();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_TRANSFER_TOSEGY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedTransferSplit();
};

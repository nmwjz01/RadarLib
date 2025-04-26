#pragma once


// DialogZLevel 对话框

class DialogZLevel : public CDialogEx
{
	DECLARE_DYNAMIC(DialogZLevel)

public:
	DialogZLevel(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DialogZLevel();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ZLEVEL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CSliderCtrl m_oZLevelMainOne;
	CSliderCtrl m_oZLevelMainTwo;
	CSliderCtrl m_oZLevelLeftOne;
	CSliderCtrl m_oZLevelLeftTwo;
	CSliderCtrl m_oZLevelLeftThree;
	CSliderCtrl m_oZLevelRightOne;
	CSliderCtrl m_oZLevelRightTwo;
	CSliderCtrl m_oZLevelRightThree;

private:
	void UpdateMainOne();
	void UpdateMainTwo();

	void UpdateLeftOne();
	void UpdateLeftTwo();
	void UpdateLeftThree();

	void UpdateRightOne();
	void UpdateRightTwo();
	void UpdateRightThree();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedZlevelSave();
};

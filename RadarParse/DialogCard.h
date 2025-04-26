#pragma once


// DialogCard 对话框

class DialogCard : public CDialogEx
{
	DECLARE_DYNAMIC(DialogCard)

public:
	DialogCard(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DialogCard();

	void SetParam(Project* pProject, Swath* pSwath, SwathChannel* pChannel, CRect oRect);

	//在指定的通道上创建开始trace到结束trace的侧视图
	void MakeCardSideView(int iTraceStart, int iTraceEnd, SwathChannel* pChannel);
	//指定开始Trace和结束Trace，指定深度，创建俯视图
	void MakeCardTopView(int iTraceStart, int iTraceEnd, Swath* pSwath, int iDeep);
	//在指定Trace的位置，创建截视图
	void MakeCardCutView(int iTrace, Swath* pSwath);
	//计算开始trace和结束trace
	void GetStartEndTrace(Project* pProject, Swath* pSwath, SwathChannel* pChannel, int iCurrent, int Length, int& iStart, int& iEnd);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CARD};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	Project     * m_pProject = NULL;
	Swath       * m_pSwath   = NULL;
	SwathChannel* m_pChannel = NULL;
	int           m_iCurrent = 0;
public:
	afx_msg void OnBnClickedCardCreate();
	afx_msg void OnPaint();
private:
	// 接受用户输入的编号
	CString m_CardNo;
	// 坐标B
	CString m_B;
	// 坐标L
	CString m_L;
	// 坐标N
	CString m_N;
	// 坐标E
	CString m_E;
	// 病害尺寸
	CString m_Size;
	// 病害深度
	CString m_Deep;
	// 病害位置
	CString m_Position;
	// 病害时间
	CString m_Time;
	//净高
	CString m_Height;
	//抓拍照片
	CString m_PictureFront;
	CString m_PictureBack;
	CString m_PictureLeft;
	CString m_PictureRight;
	//雷达数据图片
	CString m_LadarPicture1;
	CString m_LadarPicture2;
	CString m_LadarPicture3;
	CString m_LadarPicture4;
private:
	//显示前置抓拍机的照片
	void DisplayPhotoFront();
	//显示后置抓拍机的照片
	void DisplayPhotoBack();
	//显示左置抓拍机的照片
	void DisplayPhotoLeft();
	//显示右置抓拍机的照片
	void DisplayPhotoRight();
	//转化UTC时间到北京时间
	void TransferTime(char * pTime);

	//这是一个完整的Demo，用于创建word文档(可用)
	void MakeCardDoc(CString strCardFile);

	CComboBox m_oLevel;
	CComboBox m_oType;
public:
	virtual BOOL OnInitDialog();

};

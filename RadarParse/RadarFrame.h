
// RadarParseMainFrm.h: RadarFrame 类的接口
//

#pragma once
#include "RadarViewSwathList.h"
#include "OutputWnd.h"
#include "RadarViewSetting.h"
#include "Project.h"
#include "RadarViewMain.h"

#define VIEW_TYPE_ALL   0
#define VIEW_TYPE_MAIN  1
#define VIEW_TYPE_LEFT  2
#define VIEW_TYPE_RIGHT 3

class RadarFrame : public CFrameWndEx
{
	
protected: // 仅从序列化创建
	RadarFrame() noexcept;
	DECLARE_DYNCREATE(RadarFrame)

// 操作
public:
	void setStatusBar( CString strSwathName , CString strChannel );

	void ProjectOpen(Project *pProject);
	void ProjectExport(Project *pProject);

	//打开左视图
	void OpenViewLeft();
	//打开右视图
	void OpenViewRight();

	//主视图第一视图
	void SetChannelViewMainOne(Swath* pSwath, SwathChannel* pChannel);
	//主视图第二视图
	void SetChannelViewMainTwo(Swath* pSwath, SwathChannel* pChannel);

	//左视图第一视图
	void SetChannelViewLeftOne(Swath* pSwath, SwathChannel* pChannel);
	//左视图第二视图
	void SetChannelViewLeftTwo(Swath* pSwath, SwathChannel* pChannel);
	//左视图第三视图
	void SetChannelViewLeftThree(Swath* pSwath, SwathChannel* pChannel);

	//右视图第一视图
	void SetChannelViewRightOne(Swath* pSwath, SwathChannel* pChannel);
	//右视图第二视图
	void SetChannelViewRightTwo(Swath* pSwath, SwathChannel* pChannel);
	//右视图第三视图
	void SetChannelViewRightThree(Swath* pSwath, SwathChannel* pChannel);

	//获取主视图的View
	RadarViewMain *GetViewMain();
	//获取左视图的View
	RadarViewAssistant *GetViewLeft();
	//获取右视图的View
	RadarViewAssistant *GetViewRight();
	//更新所有界面显示
	void UpdateAllView( int iType= VIEW_TYPE_ALL);
	//打开测线树
	void OpenSwath();
// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

// 实现
public:
	virtual ~RadarFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:  // 控件条嵌入成员
	CMFCMenuBar          m_wndMenuBar;
	CMFCStatusBar        m_wndStatusBar;
	COutputWnd           m_wndOutput;
	CSplitterWnd         m_mainSplitter;
	RadarViewSwathList  m_wndSwathListView;     //测线树视图
	RadarViewMain      *m_pViewMain    = NULL;  //主视图
	RadarViewAssistant *m_pViewLeft    = NULL;  //左视图
	RadarViewAssistant *m_pViewRight   = NULL;  //右视图
	RadarViewSetting   *m_pSettingView = NULL;  //主视图右边的操作视图

	Project             *m_pProject     = NULL;  //工程对象

	BOOL m_bInitSplitter = FALSE;

private:
	void SaveAsJpg(CString strPathFile, SwathChannel * pChannel);                           //将指定的通道数据存储为图片
	void SaveAsJpgs(CString strPathFile, SwathChannel * pChannel[32], int channelCount);    //将指定的通道数据存储为图片
	void CreateChannelDC(CDC oDCImage, SwathChannel * pChannel, int iChannelStart, int iChannelWidth, int iPictureWidth);    //使用通道数据，创建一个内存DC

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

};



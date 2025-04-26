
// RadarViewMain.h: RadarViewMain 类的接口
//

#pragma once

#include "Scale.h"
#include "RadarViewAssistant.h"

class RadarViewMain : public CFormView
{
protected: // 仅从序列化创建
	RadarViewMain() noexcept;
	DECLARE_DYNCREATE(RadarViewMain)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEW_MAIN};
#endif

// 特性
public:
	//RadarDoc* GetDocument() const;

// 操作
public:
	//设置工程
	void SetProject(Project* pProject);
	//第一视图打开通道
	void SetChannelViewOne(Swath* pSwath, SwathChannel* pChannel);
	//第二视图打开通道
	void SetChannelViewTwo(Swath* pSwath, SwathChannel* pChannel);

	SwathChannel * GetChannelViewOne();
	SwathChannel * GetChannelViewTwo();

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnInitialUpdate(); // 构造后第一次调用

// 实现
public:
	virtual ~RadarViewMain();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

private:
	Project     * m_pProject     = NULL;    //雷达工程对象
	Swath       * m_pSwath       = NULL;    //测线对象
	SwathChannel* m_pChannel1    = NULL;    //测线通道对象(第一视图显示的通道)
	SwathChannel* m_pChannel2    = NULL;    //测线通道对象(第二视图显示的通道)

	int           m_iHeight      = 0;       //侧视图的高度（探测的深度）
	int           m_iWidth       = 0;       //侧视图的宽度（测线的长度）

	PalSET        m_Palette;                //调色板
	Scale         m_Scale;                  //绘制标尺的对象

	//用户标注的可疑区域
	std::vector<CRect*> m_lstMark;

	CPoint m_PointStart;    //保存鼠标左键按下时的鼠标位置
	CPoint m_PointEnd;      //保存鼠标拖动（即鼠标左键按下并移动）时的位置并不断更新。
	bool m_bMouseDownFlag;  //鼠标左键按下的标志。


private:
	void CreateScale1(CDC& oDC);
	void CreateScale2(CDC& oDC);
	void CreateViewSide(CDC& oDC);
	void CreateViewSide1(CDC& oDC);
	void CreateViewSide2(CDC& oDC);
	void CreateViewTop(CDC& oDC);

	void CreateMarks(CDC &oDC);

	//设置显示数据
	void SetProjectData(Swath* pSwath, SwathChannel* pChannel);

	//更新显示系数
	void UpdateCoef();

	//初始化按键状态和Marks
	void initMarks();

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnDraw(CDC* /*pDC*/);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

/*
#ifndef _DEBUG  // RadarViewMain.cpp 中的调试版本
inline RadarDoc* RadarViewMain::GetDocument() const
   { return reinterpret_cast<RadarDoc*>(m_pDocument); }
#endif
*/
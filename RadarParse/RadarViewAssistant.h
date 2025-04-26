#pragma once


#include "Project.h"
#include "DialogCard.h"
#include "RadarViewSetting.h"
//#include "FSize.h"
//#include "PalSET.h"
#include "Scale.h"

// RadarViewAssistant 对话框
class RadarViewAssistant : public CDialogEx
{
	DECLARE_DYNAMIC(RadarViewAssistant)

public:
	RadarViewAssistant(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~RadarViewAssistant();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEW_ASSISTANT };
#endif

public:
	void SetProject(Project* pProject);
	void SetChannelViewOne(Swath* pSwath, SwathChannel* pChannel);
	void SetChannelViewTwo(Swath* pSwath, SwathChannel* pChannel);
	void SetChannelViewThree(Swath* pSwath, SwathChannel* pChannel);
	SwathChannel *GetChannelViewOne();
	SwathChannel *GetChannelViewTwo();
	SwathChannel *GetChannelViewThree();

	//设置水平滚动
	void SetHScroll(UINT nPos);

private:
	Project     * m_pProject  = NULL;    //工程对象

	Swath       * m_pSwath1   = NULL;    //视图1测线对象
	SwathChannel* m_pChannel1 = NULL;    //视图1测线通道对象

	Swath       * m_pSwath2   = NULL;    //视图2测线对象
	SwathChannel* m_pChannel2 = NULL;    //视图2测线通道对象

	Swath       * m_pSwath3   = NULL;    //视图3测线对象
	SwathChannel* m_pChannel3 = NULL;    //视图3测线通道对象

	int           m_iHeight1  = 0;       //侧视图1的高度（探测的深度）
	int           m_iWidth1   = 0;       //侧视图1的宽度（测线的长度）
	int           m_iHeight2  = 0;       //侧视图2的高度（探测的深度）
	int           m_iWidth2   = 0;       //侧视图2的宽度（测线的长度）
	int           m_iHeight3  = 0;       //侧视图3的高度（探测的深度）
	int           m_iWidth3   = 0;       //侧视图3的宽度（测线的长度）

	PalSET        m_Palette;             //调色板
	Scale         m_Scale;               //绘制标尺的对象

private:
	void CreateScale1(CDC& oDC);
	void CreateScale2(CDC& oDC);
	void CreateScale3(CDC& oDC);
	void CreateViewSide(CDC& oDC);
	void CreateViewSide1(CDC& oDC);
	void CreateViewSide2(CDC& oDC);
	void CreateViewSide3(CDC& oDC);
	void InitScrollSizes();
	void SetScrollSizes();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

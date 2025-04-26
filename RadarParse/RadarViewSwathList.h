
#pragma once

#include "Utils\\PalSET.h"
#include "Utils\\Utils.h"
#include "Utils\\RadarConst.h"

#include "Impluse\\ImpluseTrace16.h"
#include "Impluse\\ImpluseTrace32.h"
#include "Impluse\\ImpluseCor.h"
#include "Impluse\\ImpluseTime.h"
#include "Impluse\\ImpluseChannelHeader.h"
#include "Impluse\\ImpluseChannelBlob.h"
#include "Impluse\\ImpluseChannel.h"
#include "Impluse\\ImpluseSwath.h"

#include "IDS\\IDSChannel.h"
#include "IDS\\IDSChannelBlob.h"
#include "IDS\\IDSChannelHeader.h"
#include "IDS\\IDSSwath.h"
#include "IDS\\IDSSwathFragment.h"
#include "IDS\\IDSTrace16.h"
#include "IDS\\IDSTrace32.h"

#include "Mala\\MalaChannel.h"
#include "Mala\\MalaChannelBlob.h"
#include "Mala\\MalaChannelHeader.h"
#include "Mala\\MalaSwath.h"
#include "Mala\\MalaTime.h"
#include "Mala\\MalaTrace16.h"
#include "Mala\\MalaTrace32.h"

#include "Project.h"

class CFileViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class RadarViewSwathList : public CDockablePane
{
// 构造
public:
	RadarViewSwathList() noexcept;

	void AdjustLayout();
	void OnChangeVisualStyle();

// 特性
protected:
	CTreeCtrl m_wndSwathListView;
	CImageList m_SwathListViewImages;
	CFileViewToolBar m_wndToolBar;

public:
	void SetProject(Project *pProject);

public:
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);

// 实现
public:
	virtual ~RadarViewSwathList();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	afx_msg void OnDisplayMainOne();    //右键菜单事件响应--显示在主视图，第一视图
	afx_msg void OnDisplayMainTwo();    //右键菜单事件响应--显示在主视图，第二视图

	afx_msg void OnDisplayLeftOne(); 	//右键菜单事件响应--显示在左视图，第一视图
	afx_msg void OnDisplayLeftTwo(); 	//右键菜单事件响应--显示在左视图，第二视图
	afx_msg void OnDisplayLeftThree();	//右键菜单事件响应--显示在左视图，第三视图

	afx_msg void OnDisplayRightOne();	//右键菜单事件响应--显示在右视图，第一视图
	afx_msg void OnDisplayRightTwo();	//右键菜单事件响应--显示在右视图，第二视图
	afx_msg void OnDisplayRightThree();	//右键菜单事件响应--显示在右视图，第三视图

	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

private:
	void DisplayChannel(int iType, int iView);
private:
	Project * m_pProject = NULL;
};

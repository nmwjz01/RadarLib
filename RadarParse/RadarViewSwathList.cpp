
#include <io.h>
#include "stdafx.h"

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

#include "Utils\\FSize.h"


#include "RadarFrame.h"
#include "RadarDoc.h"
#include "RadarViewSwathList.h"
#include "Resource.h"
#include "RadarApp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define VIEW_LEFT  1
#define VIEW_MAIN  2
#define VIEW_RIGHT 3

/////////////////////////////////////////////////////////////////////////////
// RadarViewSwathList

RadarViewSwathList::RadarViewSwathList() noexcept
{
}

RadarViewSwathList::~RadarViewSwathList()
{
}

BEGIN_MESSAGE_MAP(RadarViewSwathList, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()

	ON_COMMAND(ID_DISPLAY_IN_ONE, OnDisplayMainOne)
	ON_COMMAND(ID_DISPLAY_IN_TWO, OnDisplayMainTwo)

	ON_COMMAND(ID_DISPLAY_LEFT_ONE, OnDisplayLeftOne)
	ON_COMMAND(ID_DISPLAY_LEFT_TWO, OnDisplayLeftTwo)
	ON_COMMAND(ID_DISPLAY_LEFT_THREE, OnDisplayLeftThree)

	ON_COMMAND(ID_DISPLAY_RIGHT_ONE, OnDisplayRightOne)
	ON_COMMAND(ID_DISPLAY_RIGHT_TWO, OnDisplayRightTwo)
	ON_COMMAND(ID_DISPLAY_RIGHT_THREE, OnDisplayRightThree)

	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(NM_DBLCLK, 4,OnNMDblclk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 消息处理程序

int RadarViewSwathList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建视图: 
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndSwathListView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("未能创建Swath视图\n");
		return -1;      // 未能创建
	}

	// 加载视图图像: 
	m_SwathListViewImages.Create(IDB_SWATH_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndSwathListView.SetImageList(&m_SwathListViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* 已锁定*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由: 
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
	AdjustLayout();

	return 0;
}

void RadarViewSwathList::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void RadarViewSwathList::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndSwathListView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项: 
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != nullptr)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	//弹出右键菜单
	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void RadarViewSwathList::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndSwathListView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + 1, rectClient.Width() - 2, rectClient.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void RadarViewSwathList::SetProject(Project *pProject)
{
	if (NULL == pProject)
		return;

	if (m_pProject != pProject)
		m_pProject = pProject;

	//删除原有的节点
	m_wndSwathListView.DeleteAllItems();

	HTREEITEM hRoot = m_wndSwathListView.InsertItem(_T("Swath列表"), 0, 0);
	m_wndSwathListView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	//获取所有SwathName
	std::vector<char *> lstSwath;
	int iResult = pProject->getAllSwathName(lstSwath);
	if (0 != iResult)
	{
		lstSwath.clear();
		return;
	}
	for ( int i=0; i<(int)lstSwath.size(); i++ )
	{
		//获取，并且在列表中插入SwathName
		CString strSwathName = (CString)lstSwath[i];
		HTREEITEM hSwath = m_wndSwathListView.InsertItem(strSwathName, 0, 0, hRoot);

		//获取Swath详细信息（channel）
		Swath *pSwath = pProject->getSwath(lstSwath[i]);
		int iCount = 0;
		iResult = pSwath->getChannelCount(iCount);
		if (0 != iResult)
			continue;
		//循环将通道插入Swath项目下面
		for (int j = 1; j <= iCount; j++)
		{
			char szChannel[32] = { 0 };
			sprintf(szChannel, "通道%d", j);
			CString strChannel = ( CString )szChannel;
			HTREEITEM hChannel = m_wndSwathListView.InsertItem(strChannel, 1, 2, hSwath);

			//附带通道数据
			SwathChannel * pChannel = pSwath->getChannel(j);
			m_wndSwathListView.SetItemData(hChannel, (DWORD_PTR)pChannel);
		}

		m_wndSwathListView.Expand(hSwath, TVE_EXPAND);
	}

	m_wndSwathListView.Expand(hRoot, TVE_EXPAND);

	//清除临时数据
	lstSwath.clear();
}

void RadarViewSwathList::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndSwathListView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void RadarViewSwathList::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndSwathListView.SetFocus();
}

void RadarViewSwathList::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* 锁定*/);

	m_SwathListViewImages.DeleteImageList();

	//UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_SWATH_VIEW_24 : IDB_SWATH_VIEW;
	UINT uiBmpId = IDB_SWATH_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("无法加载位图: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_SwathListViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_SwathListViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndSwathListView.SetImageList(&m_SwathListViewImages, TVSIL_NORMAL);
}


//双击事件
void RadarViewSwathList::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	DisplayChannel(VIEW_MAIN , 1);
}


//右键菜单事件响应--显示在主视图，第一视图
void RadarViewSwathList::OnDisplayMainOne()
{
	DisplayChannel(VIEW_MAIN, 1);
}
//右键菜单事件响应--显示在主视图，第二视图
void RadarViewSwathList::OnDisplayMainTwo()
{
	DisplayChannel(VIEW_MAIN, 2);
}


//右键菜单事件响应--显示在左视图，第一视图
void RadarViewSwathList::OnDisplayLeftOne()
{
	DisplayChannel(VIEW_LEFT, 1);
}
//右键菜单事件响应--显示在左视图，第二视图
void RadarViewSwathList::OnDisplayLeftTwo()
{
	DisplayChannel(VIEW_LEFT, 2);
}
//右键菜单事件响应--显示在左视图，第三视图
void RadarViewSwathList::OnDisplayLeftThree()
{
	DisplayChannel(VIEW_LEFT, 3);
}

//右键菜单事件响应--显示在右视图，第一视图
void RadarViewSwathList::OnDisplayRightOne()
{
	DisplayChannel(VIEW_RIGHT, 1);
}
//右键菜单事件响应--显示在右视图，第二视图
void RadarViewSwathList::OnDisplayRightTwo()
{
	DisplayChannel(VIEW_RIGHT, 2);
}
//右键菜单事件响应--显示在右视图，第三视图
void RadarViewSwathList::OnDisplayRightThree()
{
	DisplayChannel(VIEW_RIGHT, 3);
}


void RadarViewSwathList::DisplayChannel( int iType , int iView)
{
	//获得TreeView当前选中ID
	HTREEITEM hSelectedItem = m_wndSwathListView.GetSelectedItem();
	if (NULL == hSelectedItem)
	{
		MessageBox(_T("请打开工程"));
		return;
	}

	//获取当前选中节点的名称
	CString strChannelNum = m_wndSwathListView.GetItemText(hSelectedItem);
	//获得当前选中节点的附件信息（通道对象）
	SwathChannel* pChannel = (SwathChannel*)m_wndSwathListView.GetItemData(hSelectedItem);
	//没有选中通道，可能点击到了Swath或根节点
	if (NULL == pChannel)
	{
		MessageBox(_T( "请选择通道" ));
		return;
	}

	//取得Swath信息
	HTREEITEM hSawthName = m_wndSwathListView.GetParentItem(hSelectedItem);
	CString strSawthName = m_wndSwathListView.GetItemText(hSawthName);

	//通知状态栏
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->setStatusBar(strSawthName, strChannelNum);

	//取得测线对象
	USES_CONVERSION;
	char* pSwathName = T2A(strSawthName.GetBuffer(0));    //编码转换
	Swath* pSwath = m_pProject->getSwath(pSwathName);

	//显示到指定的View对象
	switch (iType)
	{
	case VIEW_MAIN:
		if (1 == iView)
			pFrame->SetChannelViewMainOne(pSwath, pChannel);
		else
			pFrame->SetChannelViewMainTwo(pSwath, pChannel);
		break;
	case VIEW_LEFT:
		if (1 == iView)
			pFrame->SetChannelViewLeftOne(pSwath, pChannel);
		else if(2 == iView)
			pFrame->SetChannelViewLeftTwo(pSwath, pChannel);
		else
			pFrame->SetChannelViewLeftThree(pSwath, pChannel);
		break;
	case VIEW_RIGHT:
		if (1 == iView)
			pFrame->SetChannelViewRightOne(pSwath, pChannel);
		else if (2 == iView)
			pFrame->SetChannelViewRightTwo(pSwath, pChannel);
		else
			pFrame->SetChannelViewRightThree(pSwath, pChannel);
		break;
	}
}

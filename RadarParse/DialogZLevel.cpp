// DialogZLevel.cpp: 实现文件
//

#include <io.h>
#include "stdafx.h"
#include "afxdialogex.h"

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

#include "DialogZLevel.h"
#include "Resource.h"
#include "RadarFrame.h"
// DialogZLevel 对话框

IMPLEMENT_DYNAMIC(DialogZLevel, CDialogEx)

DialogZLevel::DialogZLevel(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ZLEVEL, pParent)
{

}

DialogZLevel::~DialogZLevel()
{
}

void DialogZLevel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_ZLEVEL_MAIN_ONE, m_oZLevelMainOne);
	DDX_Control(pDX, IDC_SLIDER_ZLEVEL_MAIN_TWO, m_oZLevelMainTwo);
	DDX_Control(pDX, IDC_SLIDER_ZLEVEL_LEFT_ONE, m_oZLevelLeftOne);
	DDX_Control(pDX, IDC_SLIDER_ZLEVEL_LEFT_TWO, m_oZLevelLeftTwo);
	DDX_Control(pDX, IDC_SLIDER_ZLEVEL_LEFT_THREE, m_oZLevelLeftThree);
	DDX_Control(pDX, IDC_SLIDER_ZLEVEL_RIGHT_ONE, m_oZLevelRightOne);
	DDX_Control(pDX, IDC_SLIDER_ZLEVEL_RIGHT_TWO, m_oZLevelRightTwo);
	DDX_Control(pDX, IDC_SLIDER_ZLEVEL_RIGHT_THREE, m_oZLevelRightThree);
}


BEGIN_MESSAGE_MAP(DialogZLevel, CDialogEx)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_ZLEVEL_SAVE, &DialogZLevel::OnBnClickedZlevelSave)
END_MESSAGE_MAP()

//对话框初始化
BOOL DialogZLevel::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//获取各个Channel的初始值
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();

	//主视图默认值获取
	RadarViewMain *pViewMain = pFrame->GetViewMain();
	if (NULL != pViewMain)
	{
		SwathChannel *pChannel = pViewMain->GetChannelViewOne();
		if (NULL != pChannel)
		{
			int iZLevel = pChannel->getChannelHeader()->getZeroLevel();
			int iZLevelMax = pChannel->getChannelHeader()->getSample();
			//Z-Level
			m_oZLevelMainOne.SetRange(1, iZLevelMax);
			m_oZLevelMainOne.SetPos(iZLevel);

			CString strOut = _T("");
			strOut.Format(_T("第一视图:%d"), iZLevel);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_MAIN_ONE);
			pStatic->SetWindowText(strOut);
		}

		pChannel = pViewMain->GetChannelViewTwo();
		if (NULL != pChannel)
		{
			int iZLevel = pChannel->getChannelHeader()->getZeroLevel();
			int iZLevelMax = pChannel->getChannelHeader()->getSample();
			//Z-Level
			m_oZLevelMainTwo.SetRange(1, iZLevelMax);
			m_oZLevelMainTwo.SetPos(iZLevel);

			CString strOut = _T("");
			strOut.Format(_T("第二视图:%d"), iZLevel);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_MAIN_TWO);
			pStatic->SetWindowText(strOut);
		}
	}

	//左辅图默认值获取
	RadarViewAssistant *pViewLeft = pFrame->GetViewLeft();
	if (NULL != pViewLeft)
	{
		SwathChannel *pChannel = pViewLeft->GetChannelViewOne();
		if (NULL != pChannel)
		{
			int iZLevel = pChannel->getChannelHeader()->getZeroLevel();
			int iZLevelMax = pChannel->getChannelHeader()->getSample();
			//Z-Level
			m_oZLevelLeftOne.SetRange(1, iZLevelMax);
			m_oZLevelLeftOne.SetPos(iZLevel);

			CString strOut = _T("");
			strOut.Format(_T("第一视图:%d"), iZLevel);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_LEFT_ONE);
			pStatic->SetWindowText(strOut);
		}

		pChannel = pViewLeft->GetChannelViewTwo();
		if (NULL != pChannel)
		{
			int iZLevel = pChannel->getChannelHeader()->getZeroLevel();
			int iZLevelMax = pChannel->getChannelHeader()->getSample();
			//Z-Level
			m_oZLevelLeftTwo.SetRange(1, iZLevelMax);
			m_oZLevelLeftTwo.SetPos(iZLevel);

			CString strOut = _T("");
			strOut.Format(_T("第二视图:%d"), iZLevel);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_LEFT_TWO);
			pStatic->SetWindowText(strOut);
		}

		pChannel = pViewLeft->GetChannelViewThree();
		if (NULL != pChannel)
		{
			int iZLevel = pChannel->getChannelHeader()->getZeroLevel();
			int iZLevelMax = pChannel->getChannelHeader()->getSample();
			//Z-Level
			m_oZLevelLeftThree.SetRange(1, iZLevelMax);
			m_oZLevelLeftThree.SetPos(iZLevel);

			CString strOut = _T("");
			strOut.Format(_T("第三视图:%d"), iZLevel);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_LEFT_THREE);
			pStatic->SetWindowText(strOut);
		}
	}

	//右辅图默认值获取
	RadarViewAssistant *pViewRight = pFrame->GetViewRight();
	if (NULL != pViewRight)
	{
		SwathChannel *pChannel = pViewRight->GetChannelViewOne();
		if (NULL != pChannel)
		{
			int iZLevel = pChannel->getChannelHeader()->getZeroLevel();
			int iZLevelMax = pChannel->getChannelHeader()->getSample();
			//Z-Level
			m_oZLevelRightOne.SetRange(1, iZLevelMax);
			m_oZLevelRightOne.SetPos(iZLevel);

			CString strOut = _T("");
			strOut.Format(_T("第一视图:%d"), iZLevel);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_RIGHT_ONE);
			pStatic->SetWindowText(strOut);
		}

		pChannel = pViewRight->GetChannelViewTwo();
		if (NULL != pChannel)
		{
			int iZLevel = pChannel->getChannelHeader()->getZeroLevel();
			int iZLevelMax = pChannel->getChannelHeader()->getSample();
			//Z-Level
			m_oZLevelRightTwo.SetRange(1, iZLevelMax);
			m_oZLevelRightTwo.SetPos(iZLevel);

			CString strOut = _T("");
			strOut.Format(_T("第二视图:%d"), iZLevel);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_RIGHT_TWO);
			pStatic->SetWindowText(strOut);
		}

		pChannel = pViewRight->GetChannelViewThree();
		if (NULL != pChannel)
		{
			int iZLevel = pChannel->getChannelHeader()->getZeroLevel();
			int iZLevelMax = pChannel->getChannelHeader()->getSample();
			//Z-Level
			m_oZLevelRightThree.SetRange(1, iZLevelMax);
			m_oZLevelRightThree.SetPos(iZLevel);

			CString strOut = _T("");
			strOut.Format(_T("第三视图:%d"), iZLevel);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_RIGHT_THREE);
			pStatic->SetWindowText(strOut);
		}
	}

	return TRUE;
}


// DialogZLevel 消息处理程序
//上下滚动事件处理
void DialogZLevel::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (NULL == pScrollBar)
		return;

	switch (pScrollBar->GetDlgCtrlID())
	{
	case(IDC_SLIDER_ZLEVEL_MAIN_ONE):
	{
		//刷新主数据View
		UpdateMainOne();

		break;
	}
	case(IDC_SLIDER_ZLEVEL_MAIN_TWO):
	{
		//刷新主数据View
		UpdateMainTwo();

		break;
	}

	case(IDC_SLIDER_ZLEVEL_LEFT_ONE):
	{
		//刷新主数据View
		UpdateLeftOne();

		break;
	}
	case(IDC_SLIDER_ZLEVEL_LEFT_TWO):
	{
		//刷新主数据View
		UpdateLeftTwo();

		break;
	}
	case(IDC_SLIDER_ZLEVEL_LEFT_THREE):
	{
		//刷新主数据View
		UpdateLeftThree();

		break;
	}

	case(IDC_SLIDER_ZLEVEL_RIGHT_ONE):
	{
		//刷新主数据View
		UpdateRightOne();

		break;
	}
	case(IDC_SLIDER_ZLEVEL_RIGHT_TWO):
	{
		//刷新主数据View
		UpdateRightTwo();

		break;
	}
	case(IDC_SLIDER_ZLEVEL_RIGHT_THREE):
	{
		//刷新主数据View
		UpdateRightThree();

		break;
	}
	}

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

void DialogZLevel::OnBnClickedZlevelSave()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("暂不支持"));
}

void DialogZLevel::UpdateMainOne()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到主视图
	RadarViewMain *pView = pFrame->GetViewMain();
	if (NULL == pView)
		return;

	//取得通道Channel
	SwathChannel *pChannel = pView->GetChannelViewOne();
	if (NULL == pChannel)
		return;

	//Z-Level
	int iZLevel = m_oZLevelMainOne.GetPos();

	//界面更新显示
	CString strOut = _T("");
	strOut.Format(_T("第一视图:%d"), iZLevel);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_MAIN_ONE);
	pStatic->SetWindowText(strOut);

	pChannel->getChannelHeader()->setZeroLevel(iZLevel);

	pFrame->UpdateAllView(VIEW_TYPE_MAIN);
}
void DialogZLevel::UpdateMainTwo()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到主视图
	RadarViewMain *pView = pFrame->GetViewMain();
	if (NULL == pView)
		return;

	//取得通道Channel
	SwathChannel *pChannel = pView->GetChannelViewTwo();
	if (NULL == pChannel)
		return;

	//Z-Level
	int iZLevel = m_oZLevelMainTwo.GetPos();

	//界面更新显示
	CString strOut = _T("");
	strOut.Format(_T("第二视图:%d"), iZLevel);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_MAIN_TWO);
	pStatic->SetWindowText(strOut);

	pChannel->getChannelHeader()->setZeroLevel(iZLevel);

	pFrame->UpdateAllView(VIEW_TYPE_MAIN);
}

void DialogZLevel::UpdateLeftOne()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到主视图
	RadarViewAssistant *pView = pFrame->GetViewLeft();
	if (NULL == pView)
		return;

	//取得通道Channel
	SwathChannel *pChannel = pView->GetChannelViewOne();
	if (NULL == pChannel)
		return;

	//Z-Level
	int iZLevel = m_oZLevelLeftOne.GetPos();

	//界面更新显示
	CString strOut = _T("");
	strOut.Format(_T("第一视图:%d"), iZLevel);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_LEFT_ONE);
	pStatic->SetWindowText(strOut);

	pChannel->getChannelHeader()->setZeroLevel(iZLevel);

	pFrame->UpdateAllView(VIEW_TYPE_LEFT);
	pFrame->UpdateAllView(VIEW_TYPE_MAIN);
}
void DialogZLevel::UpdateLeftTwo()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到主视图
	RadarViewAssistant *pView = pFrame->GetViewLeft();
	if (NULL == pView)
		return;

	//取得通道Channel
	SwathChannel *pChannel = pView->GetChannelViewTwo();
	if (NULL == pChannel)
		return;

	//Z-Level
	int iZLevel = m_oZLevelLeftTwo.GetPos();

	//界面更新显示
	CString strOut = _T("");
	strOut.Format(_T("第二视图:%d"), iZLevel);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_LEFT_TWO);
	pStatic->SetWindowText(strOut);

	pChannel->getChannelHeader()->setZeroLevel(iZLevel);

	pFrame->UpdateAllView(VIEW_TYPE_LEFT);
	pFrame->UpdateAllView(VIEW_TYPE_MAIN);
}
void DialogZLevel::UpdateLeftThree()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到主视图
	RadarViewAssistant *pView = pFrame->GetViewLeft();
	if (NULL == pView)
		return;

	//取得通道Channel
	SwathChannel *pChannel = pView->GetChannelViewThree();
	if (NULL == pChannel)
		return;

	//Z-Level
	int iZLevel = m_oZLevelLeftThree.GetPos();

	//界面更新显示
	CString strOut = _T("");
	strOut.Format(_T("第三视图:%d"), iZLevel);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_LEFT_THREE);
	pStatic->SetWindowText(strOut);

	pChannel->getChannelHeader()->setZeroLevel(iZLevel);

	pFrame->UpdateAllView(VIEW_TYPE_LEFT);
	pFrame->UpdateAllView(VIEW_TYPE_MAIN);
}

void DialogZLevel::UpdateRightOne()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到主视图
	RadarViewAssistant *pView = pFrame->GetViewRight();
	if (NULL == pView)
		return;

	//取得通道Channel
	SwathChannel *pChannel = pView->GetChannelViewOne();
	if (NULL == pChannel)
		return;

	//Z-Level
	int iZLevel = m_oZLevelRightOne.GetPos();

	//界面更新显示
	CString strOut = _T("");
	strOut.Format(_T("第一视图:%d"), iZLevel);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_RIGHT_ONE);
	pStatic->SetWindowText(strOut);

	pChannel->getChannelHeader()->setZeroLevel(iZLevel);

	pFrame->UpdateAllView(VIEW_TYPE_RIGHT);
	pFrame->UpdateAllView(VIEW_TYPE_MAIN);
}
void DialogZLevel::UpdateRightTwo()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到主视图
	RadarViewAssistant *pView = pFrame->GetViewRight();
	if (NULL == pView)
		return;

	//取得通道Channel
	SwathChannel *pChannel = pView->GetChannelViewTwo();
	if (NULL == pChannel)
		return;

	//Z-Level
	int iZLevel = m_oZLevelRightTwo.GetPos();

	//界面更新显示
	CString strOut = _T("");
	strOut.Format(_T("第二视图:%d"), iZLevel);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_RIGHT_TWO);
	pStatic->SetWindowText(strOut);

	pChannel->getChannelHeader()->setZeroLevel(iZLevel);

	pFrame->UpdateAllView(VIEW_TYPE_RIGHT);
	pFrame->UpdateAllView(VIEW_TYPE_MAIN);
}
void DialogZLevel::UpdateRightThree()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到主视图
	RadarViewAssistant *pView = pFrame->GetViewRight();
	if (NULL == pView)
		return;

	//取得通道Channel
	SwathChannel *pChannel = pView->GetChannelViewThree();
	if (NULL == pChannel)
		return;

	//Z-Level
	int iZLevel = m_oZLevelRightThree.GetPos();

	//界面更新显示
	CString strOut = _T("");
	strOut.Format(_T("第三视图:%d"), iZLevel);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_ZLEVEL_RIGHT_THREE);
	pStatic->SetWindowText(strOut);

	pChannel->getChannelHeader()->setZeroLevel(iZLevel);

	pFrame->UpdateAllView(VIEW_TYPE_RIGHT);
	pFrame->UpdateAllView(VIEW_TYPE_MAIN);
}
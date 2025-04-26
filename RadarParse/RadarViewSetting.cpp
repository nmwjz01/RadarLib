
#include "framework.h"
#include <io.h>
#include <atlimage.h>

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

#include "RadarViewSetting.h"
#include "RadarFrame.h"


int RadarViewSetting::m_iContrast = 15;       //对比度
int RadarViewSetting::m_iTimegain = 1;        //时间增益
int RadarViewSetting::m_iDeep     = 150;      //深度
BOOL RadarViewSetting::m_Auto     = FALSE;    //是否自动找平

// RadarViewSetting
IMPLEMENT_DYNCREATE(RadarViewSetting, CFormView)
RadarViewSetting::RadarViewSetting():CFormView(IDD_VIEW_SETTING)
{
}
RadarViewSetting::~RadarViewSetting()
{
}

BEGIN_MESSAGE_MAP(RadarViewSetting, CFormView)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_SETTING_OPEN_LEFT, &RadarViewSetting::OnBnClickedSettingOpenLeft)
	ON_BN_CLICKED(IDC_SETTING_OPEN_RIGHT, &RadarViewSetting::OnBnClickedSettingOpenRight)
	ON_BN_CLICKED(IDC_SLIDER_DEEP_AUTO, &RadarViewSetting::OnBnClickedSliderDeepAuto)
END_MESSAGE_MAP()

void RadarViewSetting::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_CONTRAST, m_oContrast );
	DDX_Control(pDX, IDC_SLIDER_TIMEGAIN, m_oTimeGain );
	DDX_Control(pDX, IDC_SLIDER_DEEP    , m_oDeep     );
}

//对话框初始化
void RadarViewSetting::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	//对比度设置
	m_oContrast.SetRange(5, 500);
	m_oContrast.SetPos( 15 );   //初始化默认为15

	//增益设置
	m_oTimeGain.SetRange(0, 500);
	m_oTimeGain.SetPos( 0 );   //初始化默认为1

	//水平层距设置
	m_oDeep.SetRange(5, 500);
	m_oDeep.SetPos(150);
}
//左右滚动事件处理
void RadarViewSetting::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (NULL == pScrollBar)
		return;

	switch (pScrollBar->GetDlgCtrlID())
	{
		case(IDC_SLIDER_CONTRAST):
		{
			RadarViewSetting::m_iContrast = m_oContrast.GetPos();
			CString strOut = _T("");
			strOut.Format(_T("对比度:%d"), RadarViewSetting::m_iContrast);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_CONTRAST);
			pStatic->SetWindowText(strOut);

			//刷新主数据View
			RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
			pFrame->UpdateAllView(VIEW_TYPE_ALL);

			break;
		}
		case(IDC_SLIDER_TIMEGAIN):
		{
			RadarViewSetting::m_iTimegain = m_oTimeGain.GetPos();
			CString strOut = _T("");
			strOut.Format(_T("增益:%d"), RadarViewSetting::m_iTimegain);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_TIMEGAIN);
			pStatic->SetWindowText(strOut);

			//刷新主数据View
			RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
			pFrame->UpdateAllView(VIEW_TYPE_ALL);

			break;
		}
	}

	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}
//上下滚动事件处理
void RadarViewSetting::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (NULL == pScrollBar)
		return;

	switch (pScrollBar->GetDlgCtrlID())
	{
	case(IDC_SLIDER_DEEP):
	{
		RadarViewSetting::m_iDeep = m_oDeep.GetPos();
		CString strOut = _T("");
		strOut.Format(_T("俯视图深度:%d(cm)"), RadarViewSetting::m_iDeep);
		CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_DEEP);
		pStatic->SetWindowText(strOut);

		//刷新主数据View
		RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
		pFrame->UpdateAllView(VIEW_TYPE_MAIN);

		break;
	}
	}

	CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
}

//打开左视图
void RadarViewSetting::OnBnClickedSettingOpenLeft()
{
	//打开左视图
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->OpenViewLeft();
}
//打开右视图
void RadarViewSetting::OnBnClickedSettingOpenRight()
{
	//打开右视图
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->OpenViewRight();
}


void RadarViewSetting::OnBnClickedSliderDeepAuto()
{
	//
	CButton *btn = (CButton *)this->GetDlgItem(IDC_SLIDER_DEEP_AUTO);
	int state = btn->GetCheck();
	if (BST_CHECKED == state)
	{
		//m_oDeep.ShowWindow(SW_HIDE);
		m_oDeep.EnableWindow(FALSE);
		RadarViewSetting::m_Auto = TRUE;
	}
	else
	{
		//m_oDeep.ShowWindow(SW_SHOW);
		m_oDeep.EnableWindow(TRUE);
		RadarViewSetting::m_Auto = FALSE;
	}
}

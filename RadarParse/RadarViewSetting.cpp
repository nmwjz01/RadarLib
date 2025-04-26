
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


int RadarViewSetting::m_iContrast = 15;       //�Աȶ�
int RadarViewSetting::m_iTimegain = 1;        //ʱ������
int RadarViewSetting::m_iDeep     = 150;      //���
BOOL RadarViewSetting::m_Auto     = FALSE;    //�Ƿ��Զ���ƽ

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

//�Ի����ʼ��
void RadarViewSetting::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	//�Աȶ�����
	m_oContrast.SetRange(5, 500);
	m_oContrast.SetPos( 15 );   //��ʼ��Ĭ��Ϊ15

	//��������
	m_oTimeGain.SetRange(0, 500);
	m_oTimeGain.SetPos( 0 );   //��ʼ��Ĭ��Ϊ1

	//ˮƽ�������
	m_oDeep.SetRange(5, 500);
	m_oDeep.SetPos(150);
}
//���ҹ����¼�����
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
			strOut.Format(_T("�Աȶ�:%d"), RadarViewSetting::m_iContrast);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_CONTRAST);
			pStatic->SetWindowText(strOut);

			//ˢ��������View
			RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
			pFrame->UpdateAllView(VIEW_TYPE_ALL);

			break;
		}
		case(IDC_SLIDER_TIMEGAIN):
		{
			RadarViewSetting::m_iTimegain = m_oTimeGain.GetPos();
			CString strOut = _T("");
			strOut.Format(_T("����:%d"), RadarViewSetting::m_iTimegain);
			CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_TIMEGAIN);
			pStatic->SetWindowText(strOut);

			//ˢ��������View
			RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
			pFrame->UpdateAllView(VIEW_TYPE_ALL);

			break;
		}
	}

	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}
//���¹����¼�����
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
		strOut.Format(_T("����ͼ���:%d(cm)"), RadarViewSetting::m_iDeep);
		CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_DEEP);
		pStatic->SetWindowText(strOut);

		//ˢ��������View
		RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
		pFrame->UpdateAllView(VIEW_TYPE_MAIN);

		break;
	}
	}

	CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
}

//������ͼ
void RadarViewSetting::OnBnClickedSettingOpenLeft()
{
	//������ͼ
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->OpenViewLeft();
}
//������ͼ
void RadarViewSetting::OnBnClickedSettingOpenRight()
{
	//������ͼ
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

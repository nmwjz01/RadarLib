// RadarViewAssistant.cpp: 实现文件
//

#include "stdafx.h"
#include "afxdialogex.h"
#include <io.h>

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

#include "resource.h"
#include "RadarViewAssistant.h"


#define SPACE_LEFT     50           //左边垂直标尺绘制时，距离左边的距离
#define SPACE_TOP      50           //上边水平标尺绘制时，距离顶部的距离
#define SPACE_TOP_SIDE 50           //侧视图和侧视图之间的距离

// RadarViewAssistant 对话框
IMPLEMENT_DYNAMIC(RadarViewAssistant, CDialogEx)

RadarViewAssistant::RadarViewAssistant(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VIEW_ASSISTANT, pParent)
{

}
RadarViewAssistant::~RadarViewAssistant()
{
}

void RadarViewAssistant::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}
BOOL RadarViewAssistant::OnEraseBkgnd(CDC* pDC)
{
	if (NULL == m_pProject)
		return CDialogEx::OnEraseBkgnd(pDC);

	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//定义内存DC上的BMP
	CBitmap oMemBitmap;//定义一个位图对象
	oMemBitmap.CreateCompatibleBitmap(pDC, oRect.Width(), oRect.Height());

	//定义内存DC
	CDC oMemDC;
	oMemDC.CreateCompatibleDC(NULL);
	CBitmap* pOldBit = oMemDC.SelectObject(&oMemBitmap);
	//先使用背景颜色填充整个View
	oMemDC.FillSolidRect(0, 0, oRect.Width(), oRect.Height(), pDC->GetBkColor());

	//在内存DC上绘制
	CreateViewSide(oMemDC);

	pDC->SetStretchBltMode(COLORONCOLOR);
	pDC->SetStretchBltMode(HALFTONE);
	pDC->SetBrushOrg(0, 0);
	//将内存DC copy到主DC
	pDC->BitBlt(0, 0, oRect.Width(), oRect.Height(), &oMemDC, 0, 0, SRCCOPY);

	//int iiHeigh = oRect.Height();
	//int iiiHeigh = SPACE_TOP + m_iHeight1 + SPACE_TOP_SIDE + m_iHeight2 + SPACE_TOP_SIDE + m_iHeight3;
	//pDC->StretchBlt(0, 0, oRect.Width(), oRect.Height(), &oMemDC, 0, 0, oRect.Width(), SPACE_TOP + m_iHeight1 + SPACE_TOP_SIDE + m_iHeight2 + SPACE_TOP_SIDE + m_iHeight3 , SRCCOPY);
	oMemBitmap.DeleteObject();
	oMemDC.DeleteDC();

	//return CDialogEx::OnEraseBkgnd(pDC);
	if (NULL == m_pChannel1)
		return CDialogEx::OnEraseBkgnd(pDC);
	else
		return TRUE;
}

BEGIN_MESSAGE_MAP(RadarViewAssistant, CDialogEx)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// RadarViewAssistant 消息处理程序

void RadarViewAssistant::InitScrollSizes()
{
	//计算宽度和高度
	int iWidth = max(max(m_iWidth1, m_iWidth2), m_iWidth3);
	int iHeight = SPACE_TOP + m_iHeight1 + SPACE_TOP_SIDE + m_iHeight2 + SPACE_TOP_SIDE + m_iHeight3;

	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//设置垂直滚动条
	SCROLLINFO oVinfo;
	oVinfo.cbSize    = sizeof(oVinfo);
	oVinfo.fMask     = SIF_ALL;
	oVinfo.nPage     = oRect.Height();    //页尺寸，用来确定比例滚动框的大小，一般设置为窗口在宽或高，分别对应用于横向滚动条和竖向滚动条
	oVinfo.nMax      = iHeight;           //滚动条所能滚动的最大值
	oVinfo.nMin      = 0;                 //滚动条所能滚动的最小值
	oVinfo.nTrackPos = 0;                 //页尺寸，用来确定比例滚动框的大小，一般设置为窗口在宽或高，分别对应用于横向滚动条和竖向滚动条
	oVinfo.nPos      = 0;
	SetScrollInfo(SB_VERT, &oVinfo);      //即使上述步骤一不做，使用此条语句也可以显示滚动条

	//水平滚动条
	SCROLLINFO oHinfo;
	oHinfo.cbSize    = sizeof(oHinfo);
	oHinfo.fMask     = SIF_ALL;
	oHinfo.nPage     = oRect.Width();     //页尺寸，用来确定比例滚动框的大小，一般设置为窗口在宽或高，分别对应用于横向滚动条和竖向滚动条
	oHinfo.nMax      = iWidth + 2 * SPACE_LEFT;   //滚动条所能滚动的最大值
	oHinfo.nMin      = 0;                 //滚动条所能滚动的最小值
	oHinfo.nTrackPos = 0;                 //页尺寸，用来确定比例滚动框的大小，一般设置为窗口在宽或高，分别对应用于横向滚动条和竖向滚动条
	oHinfo.nPos      = 0;
	SetScrollInfo(SB_HORZ, &oHinfo);      //即使上述步骤一不做，使用此条语句也可以显示滚动条
}
void RadarViewAssistant::SetScrollSizes()
{
	//计算宽度和高度
	int iWidth = max(max(m_iWidth1, m_iWidth2), m_iWidth3);
	int iHeight = SPACE_TOP + m_iHeight1 + SPACE_TOP_SIDE + m_iHeight2 + SPACE_TOP_SIDE + m_iHeight3;

	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//设置垂直滚动条
	SCROLLINFO oVinfo;
	GetScrollInfo(SB_VERT, &oVinfo, SIF_ALL);
	oVinfo.cbSize    = sizeof(oVinfo);
	oVinfo.fMask     = SIF_ALL;
	oVinfo.nPage     = oRect.Height();    //页尺寸，用来确定比例滚动框的大小，一般设置为窗口在宽或高，分别对应用于横向滚动条和竖向滚动条
	oVinfo.nMax      = iHeight;           //滚动条所能滚动的最大值
	oVinfo.nMin      = 0;                 //滚动条所能滚动的最小值
	SetScrollInfo(SB_VERT, &oVinfo);      //即使上述步骤一不做，使用此条语句也可以显示滚动条

	//水平滚动条
	SCROLLINFO oHinfo;
	GetScrollInfo(SB_HORZ, &oHinfo, SIF_ALL);
	oHinfo.cbSize    = sizeof(oHinfo);
	oHinfo.fMask     = SIF_ALL;
	oHinfo.nPage     = oRect.Width();     //页尺寸，用来确定比例滚动框的大小，一般设置为窗口在宽或高，分别对应用于横向滚动条和竖向滚动条
	oHinfo.nMax      = iWidth + 2 * SPACE_LEFT;   //滚动条所能滚动的最大值
	oHinfo.nMin      = 0;                 //滚动条所能滚动的最小值
	SetScrollInfo(SB_HORZ, &oHinfo);      //即使上述步骤一不做，使用此条语句也可以显示滚动条
}
void RadarViewAssistant::SetProject(Project* pProject)
{
	if (m_pProject == pProject)
		return;

	m_pProject = pProject;
}

void RadarViewAssistant::SetChannelViewOne(Swath* pSwath, SwathChannel* pChannel)
{
	if (NULL == pSwath)
		return;
	if (NULL == pChannel)
		return;

	//如果已经显示，则不再处理
	if (m_pChannel1 == pChannel)
		return;

	//加载Blob数据
	pChannel->getChannelBlob()->loadTrace();

	//记录测线
	m_pSwath1   = pSwath;
	//记录通道对象
	m_pChannel1 = pChannel;

	//============计算图像的宽高==开始============//
	//取得图像的高和宽，用于设置滚动条
	m_iHeight1 = pChannel->getChannelHeader()->getSample();
	//获取最小的trace数，作为数据图像区域的完整宽
	m_iWidth1 = pChannel->getChannelHeader()->getTraceCount();
	//============计算图像的宽高==完成============//

	//设置整个屏幕的滚动条
	InitScrollSizes();

	//更新界面显示
	Invalidate(TRUE);
}
void RadarViewAssistant::SetChannelViewTwo(Swath* pSwath, SwathChannel* pChannel)
{
	if (NULL == pSwath)
		return;
	if (NULL == pChannel)
		return;

	//如果已经显示，则不再处理
	if (m_pChannel2 == pChannel)
		return;

	//加载Blob数据
	pChannel->getChannelBlob()->loadTrace();

	//记录测线
	m_pSwath2   = pSwath;
	//记录通道对象
	m_pChannel2 = pChannel;

	//============计算图像的宽高==开始============//
	//取得图像的高和宽，用于设置滚动条
	m_iHeight2 = pChannel->getChannelHeader()->getSample();
	//获取最小的trace数，作为数据图像区域的完整宽
	m_iWidth2  = pChannel->getChannelHeader()->getTraceCount();
	//============计算图像的宽高==完成============//

	//设置整个屏幕的滚动条
	InitScrollSizes();

	//更新界面显示
	Invalidate(TRUE);
}
void RadarViewAssistant::SetChannelViewThree(Swath* pSwath, SwathChannel* pChannel)
{
	if (NULL == pSwath)
		return;
	if (NULL == pChannel)
		return;

	//如果已经显示，则不再处理
	if (m_pChannel2 == pChannel)
		return;

	//加载Blob数据
	pChannel->getChannelBlob()->loadTrace();

	//记录测线
	m_pSwath3   = pSwath;
	//记录通道对象
	m_pChannel3 = pChannel;

	//============计算图像的宽高==开始============//
	//取得图像的高和宽，用于设置滚动条
	m_iHeight3 = pChannel->getChannelHeader()->getSample();
	//获取最小的trace数，作为数据图像区域的完整宽
	m_iWidth3  = pChannel->getChannelHeader()->getTraceCount();
	//============计算图像的宽高==完成============//

	//设置整个屏幕的滚动条
	InitScrollSizes();

	//更新界面显示
	Invalidate(TRUE);
}

SwathChannel *RadarViewAssistant::GetChannelViewOne()
{
	return m_pChannel1;    //视图1测线通道对象
}
SwathChannel *RadarViewAssistant::GetChannelViewTwo()
{
	return m_pChannel2;    //视图2测线通道对象
}
SwathChannel *RadarViewAssistant::GetChannelViewThree()
{
	return m_pChannel3;    //视图3测线通道对象
}

//窗口大小变化时触发
void RadarViewAssistant::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	//窗口变化时需要重新设置滚动条
	SetScrollSizes();

	//窗口变化时刷新界面
	Invalidate(TRUE);
}

void RadarViewAssistant::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO scrollinfo;
	GetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);

	switch (nSBCode)
	{
	case SB_RIGHT: //滑块滚动到最右边
		//ScrollWindow(0, (scrollinfo.nPos - scrollinfo.nMax) * 10); //滚动屏幕
		scrollinfo.nPos = scrollinfo.nMax; //设定滑块新位置
		SetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL); //更新滑块位置
		break;

	case SB_LEFT: //滑块滚动到最左边
		//ScrollWindow(0, (scrollinfo.nPos - scrollinfo.nMin) * 10);
		scrollinfo.nPos = scrollinfo.nMin;
		SetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);
		break;

	case SB_LINELEFT: //单击左箭头
		scrollinfo.nPos -= 30;
		if (scrollinfo.nPos < scrollinfo.nMin)
			scrollinfo.nPos = scrollinfo.nMin;
		SetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);
		//ScrollWindow(0, 30);
		break;

	case SB_LINERIGHT: //单击右箭头
		scrollinfo.nPos += 30;
		if (scrollinfo.nPos > scrollinfo.nMax)
		{
			scrollinfo.nPos = scrollinfo.nMax;
			break;
		}
		SetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);
		//ScrollWindow(0, -30);
		break;

	case SB_PAGELEFT: //单击滑块左边空白区域
		scrollinfo.nPos -= 60;
		if (scrollinfo.nPos < scrollinfo.nMin)
		{
			scrollinfo.nPos = scrollinfo.nMin;
			break;
		}
		SetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);
		//ScrollWindow(0, 10 * 5);
		break;

	case SB_PAGERIGHT: //单击滑块右边空白区域
		scrollinfo.nPos += 60;
		if (scrollinfo.nPos > scrollinfo.nMax)
		{
			scrollinfo.nPos = scrollinfo.nMax;
			break;
		}
		SetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);
		//ScrollWindow(0, -10 * 5);
		break;

	case SB_ENDSCROLL: //鼠标离开滑块，结束滑块拖动
	// MessageBox("SB_ENDSCROLL");
		break;

	case SB_THUMBPOSITION:
		// ScrollWindow(0,(scrollinfo.nPos-nPos)*10);
		// scrollinfo.nPos = nPos;
		// SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);
		break;

	case SB_THUMBTRACK: //拖动滑块
		//ScrollWindow(0, (scrollinfo.nPos - nPos) * 10);
		scrollinfo.nPos = nPos;
		SetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);
		break;
	}

	//水平滚动时刷新界面
	Invalidate(TRUE);

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
void RadarViewAssistant::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO scrollinfo;
	GetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);

	switch (nSBCode)
	{
	case SB_BOTTOM: //滑块滚动到最底部
		ScrollWindow(0, (scrollinfo.nPos - scrollinfo.nMax) * 10); //滚动屏幕
		scrollinfo.nPos = scrollinfo.nMax; //设定滑块新位置
		SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL); //更新滑块位置
		break;

	case SB_TOP: //滑块滚动到最顶部
		ScrollWindow(0, (scrollinfo.nPos - scrollinfo.nMin) * 10);
		scrollinfo.nPos = scrollinfo.nMin;
		SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
		break;

	case SB_LINEUP: //单击上箭头
		scrollinfo.nPos -= 30;
		if (scrollinfo.nPos < scrollinfo.nMin)
			scrollinfo.nPos = scrollinfo.nMin;
		SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
		//ScrollWindow(0, 30);
		break;

	case SB_LINEDOWN: //单击下箭头
		scrollinfo.nPos += 30;
		if (scrollinfo.nPos > scrollinfo.nMax)
		{
			scrollinfo.nPos = scrollinfo.nMax;
			break;
		}
		SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
		//ScrollWindow(0, -30);
		break;

	case SB_PAGEUP: //单击滑块上方空白区域
		scrollinfo.nPos -= 60;
		if (scrollinfo.nPos < scrollinfo.nMin)
		{
			scrollinfo.nPos = scrollinfo.nMin;
			break;
		}
		SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
		//ScrollWindow(0, 10 * 5);
		break;

	case SB_PAGEDOWN: //单击滑块下方空白区域
		scrollinfo.nPos += 60;
		if (scrollinfo.nPos > scrollinfo.nMax)
		{
			scrollinfo.nPos = scrollinfo.nMax;
			break;
		}
		SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
		//ScrollWindow(0, -10 * 5);
		break;

	case SB_ENDSCROLL: //鼠标离开滑块，结束滑块拖动
	// MessageBox("SB_ENDSCROLL");
		break;

	case SB_THUMBPOSITION:
		// ScrollWindow(0,(scrollinfo.nPos-nPos)*10);
		// scrollinfo.nPos = nPos;
		// SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);
		break;

	case SB_THUMBTRACK: //拖动滑块
		//ScrollWindow(0, (scrollinfo.nPos - nPos) * 10);
		scrollinfo.nPos = nPos;
		SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
		break;
	}

	//垂直滚动时刷新界面
	Invalidate(TRUE);

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}
BOOL RadarViewAssistant::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	//滚动时刷新界面
	Invalidate(FALSE);

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

//双击鼠标右键，弹出病害卡窗口
void RadarViewAssistant::OnLButtonDblClk(UINT nFlags, CPoint point)
{
/*
	if (NULL == m_pChannel1)
		return;
	if (NULL == m_pSwath)
		return;
	if (NULL == m_pProject)
		return;

	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//取得当前滚动条的位置
	CPoint oPointStart = GetScrollPosition();
	//鼠标双击时的x位置(相对于完整图像的开头到当前的X坐标)
	int iCurrentX = oPointStart.x + point.x - SPACE_LEFT;
	//鼠标双击时的y位置(相对于完整图像的开头到当前的Y坐标)
	int iCurrentY = point.y - SPACE_TOP;

	//位置超出右边范围
	if (iCurrentX > (oPointStart.x + oRect.Width() - 2 * SPACE_LEFT))
		return;
	//位置超出左边范围
	if (oPointStart.x > iCurrentX)
		return;

	//位置超出下边范围
	if (iCurrentY > m_pChannel1->getChannelHeader()->getSample())
		return;
	//位置超出上边范围
	if (0 > iCurrentY)
		return;

	//弹出病害卡窗口（在对话框上需要有一些参数的输入）,生成指定位置的病害卡
	DialogCard oCard;
	oCard.SetParam(m_pProject, m_pSwath, m_pChannel1, iCurrentX, iCurrentY);
	oCard.DoModal();
*/
	CDialogEx::OnLButtonDblClk(nFlags, point);
}

//创建View上显示出的内存DC
void RadarViewAssistant::CreateViewSide(CDC& oDC)
{
	if (NULL == m_pProject)
		return;

	//显示对比度
	int iContrast = RadarViewSetting::GetContrast();
	//增益
	int iTimeGain = RadarViewSetting::GetTimegain();

	if (NULL != m_pChannel1)
	{
		//初始化显示系数
		m_pChannel1->getChannelHeader()->setCoef(iContrast, iTimeGain);
		//绘制坐标
		CreateScale1(oDC);
		//构造侧视图开始
		CreateViewSide1(oDC);
	}

	if (NULL != m_pChannel2)
	{
		//初始化显示系数
		m_pChannel2->getChannelHeader()->setCoef(iContrast, iTimeGain);
		//绘制坐标
		CreateScale2(oDC);
		//构造侧视图开始
		CreateViewSide2(oDC);
	}

	if (NULL != m_pChannel3)
	{
		//初始化显示系数
		m_pChannel3->getChannelHeader()->setCoef(iContrast, iTimeGain);
		//绘制坐标
		CreateScale3(oDC);
		//构造侧视图开始
		CreateViewSide3(oDC);
	}
}
//绘制水平坐标和垂直坐标(第一视图)
void RadarViewAssistant::CreateScale1(CDC& oDC)
{
	USES_CONVERSION;
	CString strScaleText = _T("");
	strScaleText.Format(_T("距离[m]      测线：%s；  通道%d"), A2W(m_pSwath1->getSwathID()), m_pChannel1->getNo() );
	// Horizontal scale
	m_Scale.SetTitle(TRUE, strScaleText);
	strScaleText = _T("深度[m]");
	// Depth scale
	m_Scale.SetTitle(FALSE, strScaleText);

	//======取得当前滚动条的位置=======//
	SCROLLINFO oScrollInfoV;
	GetScrollInfo(SB_VERT, &oScrollInfoV, SIF_ALL);
	SCROLLINFO oScrollInfoH;
	GetScrollInfo(SB_HORZ, &oScrollInfoH, SIF_ALL);
	CPoint oPoint(oScrollInfoH.nPos, oScrollInfoV.nPos);

	//======绘制雷达侧视图的比例尺=======//
	//绘制比例标尺
	CRect rect1 = CRect(0, 0, m_iWidth1 - oPoint.x, m_iHeight1);
	rect1.OffsetRect(CPoint(SPACE_LEFT, SPACE_TOP));
	CRect rect2 = CRect(0, 0, m_iWidth1, m_iHeight1);
	rect2.OffsetRect(CPoint(0, 0));

	int iZLevel = m_pChannel1->getChannelHeader()->getZeroLevel();
	double cx = m_pChannel1->getChannelHeader()->getIntervalDist();
	double cy = 1 / m_pChannel1->getChannelHeader()->getFrequency() * m_pChannel1->getChannelHeader()->getSoilVel() / 2;

	FSize units = FSize(cx, cy);
	m_Scale.sInit(rect1, rect2, units, iZLevel);
	m_Scale.Draw(&oDC, oPoint.x, oPoint.y);
}
//绘制水平坐标和垂直坐标(第二视图)
void RadarViewAssistant::CreateScale2(CDC& oDC)
{
	USES_CONVERSION;
	CString strScaleText = _T("");
	strScaleText.Format(_T("距离[m]      测线：%s；  通道%d"), A2W(m_pSwath2->getSwathID()), m_pChannel2->getNo());
	// Horizontal scale
	m_Scale.SetTitle(TRUE, strScaleText);
	strScaleText = _T("深度[m]");
	// Depth scale
	m_Scale.SetTitle(FALSE, strScaleText);

	//======取得当前滚动条的位置=======//
	SCROLLINFO oScrollInfoV;
	GetScrollInfo(SB_VERT, &oScrollInfoV, SIF_ALL);
	SCROLLINFO oScrollInfoH;
	GetScrollInfo(SB_HORZ, &oScrollInfoH, SIF_ALL);
	CPoint oPoint(oScrollInfoH.nPos,oScrollInfoV.nPos);

	//======绘制雷达侧视图的比例尺=======//
	//绘制比例标尺
	CRect rect1 = CRect(0, 0, m_iWidth2 - oPoint.x, m_iHeight2);
	rect1.OffsetRect(CPoint(SPACE_LEFT, SPACE_TOP + m_iHeight1 + SPACE_TOP_SIDE));
	CRect rect2 = CRect(0, 0, m_iWidth2, m_iHeight2);
	rect2.OffsetRect(CPoint(0, 0));

	int iZLevel = m_pChannel2->getChannelHeader()->getZeroLevel();
	double cx = m_pChannel2->getChannelHeader()->getIntervalDist();
	double cy = 1 / m_pChannel2->getChannelHeader()->getFrequency() * m_pChannel2->getChannelHeader()->getSoilVel() / 2;

	FSize units = FSize(cx, cy);
	m_Scale.sInit(rect1, rect2, units, iZLevel);
	m_Scale.Draw(&oDC, oPoint.x, oPoint.y);
}
//绘制水平坐标和垂直坐标(第三视图)
void RadarViewAssistant::CreateScale3(CDC& oDC)
{
	USES_CONVERSION;
	CString strScaleText = _T("");
	strScaleText.Format(_T("距离[m]      测线：%s；  通道%d"), A2W(m_pSwath3->getSwathID()), m_pChannel3->getNo());
	// Horizontal scale
	m_Scale.SetTitle(TRUE, strScaleText);
	strScaleText = _T("深度[m]");
	// Depth scale
	m_Scale.SetTitle(FALSE, strScaleText);

	//======取得当前滚动条的位置=======//
	SCROLLINFO oScrollInfoV;
	GetScrollInfo(SB_VERT, &oScrollInfoV, SIF_ALL);
	SCROLLINFO oScrollInfoH;
	GetScrollInfo(SB_HORZ, &oScrollInfoH, SIF_ALL);
	CPoint oPoint(oScrollInfoH.nPos, oScrollInfoV.nPos);

	//======绘制雷达侧视图的比例尺=======//
	//绘制比例标尺
	CRect rect1 = CRect(0, 0, m_iWidth3 - oPoint.x, m_iHeight3);
	rect1.OffsetRect(CPoint(SPACE_LEFT, SPACE_TOP + m_iHeight1 + SPACE_TOP_SIDE + m_iHeight2 + SPACE_TOP_SIDE));
	CRect rect2 = CRect(0, 0, m_iWidth3, m_iHeight3);
	rect2.OffsetRect(CPoint(0, 0));

	int iZLevel = m_pChannel3->getChannelHeader()->getZeroLevel();
	double cx = m_pChannel3->getChannelHeader()->getIntervalDist();
	double cy = 1 / m_pChannel3->getChannelHeader()->getFrequency() * m_pChannel3->getChannelHeader()->getSoilVel() / 2;

	FSize units = FSize(cx, cy);
	m_Scale.sInit(rect1, rect2, units, iZLevel);
	m_Scale.Draw(&oDC, oPoint.x, oPoint.y);
}

//将雷达数据绘制为BMP--侧视图(第一视图)
void RadarViewAssistant::CreateViewSide1(CDC& oDC)
{
	//读取显示系数
	double* tgCoef = m_pChannel1->getChannelHeader()->getCoef();

	//======取得当前滚动条的位置=======//
	SCROLLINFO oScrollInfoV;
	GetScrollInfo(SB_VERT, &oScrollInfoV, SIF_ALL);

	SCROLLINFO oScrollInfoH;
	GetScrollInfo(SB_HORZ, &oScrollInfoH, SIF_ALL);

	CPoint oPoint(oScrollInfoH.nPos, oScrollInfoV.nPos);
	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//侧视图，获取数据区域的高度和宽度(也就是侧视图显示区域的高度和宽度)
	int iWidthToDisplay = SPACE_LEFT + m_iWidth1 - oPoint.x;
	if (iWidthToDisplay <= 0)
		return;
	if (iWidthToDisplay > (oRect.Width() - SPACE_LEFT))
		iWidthToDisplay = oRect.Width() - SPACE_LEFT;

	//===================构造侧视图开始==================//
	//构造位图信息头(只构造显示区域的位图，超出部分不构造)
	BITMAPINFOHEADER oBIHSideView;
	memset(&oBIHSideView, 0, sizeof(oBIHSideView));
	oBIHSideView.biSize        = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
	oBIHSideView.biHeight      = m_iHeight1;                  //位图的高度，以像素为单位
	oBIHSideView.biWidth       = iWidthToDisplay;             //位图的宽度，以像素为单位
	oBIHSideView.biPlanes      = 1;                           //位平面数，必须为1
	oBIHSideView.biBitCount    = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHSideView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHSideView.biSizeImage   = m_iHeight1 *
		                        iWidthToDisplay *
		                        4;                            //位图的大小，以字节为单位

	//分配像素空间，侧视图数据
	COLORREF* pPicSideView = new COLORREF[m_iHeight1 * iWidthToDisplay];

	//获取需要的Trace像素数据
	std::map<int, Trace16*>* pTraceData = m_pChannel1->getChannelBlob()->getTraceData();

	int iTraceCount = m_pChannel1->getChannelBlob()->getTraceCount();

	//循环将像素数据转化为BMP(一列一列处理)
	for (int i = oPoint.x; i < iWidthToDisplay + oPoint.x; i++)
	{
		Trace16* pTraceTmp = (*pTraceData)[i];
		if (NULL == pTraceTmp)
			break;
		if (i >= iTraceCount)
			break;

		short* pData = pTraceTmp->getTrace();
		for (int jj = 0; jj < m_iHeight1; jj++)
		{
			short sColor = int(pData[jj] * tgCoef[jj]) / 2048 + 32;

			if (sColor < 0)
				sColor = 0;
			else if (sColor > 63)
				sColor = 63;

			COLORREF* pBuf = pPicSideView + iWidthToDisplay * (m_iHeight1 - jj - 1) + i - oPoint.x;
			*pBuf = m_Palette.getColorref()[sColor];
		}
	}
	//===================构造侧视图完成==================//

	CClientDC oDCTmp(this);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView, CBM_INIT, pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;    oDCImage.CreateCompatibleDC(&oDCTmp);    //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象
	oDC.BitBlt(SPACE_LEFT, SPACE_TOP - oPoint.y, iWidthToDisplay - SPACE_LEFT, m_iHeight1, &oDCImage, 0, 0, SRCCOPY); //显示
	oDCImage.DeleteDC();
	oBitmap.DeleteObject();
	DeleteObject(hBitmap);

	//释放BMP像素空间
	delete[] pPicSideView;

	return;
}
//将雷达数据绘制为BMP--侧视图(第一视图)
void RadarViewAssistant::CreateViewSide2(CDC& oDC)
{
	//读取显示系数
	double* tgCoef = m_pChannel2->getChannelHeader()->getCoef();

	//======取得当前滚动条的位置=======//
	SCROLLINFO oScrollInfoV;
	GetScrollInfo(SB_VERT, &oScrollInfoV, SIF_ALL);

	SCROLLINFO oScrollInfoH;
	GetScrollInfo(SB_HORZ, &oScrollInfoH, SIF_ALL);

	CPoint oPoint(oScrollInfoH.nPos, oScrollInfoV.nPos);

	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//侧视图，获取数据区域的高度和宽度(也就是侧视图显示区域的高度和宽度)
	int iWidthToDisplay = SPACE_LEFT + m_iWidth2 - oPoint.x;
	if (iWidthToDisplay <= 0)
		return;
	if (iWidthToDisplay > (oRect.Width() - SPACE_LEFT))
		iWidthToDisplay = oRect.Width() - SPACE_LEFT;

	//===================构造侧视图开始==================//
	//构造位图信息头(只构造显示区域的位图，超出部分不构造)
	BITMAPINFOHEADER oBIHSideView;
	memset(&oBIHSideView, 0, sizeof(oBIHSideView));
	oBIHSideView.biSize        = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
	oBIHSideView.biHeight      = m_iHeight2;                  //位图的高度，以像素为单位
	oBIHSideView.biWidth       = iWidthToDisplay;             //位图的宽度，以像素为单位
	oBIHSideView.biPlanes      = 1;                           //位平面数，必须为1
	oBIHSideView.biBitCount    = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHSideView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHSideView.biSizeImage   = m_iHeight2 *
	                             iWidthToDisplay *
		                         4;                           //位图的大小，以字节为单位

    //分配像素空间，侧视图数据
	COLORREF* pPicSideView = new COLORREF[m_iHeight2 * iWidthToDisplay];

	//获取需要的Trace像素数据
	std::map<int, Trace16*>* pTraceData = m_pChannel2->getChannelBlob()->getTraceData();

	int iTraceCount = m_pChannel2->getChannelBlob()->getTraceCount();

	//循环将像素数据转化为BMP(一列一列处理)
	for (int i = oPoint.x; i < iWidthToDisplay + oPoint.x; i++)
	{
		Trace16* pTraceTmp = (*pTraceData)[i];
		if (NULL == pTraceTmp)
			break;
		if (i >= iTraceCount)
			break;

		short* pData = pTraceTmp->getTrace();
		for (int jj = 0; jj < m_iHeight2; jj++)
		{
			short sColor = int(pData[jj] * tgCoef[jj]) / 2048 + 32;

			if (sColor < 0)
				sColor = 0;
			else if (sColor > 63)
				sColor = 63;

			COLORREF* pBuf = pPicSideView + iWidthToDisplay * (m_iHeight2 - jj - 1) + i - oPoint.x;
			*pBuf = m_Palette.getColorref()[sColor];
		}
	}
	//===================构造侧视图完成==================//

	CClientDC oDCTmp(this);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView, CBM_INIT, pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;    oDCImage.CreateCompatibleDC(&oDCTmp);    //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象
	oDC.BitBlt(SPACE_LEFT, SPACE_TOP + m_iHeight1 + SPACE_TOP_SIDE  - oPoint.y, iWidthToDisplay - SPACE_LEFT, m_iHeight2, &oDCImage, 0, 0, SRCCOPY); //显示
	oDCImage.DeleteDC();
	oBitmap.DeleteObject();
	DeleteObject(hBitmap);

	//释放BMP像素空间
	delete[] pPicSideView;


	/*
	//下面为显示图像到Picture Control
	CWnd * pWnd = GetDlgItem(IDC_CHANNEL_DATA);
	pWnd->ShowWindow(true);
	//pWnd->SetWindowPos(NULL,0,0, iWidth, iHeight,SWP_NOMOVE);

	CRect rect;
	pWnd->GetClientRect(&rect);    //获得pictrue控件所在的矩形区域
	CDC *pDC = pWnd->GetDC();      //获得pictrue控件的DC
	int ModeOld = SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);
	//pDC->SetStretchBltMode(COLORONCOLOR);
	CClientDC oDC(pWnd);               //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDC.m_hDC, &oBIHSideView, CBM_INIT, m_pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);    //关联位图对象
	BITMAP bmp;      oBitmap.GetBitmap(&bmp);    //获取位图信息

	CDC oDCImage;
	oDCImage.CreateCompatibleDC(pDC);  //内存DC
	oDCImage.SelectObject(&oBitmap);   //选取位图对象
	pDC->BitBlt(0, 0, iWidth, iHeight, &oDCImage, 0, 0, SRCCOPY); //显示
	SetStretchBltMode(pDC->m_hDC, ModeOld);
	*/

	/*
	//构造位图文件头(调试，用于存储文件)
	BITMAPFILEHEADER bfh;
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfType      = (WORD)0x4d42;                                           //位图文件的类型，必须为BM 0x424d 表示.bmp
	bfh.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);    //位图数据距文件头的偏移量，以字节为单位 即前三部分和
	bfh.bfSize      = bfh.bfOffBits + iWidth * iHeight * 4;                   //位图文件的大小，以字节为单位 包括该14字节
	bfh.bfReserved1 = 0;                                                      //位图文件保留字，必须为0
	bfh.bfReserved2 = 0;                                                      //位图文件保留字，必须为0

	//生成图像文件，用于调测，暂时不要删除
	char szPathFile[] = { "D:\\1.bmp" };
	FILE *fp;
	fopen_s(&fp, szPathFile, "wb");
	//BMP文件头写入图像
	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);
	//BMP信息头写入图像
	fwrite(&oBIHSideView, 1, sizeof(BITMAPINFOHEADER), fp);
	//写入图像文件
	fwrite(m_pPicSideView, 1, iWidth * iHeight * 4, fp);
	//关闭图像文件
	fclose(fp);
	*/
}
//将雷达数据绘制为BMP--侧视图(第三视图)
void RadarViewAssistant::CreateViewSide3(CDC& oDC)
{
	//读取显示系数
	double* tgCoef = m_pChannel3->getChannelHeader()->getCoef();

	//======取得当前滚动条的位置=======//
	SCROLLINFO oScrollInfoV;
	GetScrollInfo(SB_VERT, &oScrollInfoV, SIF_ALL);

	SCROLLINFO oScrollInfoH;
	GetScrollInfo(SB_HORZ, &oScrollInfoH, SIF_ALL);

	CPoint oPoint(oScrollInfoH.nPos, oScrollInfoV.nPos);

	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//侧视图，获取数据区域的高度和宽度(也就是侧视图显示区域的高度和宽度)
	int iWidthToDisplay = SPACE_LEFT + m_iWidth3 - oPoint.x;
	if (iWidthToDisplay <= 0)
		return;
	if (iWidthToDisplay > (oRect.Width() - SPACE_LEFT))
		iWidthToDisplay = oRect.Width() - SPACE_LEFT;

	//===================构造侧视图开始==================//
	//构造位图信息头(只构造显示区域的位图，超出部分不构造)
	BITMAPINFOHEADER oBIHSideView;
	memset(&oBIHSideView, 0, sizeof(oBIHSideView));
	oBIHSideView.biSize        = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
	oBIHSideView.biHeight      = m_iHeight3;                  //位图的高度，以像素为单位
	oBIHSideView.biWidth       = iWidthToDisplay;             //位图的宽度，以像素为单位
	oBIHSideView.biPlanes      = 1;                           //位平面数，必须为1
	oBIHSideView.biBitCount    = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHSideView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHSideView.biSizeImage   = m_iHeight3 *
		                         iWidthToDisplay *
		                         4;                           //位图的大小，以字节为单位

    //分配像素空间，侧视图数据
	COLORREF* pPicSideView = new COLORREF[m_iHeight3 * iWidthToDisplay];

	//获取需要的Trace像素数据
	std::map<int, Trace16*>* pTraceData = m_pChannel3->getChannelBlob()->getTraceData();

	int iTraceCount = m_pChannel3->getChannelBlob()->getTraceCount();

	//循环将像素数据转化为BMP(一列一列处理)
	for (int i = oPoint.x; i < iWidthToDisplay + oPoint.x; i++)
	{
		Trace16* pTraceTmp = (*pTraceData)[i];
		if (NULL == pTraceTmp)
			break;
		if (i >= iTraceCount)
			break;

		short* pData = pTraceTmp->getTrace();
		for (int jj = 0; jj < m_iHeight3; jj++)
		{
			short sColor = int(pData[jj] * tgCoef[jj]) / 2048 + 32;

			if (sColor < 0)
				sColor = 0;
			else if (sColor > 63)
				sColor = 63;

			COLORREF* pBuf = pPicSideView + iWidthToDisplay * (m_iHeight3 - jj - 1) + i - oPoint.x;
			*pBuf = m_Palette.getColorref()[sColor];
		}
	}
	//===================构造侧视图完成==================//

	CClientDC oDCTmp(this);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView, CBM_INIT, pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;    oDCImage.CreateCompatibleDC(&oDCTmp);    //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象
	oDC.BitBlt(SPACE_LEFT, SPACE_TOP + m_iHeight1 + SPACE_TOP_SIDE  + m_iHeight2 + SPACE_TOP_SIDE - oPoint.y, iWidthToDisplay - SPACE_LEFT, m_iHeight3, &oDCImage, 0, 0, SRCCOPY); //显示
	oDCImage.DeleteDC();
	oBitmap.DeleteObject();
	DeleteObject(hBitmap);

	//释放BMP像素空间
	delete[] pPicSideView;
}

//设置水平滚动的幅度
void RadarViewAssistant::SetHScroll(UINT nPos)
{
	SCROLLINFO scrollinfo;
	GetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);

	//设定滑块新位置
	if ( (0 < nPos) && (nPos < (UINT)scrollinfo.nMax) )
		scrollinfo.nPos = nPos;
	else if (nPos > (UINT)scrollinfo.nMax)
		scrollinfo.nPos = scrollinfo.nMax;
	else
		scrollinfo.nPos = 0;

	SetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL); //更新滑块位置
}




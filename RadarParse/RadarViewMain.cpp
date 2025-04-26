
// RadarViewMain.cpp: RadarViewMain 类的实现
//

#include "stdafx.h"
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




// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "RadarApp.h"
#endif

#include "RadarFrame.h"
#include "RadarDoc.h"
#include "RadarViewMain.h"
#include "DialogCard.h"
#include "RadarViewSetting.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define SPACE_LEFT     50           //左边垂直标尺绘制时，距离左边的距离
#define SPACE_TOP      50           //上边水平标尺绘制时，距离顶部的距离
#define SPACE_TOP_SIDE 50           //侧视图和俯视图之间的距离

// RadarViewMain
IMPLEMENT_DYNCREATE(RadarViewMain, CFormView)
BEGIN_MESSAGE_MAP(RadarViewMain, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()

	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// RadarViewMain 构造/析构
RadarViewMain::RadarViewMain() noexcept: CFormView(IDD_VIEW_MAIN)
{
	m_pProject  = NULL;
	m_pSwath    = NULL;
	m_pChannel1 = NULL;
	m_pChannel2 = NULL;

	m_PointStart.x = m_PointStart.y = 0;
	m_PointEnd.x = m_PointEnd.y = 0;
	m_bMouseDownFlag = false;
}

RadarViewMain::~RadarViewMain()
{
	std::vector<CRect*>::iterator it;
	for (it = m_lstMark.begin(); it != m_lstMark.end(); it++)
	{
		//释放目标空间
		CRect * pMark = *it;
		delete pMark;
	}
	m_lstMark.clear();
}

void RadarViewMain::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL RadarViewMain::PreCreateWindow(CREATESTRUCT& cs)
{
	//  CREATESTRUCT cs 来修改窗口类或样式
	cs.style &= WS_HSCROLL | ~WS_THICKFRAME;
	cs.style &= WS_VSCROLL | ~WS_THICKFRAME;

	return CFormView::PreCreateWindow(cs);
}

int RadarViewMain::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void RadarViewMain::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
}

void RadarViewMain::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

BOOL RadarViewMain::OnEraseBkgnd(CDC* pDC)
{
	if( NULL == m_pSwath)
		return CFormView::OnEraseBkgnd(pDC);
	else
		return TRUE;
}

// RadarViewMain 诊断
#ifdef _DEBUG
void RadarViewMain::AssertValid() const
{
	CFormView::AssertValid();
}

void RadarViewMain::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
/*
RadarDoc* RadarViewMain::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(RadarDoc)));
	return (RadarDoc*)m_pDocument;
}
*/
#endif //_DEBUG


//设置工程
void RadarViewMain::SetProject(Project* pProject)
{
	if (NULL == pProject)
		return;

	//清理以前分配的数据

	//记录工程
	m_pProject = pProject;

	//更新界面显示
	Invalidate(TRUE);
}
void RadarViewMain::SetProjectData(Swath *pSwath , SwathChannel *pChannel)
{
	if (NULL == pSwath)
		return;
	if (NULL == pChannel)
		return;

	//============加载Trace数据==开始=============//
	//以前还没有加载过Trace数据
	if (m_pSwath != pSwath)
	{
		//加载Swath通道数据
		int iChannelCount = 0;
		pSwath->getChannelCount(iChannelCount);
		for (int i = 1; i <= iChannelCount; i++)
		{
			SwathChannel* pChannel = pSwath->getChannel(i);
			pChannel->getChannelBlob()->loadTrace();
		}
	}
	//============加载Trace数据==完成=============//

	//记录输入对象
	m_pSwath   = pSwath;

	//============计算图像的宽高==开始============//
    //获取通道数目
	int iChannelCount = 0;
	m_pSwath->getChannelCount(iChannelCount);

	//取得图像的高和宽，用于设置滚动条
	m_iHeight = pChannel->getChannelHeader()->getSample();
	//获取最小的trace数，作为数据图像区域的完整宽
	m_iWidth = m_pSwath->getChannel(1)->getChannelHeader()->getTraceCount();
	for (int i = 1; i <= iChannelCount; i++)
	{
		SwathChannel* pChannel = m_pSwath->getChannel(i);
		int iTraceCount = pChannel->getChannelHeader()->getTraceCount();
		if (iTraceCount < m_iWidth)
			m_iWidth = iTraceCount;
	}
	//============计算图像的宽高==完成============//

	//根据图像的高宽设置滚动条
	CSize sizeTotal(SPACE_LEFT * 2 + m_iWidth, SPACE_TOP + m_iHeight + SPACE_TOP + m_iHeight + SPACE_TOP_SIDE + iChannelCount * 2);
	SetScrollSizes(MM_TEXT, sizeTotal);

	//更新界面显示
	Invalidate(TRUE);
}

void RadarViewMain::SetChannelViewOne(Swath* pSwath, SwathChannel* pChannel)
{
	if (NULL == m_pProject)
		return;
	if (NULL == pSwath)
		return;
	if (NULL == pChannel)
		return;

	//如果测线不一样
	if (m_pSwath != pSwath)
		m_pChannel2 = NULL;

	//初始化Marks和按键状态
	initMarks();

	m_pChannel1 = pChannel;

	SetProjectData(pSwath, pChannel);
}
void RadarViewMain::SetChannelViewTwo(Swath* pSwath, SwathChannel* pChannel)
{
	if (NULL == m_pProject)
		return;
	if (NULL == pSwath)
		return;
	if (NULL == pChannel)
		return;

	//如果测线不一样
	if (m_pSwath != pSwath)
		m_pChannel1 = NULL;

	//初始化Marks和按键状态
	initMarks();

	m_pChannel2 = pChannel;
	SetProjectData(pSwath, pChannel);
}

SwathChannel * RadarViewMain::GetChannelViewOne()
{
	return m_pChannel1;    //测线通道对象(第一视图显示的通道)
}
SwathChannel * RadarViewMain::GetChannelViewTwo()
{
	return m_pChannel2;    //测线通道对象(第二视图显示的通道)
}

//创建View上显示出的内存DC
void RadarViewMain::CreateViewSide(CDC& oDC)
{
	if (NULL == m_pSwath)
		return;
	if (NULL == m_pProject)
		return;

	//初始化显示系数
	UpdateCoef();

	if (NULL != m_pChannel1)
	{
		//绘制坐标
		CreateScale1(oDC);
		//构造侧视图开始
		CreateViewSide1(oDC);
	}

	if (NULL != m_pChannel2)
	{
		//绘制坐标
		CreateScale2(oDC);
		//构造侧视图开始
		CreateViewSide2(oDC);
	}
}
//绘制水平坐标和垂直坐标(第一视图)
void RadarViewMain::CreateScale1(CDC& oDC)
{
	USES_CONVERSION;
	CString strScaleText = _T( "" );
	strScaleText.Format(_T("距离[m]      测线：%s；  通道%d"), A2W(m_pSwath->getSwathID()), m_pChannel1->getNo());
	// Horizontal scale
	m_Scale.SetTitle(TRUE, strScaleText);
	strScaleText = _T("深度[m]");
	// Depth scale
	m_Scale.SetTitle(FALSE, strScaleText);

	//======取得当前滚动条的位置=======//
	CPoint oPoint = GetScrollPosition();

	//======绘制雷达侧视图的比例尺=======//
	//绘制比例标尺
	CRect rect1 = CRect(0, 0, m_iWidth - oPoint.x, m_iHeight);
	rect1.OffsetRect(CPoint(SPACE_LEFT, SPACE_TOP));
	CRect rect2 = CRect(0, 0, m_iWidth, m_iHeight);
	rect2.OffsetRect(CPoint(0, 0));

	int iZLevel = m_pChannel1->getChannelHeader()->getZeroLevel();
	double cx = m_pChannel1->getChannelHeader()->getIntervalDist();
	double cy = 1 / m_pChannel1->getChannelHeader()->getFrequency() * m_pChannel1->getChannelHeader()->getSoilVel() / 2;

	FSize units = FSize(cx, cy);
	m_Scale.sInit(rect1, rect2, units, iZLevel);
	m_Scale.Draw(&oDC, oPoint.x, oPoint.y);
}
//绘制水平坐标和垂直坐标(第二视图)
void RadarViewMain::CreateScale2(CDC& oDC)
{
	USES_CONVERSION;
	CString strScaleText = _T("");
	strScaleText.Format(_T("距离[m]      测线：%s；  通道%d"), A2W(m_pSwath->getSwathID()), m_pChannel2->getNo());
	// Horizontal scale
	m_Scale.SetTitle(TRUE, strScaleText);
	strScaleText = _T("深度[m]");
	// Depth scale
	m_Scale.SetTitle(FALSE, strScaleText);

	//======取得当前滚动条的位置=======//
	CPoint oPoint = GetScrollPosition();

	//======绘制雷达侧视图的比例尺=======//
	//绘制比例标尺
	CRect rect1 = CRect(0, 0, m_iWidth - oPoint.x, m_iHeight);
	rect1.OffsetRect(CPoint(SPACE_LEFT, SPACE_TOP * 2 + m_iHeight));
	CRect rect2 = CRect(0, 0, m_iWidth, m_iHeight);
	rect2.OffsetRect(CPoint(0, 0));

	int iZLevel = m_pChannel2->getChannelHeader()->getZeroLevel();
	double cx = m_pChannel2->getChannelHeader()->getIntervalDist();
	double cy = 1 / m_pChannel2->getChannelHeader()->getFrequency() * m_pChannel2->getChannelHeader()->getSoilVel() / 2;

	FSize units = FSize(cx, cy);
	m_Scale.sInit(rect1, rect2, units, iZLevel);
	m_Scale.Draw(&oDC, oPoint.x, oPoint.y);
}
//将雷达数据绘制为BMP--侧视图(第一视图)
void RadarViewMain::CreateViewSide1(CDC& oDC)
{
	//读取显示系数
	double* tgCoef = m_pChannel1->getChannelHeader()->getCoef();

	//======取得当前滚动条的位置=======//
	CPoint oPoint = GetScrollPosition();
	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//侧视图，获取数据区域的高度和宽度(也就是侧视图显示区域的高度和宽度)
	int iWidthToDisplay = SPACE_LEFT + m_iWidth - oPoint.x;
	if (iWidthToDisplay <= 0)
		return;
	if (iWidthToDisplay > (oRect.Width() - SPACE_LEFT))
		iWidthToDisplay = oRect.Width() - SPACE_LEFT;

	//===================构造侧视图开始==================//
	//构造位图信息头(只构造显示区域的位图，超出部分不构造)
	BITMAPINFOHEADER oBIHSideView;
	memset(&oBIHSideView, 0, sizeof(oBIHSideView));
	oBIHSideView.biSize        = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
	oBIHSideView.biHeight      = m_iHeight;                   //位图的高度，以像素为单位
	oBIHSideView.biWidth       = iWidthToDisplay;             //位图的宽度，以像素为单位
	oBIHSideView.biPlanes      = 1;                           //位平面数，必须为1
	oBIHSideView.biBitCount    = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHSideView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHSideView.biSizeImage   = m_iHeight *
                                 iWidthToDisplay *
                                 4;                           //位图的大小，以字节为单位

    //分配像素空间，侧视图数据
	COLORREF* pPicSideView = new COLORREF[m_iHeight * iWidthToDisplay];

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
		for (int jj = 0; jj < m_iHeight; jj++)
		{
			short sColor = int(pData[jj] * tgCoef[jj]) / 2048 + 64;

			if (sColor < 0)
				sColor = 0;
			else if (sColor > 128)
				sColor = 128;

			COLORREF* pBuf = pPicSideView + iWidthToDisplay * (m_iHeight - jj - 1) + i - oPoint.x;
			*pBuf = m_Palette.getColorref()[sColor];
			//*pBuf = i* 0x00400/2;
		}
	}
	//===================构造侧视图完成==================//

	CClientDC oDCTmp(this);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView, CBM_INIT, pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;    oDCImage.CreateCompatibleDC(&oDCTmp);    //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象
	oDC.BitBlt(SPACE_LEFT, SPACE_TOP - oPoint.y, iWidthToDisplay - SPACE_LEFT, m_iHeight, &oDCImage, 0, 0, SRCCOPY); //显示
	oDCImage.DeleteDC();
	oBitmap.DeleteObject();
	DeleteObject(hBitmap);

	//测试保存为图片---开始
	/*
	CImage image;
	image.Create(m_iWidth, m_iHeight, 32);
	BitBlt(image.GetDC(),0,0, m_iWidth, m_iHeight, oDCImage.m_hDC, 0, 0, SRCCOPY);
	HRESULT hResult = image.Save("C:\\MyTest.jpg");
	if (!FAILED(hResult))
	{
		MessageBox( _T( "保存图片失败!" ) );
	}
	image.ReleaseDC();
	*/
	//测试保存为图片---结束

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

	return;
}
//将雷达数据绘制为BMP--侧视图(第一视图)
void RadarViewMain::CreateViewSide2(CDC& oDC)
{
	//读取显示系数
	double* tgCoef = m_pChannel2->getChannelHeader()->getCoef();

	//======取得当前滚动条的位置=======//
	CPoint oPoint = GetScrollPosition();
	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//侧视图，获取数据区域的高度和宽度(也就是侧视图显示区域的高度和宽度)
	int iWidthToDisplay = SPACE_LEFT + m_iWidth - oPoint.x;
	if (iWidthToDisplay <= 0)
		return;
	if (iWidthToDisplay > (oRect.Width() - SPACE_LEFT))
		iWidthToDisplay = oRect.Width() - SPACE_LEFT;

	//===================构造侧视图开始==================//
	//构造位图信息头(只构造显示区域的位图，超出部分不构造)
	BITMAPINFOHEADER oBIHSideView;
	memset(&oBIHSideView, 0, sizeof(oBIHSideView));
	oBIHSideView.biSize        = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
	oBIHSideView.biHeight      = m_iHeight;                   //位图的高度，以像素为单位
	oBIHSideView.biWidth       = iWidthToDisplay;             //位图的宽度，以像素为单位
	oBIHSideView.biPlanes      = 1;                           //位平面数，必须为1
	oBIHSideView.biBitCount    = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHSideView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHSideView.biSizeImage   = m_iHeight *
                                 iWidthToDisplay *
                                 4;                           //位图的大小，以字节为单位

	//分配像素空间，侧视图数据
	COLORREF* pPicSideView = new COLORREF[m_iHeight * iWidthToDisplay];

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
		for (int jj = 0; jj < m_iHeight; jj++)
		{
			short sColor = int(pData[jj] * tgCoef[jj]) / 2048 + 64;

			if (sColor < 0)
				sColor = 0;
			else if (sColor > 128)
				sColor = 128;

			COLORREF* pBuf = pPicSideView + iWidthToDisplay * (m_iHeight - jj - 1) + i - oPoint.x;
			*pBuf = m_Palette.getColorref()[sColor];
		}
	}
	//===================构造侧视图完成==================//

	CClientDC oDCTmp(this);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView, CBM_INIT, pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;    oDCImage.CreateCompatibleDC(&oDCTmp);    //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象
	oDC.BitBlt(SPACE_LEFT, SPACE_TOP * 2 + m_iHeight - oPoint.y, iWidthToDisplay - SPACE_LEFT, m_iHeight, &oDCImage, 0, 0, SRCCOPY); //显示
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

	return;
}

//将雷达数据绘制为BMP--俯视图
void RadarViewMain::CreateViewTop(CDC& oDC)
{
	//获取通道数目
	int iChannelCount = 0;
	m_pSwath->getChannelCount(iChannelCount);

	//======取得当前滚动条的位置=======//
	CPoint oPoint = GetScrollPosition();
	//当前的显示窗口大小
	CRect oRect;
	GetClientRect(&oRect);

	//俯视图数据显示的总高度和宽度
	int iDataDisplayWidth  = oRect.Width()  - SPACE_LEFT - SPACE_LEFT;

	//构造单个俯视图的位图信息头(俯视图长宽大小都一样，所以构造一个即可)
	BITMAPINFOHEADER oBIHTopView;
	memset(&oBIHTopView, 0, sizeof(oBIHTopView));
	oBIHTopView.biSize        = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
	oBIHTopView.biHeight      = ( iChannelCount - 1 ) * 3 + 1; //位图的高度，以像素为单位
	oBIHTopView.biWidth       = iDataDisplayWidth;           //位图的宽度，以像素为单位
	oBIHTopView.biPlanes      = 1;                           //位平面数，必须为1
	oBIHTopView.biBitCount    = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHTopView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHTopView.biSizeImage   = ( (iChannelCount - 1) * 3 + 1 ) *
		                        iDataDisplayWidth  * 4;      //位图的大小，以字节为单位

    //获取得到第一个Channel
	SwathChannel* pChannel = m_pSwath->getChannel(1);
	//计算需要显示俯视图的深度(cm)
	int iDeep = ( int ) ( RadarViewSetting::GetDeep() * pChannel->getChannelHeader()->getFrequency() * 2 / (pChannel->getChannelHeader()->getSoilVel() * 100) );
	if (iDeep > pChannel->getChannelHeader()->getSample())
		iDeep = pChannel->getChannelHeader()->getSample();


	//每个平面的颜色数据，分配颜色空间数据（各个平面重复使用）
	COLORREF* pPicTopView = new COLORREF[((iChannelCount - 1) * 3 + 1) * iDataDisplayWidth];

	//为每个俯视图平面分配空间，并且在分配的俯视图颜色空间上填充数据
	//x:平行道路水平向右
	//y:垂直道路水平方向

	//下面从channel中取出数据填充到Buff
	for (int y = 0; y < iChannelCount; y++)
	{
		//读取对应通道的系数
		SwathChannel* pChannel1 = m_pSwath->getChannel(y+1);
		double* tgCoef = pChannel1->getChannelHeader()->getCoef();

		//获取需要的Trace像素数据
		std::map<int, Trace16*> *pTraceData1 = pChannel1->getChannelBlob()->getTraceData();

		//获取需要的Trace像素数据
		for (int x = (int)oPoint.x; x < (int)(oPoint.x + iDataDisplayWidth); x++)
		{
			Trace16 *pData = (*pTraceData1)[x];
			if (NULL == pData)
				break;
			if (x > m_iWidth)
				break;

			short sColor = (short)(pData->getTrace()[iDeep] * tgCoef[iDeep]) / 2048 + 64;
			if (sColor < 0)
				sColor = 0;
			else if (sColor > 128)
				sColor = 128;

			//*( pPicTopView + y * iDataDisplayWidth + x - oPoint.x) = m_Palette.getColorref()[sColor];
			*(pPicTopView + 3 * y * iDataDisplayWidth + x - oPoint.x) = m_Palette.getColorref()[sColor];
		}

		//========================//
		if (y >= iChannelCount - 1)
			continue;

		//读取对应通道的系数
		SwathChannel* pChannel2 = m_pSwath->getChannel(y + 1 + 1);

		//显示对比度
		int iContrast = RadarViewSetting::GetContrast();
		//增益
		int iTimeGain = RadarViewSetting::GetTimegain();
		//使用用户设定的对比度和增益数据，设置显示系数
		pChannel2->getChannelHeader()->setCoef(iContrast, iTimeGain);
		double* tgCoef2 = pChannel2->getChannelHeader()->getCoef();

		//获取需要的Trace像素数据
		std::map<int, Trace16*> *pTraceData2 = pChannel2->getChannelBlob()->getTraceData();

		//插入構造的数据
		for (int x = oPoint.x; x < (int)(oPoint.x + iDataDisplayWidth); x++)
		{
			Trace16 *pData1 = (*pTraceData1)[x];
			if (NULL == pData1)
				break;
			Trace16 *pData2 = (*pTraceData2)[x];
			if (NULL == pData2)
				break;
			if (x > m_iWidth)
				break;

			short sColor1 = (short)(pData1->getTrace()[iDeep] * tgCoef[iDeep]) / 2048 + 32;
			if (sColor1 < 0)
				sColor1 = 0;
			else if (sColor1 > 128)
				sColor1 = 128;
			short sColor2 = (short)(pData2->getTrace()[iDeep] * tgCoef2[iDeep]) / 2048 + 64;
			if (sColor2 < 0)
				sColor2 = 0;
			else if (sColor2 > 128)
				sColor2 = 128;

			short sColor = (sColor1 + sColor2) / 2;

			*(pPicTopView + (3 * y + 1) * iDataDisplayWidth + x - oPoint.x) = m_Palette.getColorref()[sColor];
			*(pPicTopView + (3 * y + 2) * iDataDisplayWidth + x - oPoint.x) = m_Palette.getColorref()[sColor];
		}
	}

	TRACE( _T( "iDeep=%d\n" ) , iDeep );

	CClientDC oDCTmp(this);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHTopView, CBM_INIT, pPicTopView, (LPBITMAPINFO)&oBIHTopView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;    oDCImage.CreateCompatibleDC(&oDCTmp);    //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象

	//俯视图数据显示的开始位置
	int iDataDisplayY = SPACE_TOP + pChannel->getChannelHeader()->getSample() * 2 + SPACE_TOP_SIDE * 2;
	//绘制到输出DC
	oDC.StretchBlt(SPACE_LEFT        , iDataDisplayY - oPoint.y,
	               iDataDisplayWidth , iChannelCount * 5, &oDCImage,
	               0                 ,                 0, 
                   iDataDisplayWidth , iChannelCount    , SRCCOPY); //显示

	CString strOut;
	strOut.Format(_T("水平图深度%d(cm)"), RadarViewSetting::GetDeep());
	CRect rc = oRect;
	oDC.SetTextAlign(TA_LEFT);
	SetBkMode(oDCImage, TRANSPARENT);		    //设置界面上写字时，背景透明
	oDC.TextOut(SPACE_LEFT, iDataDisplayY-20, strOut);    //绘制垂直的比例单位

	oDCImage.DeleteDC();
	oBitmap.DeleteObject();
	DeleteObject(hBitmap);

	delete pPicTopView;
}

//绘制认为是可疑区域的标志
void RadarViewMain::CreateMarks(CDC& oDC)
{
	//当前的显示窗口大小
	CRect oRect;
	GetClientRect(&oRect);

	//======取得当前滚动条的位置=======//
	CPoint oPoint = GetScrollPosition();

	CPen oPen(PS_SOLID, 1, RGB(255, 0, 0)); //红色画笔
	CPen* pOldPen = oDC.SelectObject(&oPen);//保存原始的CPen，即黑色的CPen

	//绘制每个Mark
	std::vector<CRect*>::iterator it;
	for (it = m_lstMark.begin(); it != m_lstMark.end(); it++)
	{
		//释放目标空间
		CRect * pMark = *it;
		if (NULL == pMark)
			continue;

		//4个顶点
		CPoint oLeftTop;     //左上角
		CPoint oRightTop;    //右上角
		CPoint oLeftBottom;  //左下角
		CPoint oRightBottom; //右下角

		//如果左上角超出左边，则强制要求最左边就是左上角的左边；如果左上角超出右边，则强制要求最右边就是左上角的右边
		oLeftTop.x = ((pMark->left - oPoint.x) > SPACE_LEFT) ? (pMark->left - oPoint.x) : SPACE_LEFT;		oLeftTop.x = (oLeftTop.x > oRect.right - SPACE_LEFT) ? (oRect.right - SPACE_LEFT) : oLeftTop.x;
		oLeftTop.y = ((pMark->top - oPoint.y) > 0) ? (pMark->top - oPoint.y) : 0;							oLeftTop.y = (oLeftTop.y > (SPACE_TOP - oPoint.y + m_iHeight)) ? (SPACE_TOP - oPoint.y + m_iHeight) : oLeftTop.y;

		//如果右上角超出左边，则强制要求最左边就是右上角的左边；如果右上角超出右边，则强制要求最右边就是右上角的右边
		oRightTop.x = ((pMark->right - oPoint.x) > SPACE_LEFT) ? (pMark->right - oPoint.x) : SPACE_LEFT;	oRightTop.x = (oRightTop.x > oRect.right - SPACE_LEFT) ? (oRect.right - SPACE_LEFT) : oRightTop.x;
		oRightTop.y = ((pMark->top - oPoint.y) > 0) ? (pMark->top - oPoint.y) : 0;							oRightTop.y = (oRightTop.y > (SPACE_TOP - oPoint.y + m_iHeight)) ? (SPACE_TOP - oPoint.y + m_iHeight) : oRightTop.y;

		//如果左下角超出左边，则强制要求最左边就是左下角的左边；如果左下角超出右边，则强制要求最右边就是左下角的右边
		oLeftBottom.x = ((pMark->left - oPoint.x) > SPACE_LEFT) ? (pMark->left - oPoint.x) : SPACE_LEFT;	oLeftBottom.x = (oLeftBottom.x > oRect.right - SPACE_LEFT) ? (oRect.right - SPACE_LEFT) : oLeftBottom.x;
		oLeftBottom.y = ((pMark->bottom - oPoint.y) > 0) ? (pMark->bottom - oPoint.y) : 0;					oLeftBottom.y = (oLeftBottom.y > (SPACE_TOP - oPoint.y + m_iHeight)) ? (SPACE_TOP - oPoint.y + m_iHeight) : oLeftBottom.y;

		//如果右下角超出左边，则强制要求最左边就是右下角的左边；如果右下角超出右边，则强制要求最右边就是右下角的右边
		oRightBottom.x = ((pMark->right - oPoint.x) > SPACE_LEFT) ? (pMark->right - oPoint.x) : SPACE_LEFT;	oRightBottom.x = (oRightBottom.x > oRect.right - SPACE_LEFT) ? (oRect.right - SPACE_LEFT) : oRightBottom.x;
		oRightBottom.y = ((pMark->bottom - oPoint.y) > 0) ? (pMark->bottom - oPoint.y) : 0;					oRightBottom.y = (oRightBottom.y > (SPACE_TOP - oPoint.y + m_iHeight)) ? (SPACE_TOP - oPoint.y + m_iHeight) : oRightBottom.y;

		//左右移动，导致矩形为一条线的情况，不要绘制
		if (oLeftTop.x == oRightBottom.x)
			continue;
		//上下移动，导致矩形为一条线的情况，不要绘制
		if (oLeftTop.y == oRightBottom.y)
			continue;

		//在主视图第一视图先移动到左上角
		oDC.MoveTo(oLeftTop);
		//绘制四条边
		oDC.LineTo(oRightTop   );    //从左上角画到右上角
		oDC.LineTo(oRightBottom);    //从右上角画到右下角
		oDC.LineTo(oLeftBottom );    //从右下角画到左下角
		oDC.LineTo(oLeftTop    );    //从左下角画到左上角


		//在俯视图绘制方框
		int iDisplayYStart = SPACE_TOP + m_pChannel1->getChannelHeader()->getSample() * 2 + SPACE_TOP_SIDE * 2;
		int iChannelCount = 0; m_pSwath->getChannelCount(iChannelCount);
		int iDisplayYEnd   = iDisplayYStart + iChannelCount * 5;
		oLeftTop.SetPoint(oLeftTop.x, iDisplayYStart);
		oRightTop.SetPoint(oRightTop.x, iDisplayYStart);
		oLeftBottom.SetPoint(oLeftBottom.x, iDisplayYEnd);
		oRightBottom.SetPoint(oRightBottom.x, iDisplayYEnd);

		oDC.MoveTo(oLeftTop);
		//绘制四条边
		oDC.LineTo(oRightTop);    //从左上角画到右上角
		oDC.LineTo(oRightBottom);    //从右上角画到右下角
		oDC.LineTo(oLeftBottom);    //从右下角画到左下角
		oDC.LineTo(oLeftTop);    //从左下角画到左上角
	}

	//如果鼠标按下，则需要绘制正在画的框
	if (m_bMouseDownFlag)
	{
		//先移动到左上角
		oDC.MoveTo(m_PointStart.x - oPoint.x, m_PointStart.y - oPoint.y);
		//绘制四条边
		oDC.LineTo(m_PointEnd.x - oPoint.x, m_PointStart.y - oPoint.y);
		oDC.LineTo(m_PointEnd.x - oPoint.x, m_PointEnd.y - oPoint.y);
		oDC.LineTo(m_PointStart.x - oPoint.x, m_PointEnd.y - oPoint.y);
		oDC.LineTo(m_PointStart.x - oPoint.x, m_PointStart.y - oPoint.y);
	}

	oDC.SelectObject(pOldPen);//恢复原始的CPen
	oPen.DeleteObject();
}

//设置系数参数
void RadarViewMain::UpdateCoef()
{
	//获取得到第一个Channel
	SwathChannel* pChannel = m_pSwath->getChannel(1);
	//计算需要显示的层数---计算总深度(m)
	double dDeep = 1 / pChannel->getChannelHeader()->getFrequency() * pChannel->getChannelHeader()->getSoilVel() / 2 * pChannel->getChannelHeader()->getSample();
	//计算需要显示的层数---得到平面之间的深度间隔(cm)
	int iDeepInterval = RadarViewSetting::GetDeep();

	//显示对比度
	int iContrast = RadarViewSetting::GetContrast();
	//增益
	int iTimeGain = RadarViewSetting::GetTimegain();

	if (NULL == m_pSwath)
		return;

	int iCount = 0;
	m_pSwath->getChannelCount(iCount);
	for (int i = 1; i <= iCount; i++)
	{
		SwathChannel* pChannel = m_pSwath->getChannel(i);
		if (NULL == pChannel)
			continue;

		//初始化显示系数
		pChannel->getChannelHeader()->setCoef(iContrast, iTimeGain);
	}
}
//初始化按键状态和Marks
void RadarViewMain::initMarks()
{
	//清除上次视图标志的可疑区域
	std::vector<CRect*>::iterator it;
	for (it = m_lstMark.begin(); it != m_lstMark.end(); it++)
	{
		//释放目标空间
		CRect * pMark = *it;
		delete pMark;
	}
	m_lstMark.clear();

	//鼠标按下状态也释放掉
	m_PointStart.x = m_PointStart.y = 0;
	m_PointEnd.x = m_PointEnd.y = 0;
	m_bMouseDownFlag = false;
}

//窗口大小变化时触发
void RadarViewMain::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	//窗口变化时刷新界面
	Invalidate(TRUE);

	TRACE( "" );
}
void RadarViewMain::OnDraw(CDC* /*pDC*/)
{
	if (NULL == m_pSwath)
		return;
    if (NULL == m_pProject)
        return;

	//当前的显示窗口
	CRect oRect;
	GetClientRect(&oRect);

	//获取主DC
	CDC* pDC = this->GetDC();

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
	CreateViewSide(oMemDC);    //绘制侧视图
	CreateViewTop( oMemDC);    //绘制俯视图
	CreateMarks(   oMemDC);    //绘制可疑标志

	//将内存DC copy到主DC
	pDC->BitBlt(0, 0, oRect.Width(), oRect.Height(), &oMemDC, 0, 0, SRCCOPY);
	oMemBitmap.DeleteObject();
	oMemDC.DeleteDC();
}

void RadarViewMain::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//水平滚动时刷新界面
	Invalidate(TRUE);

	//CPoint oPoint = GetScrollPosition();

	//下面通知辅视图滚动
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	//获取到左视图
	RadarViewAssistant *pView = pFrame->GetViewLeft();
	if (NULL != pView)
		pView->OnHScroll(nSBCode, nPos, pScrollBar);
	//pView->SetHScroll(oPoint.x);

	//获取到右视图
	pView = pFrame->GetViewRight();
	if (NULL != pView)
		pView->OnHScroll(nSBCode, nPos, pScrollBar);
	//pView->SetHScroll(oPoint.x);

	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}
void RadarViewMain::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//垂直滚动时刷新界面
	Invalidate(TRUE);

	CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
}
BOOL RadarViewMain::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	//滚动时刷新界面
	Invalidate(FALSE);

	return CFormView::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL RadarViewMain::PreTranslateMessage(MSG* pMsg)
{
	//如果是键盘右键头按下，那么滚动条右移(待处理)
	if (pMsg->message == WM_KEYDOWN)
	{
		if ( pMsg->wParam == VK_RIGHT )
		{
			//GetScrollBarCtrl(SB_HORZ)->SetScrollPos(200);
			return  TRUE;
		}
	}

	//
	return CFormView::PreTranslateMessage(pMsg);
}

//双击鼠标左键，弹出病害卡窗口
void RadarViewMain::OnLButtonDblClk(UINT nFlags, CPoint point)
{
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
	CPoint oScrollPos = GetScrollPosition();
	//鼠标双击时的x位置(相对于完整图像的开头到当前的X坐标)
	int iCurrentX = oScrollPos.x + point.x - SPACE_LEFT;
	//鼠标双击时的y位置(相对于完整图像的开头到当前的Y坐标)
	int iCurrentY = oScrollPos.y + point.y - SPACE_TOP;

	//位置超出右边范围
	if (iCurrentX > ( oScrollPos.x + oRect.Width() - 2 * SPACE_LEFT ) )
		return;
	//位置超出左边范围
	if (oScrollPos.x > iCurrentX)
		return;

	//位置超出下边范围
	if (iCurrentY > m_pChannel1->getChannelHeader()->getSample())
		return;
	//位置超出上边范围
	if (0 > iCurrentY)
		return;

	//目标空间
	CRect * pMark = NULL;

	//取得已经标注的可疑区域
	std::vector<CRect*>::iterator it;
	for (it = m_lstMark.begin(); it != m_lstMark.end(); it++)
	{
		pMark = *it;
		//如果点击的位置在框选区域内部，则得到对应的Mark
		if ((pMark->left <= ( iCurrentX + SPACE_LEFT ) ) && ( ( iCurrentX + SPACE_LEFT ) <= pMark->right) && 
			(pMark->top  <= ( iCurrentY + SPACE_TOP  ) ) && ( ( iCurrentY + SPACE_TOP  ) <= pMark->bottom))
		{
			break;
		}

		pMark = NULL;
	}
	if (NULL == pMark)
	{
		MessageBox( _T("此处未标识缺陷，不需要产生病害卡") );
		CFormView::OnLButtonDblClk(nFlags, point);
		return;
	}

	//弹出病害卡窗口（在对话框上需要有一些参数的输入）,生成指定位置的病害卡
	DialogCard oCard;
	oCard.SetParam(m_pProject, m_pSwath, m_pChannel1, *pMark);
	oCard.DoModal();

	CFormView::OnLButtonDblClk(nFlags, point);
}

//鼠标右键按下，删除选定的标志
void RadarViewMain::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPoint oScrollPos = GetScrollPosition();

	CPoint oCurrent;
	oCurrent.x = point.x + oScrollPos.x;
	oCurrent.y = point.y + oScrollPos.y;

	std::vector<CRect*>::iterator it;
	for (it = m_lstMark.begin(); it != m_lstMark.end(); it++)
	{
		//释放目标空间
		CRect * pMark = *it;

		//如果点击的位置在框选区域内部，则删除对应的Mark
		if ((pMark->left <= oCurrent.x) && (oCurrent.x <= pMark->right) && (pMark->top <= oCurrent.y) && (oCurrent.y <= pMark->bottom))
		{
			delete pMark;

			//删除选定的对象
			m_lstMark.erase(it);

			//更新界面显示
			Invalidate(TRUE);
			break;
		}
	}

	CFormView::OnLButtonDown(nFlags, point);
}

//鼠标左键按下，开始框定一个标志
void RadarViewMain::OnLButtonDown(UINT nFlags, CPoint point)
{
	//如果第一视图没有通道图像，就不能操作
	if (NULL == m_pChannel1)
		return;

	//当前的显示窗口大小
	CRect oRect;
	GetClientRect(&oRect);

	//滚动条位置
	CPoint oPoint = GetScrollPosition();

	//超出左边边界，或超出右边边界，或超出上边边界，或超出下边边界
	if ((point.x < SPACE_LEFT) ||
		(point.x > (oRect.right - SPACE_LEFT)) ||
		(point.y < (SPACE_TOP - oPoint.y)) ||
		(point.y > (SPACE_TOP - oPoint.y + m_iHeight)))
		return;

	//把鼠标左键按下时记录位置
	m_PointStart.x = point.x + oPoint.x;
	m_PointStart.y = point.y + oPoint.y;
	m_PointEnd.x = point.x + oPoint.x;
	m_PointEnd.y = point.y + oPoint.y;

	//更新鼠标按下标志
	m_bMouseDownFlag = true;

	CFormView::OnLButtonDown(nFlags, point);
}

//鼠标左键抬起，完成框定一个标志
void RadarViewMain::OnLButtonUp(UINT nFlags, CPoint point)
{
	//如果第一视图没有通道图像，就不能操作
	if (NULL == m_pChannel1)
		return;
	//如果点击的位置在第一视图之外不操作
	if (!m_bMouseDownFlag)
		return;

	//当前的显示窗口大小
	CRect oRect;
	GetClientRect(&oRect);

	//滚动条位置
	CPoint oPoint = GetScrollPosition();
	//鼠标抬起时记录结束位置
	m_PointEnd.x = point.x + oPoint.x;
	m_PointEnd.y = point.y + oPoint.y;

	//恢复标志为鼠标释放状态
	m_bMouseDownFlag = false;

	//超出左边边界，或超出右边边界，或超出上边边界，或超出下边边界
	if ((point.x < SPACE_LEFT) ||
		(point.x > (oRect.right - SPACE_LEFT)) ||
		(point.y < (SPACE_TOP - oPoint.y)) ||
		(point.y > (SPACE_TOP - oPoint.y + m_iHeight)))
	{
		//更新界面显示
		Invalidate(TRUE);
		CFormView::OnLButtonUp(nFlags, point);
		return;
	}

	//如果框选的区域足够大，则作为标志位记录，否则丢弃，这里不提示，可能是用户误操作
	if ((abs(m_PointStart.x - m_PointEnd.x) <= 3) || (abs(m_PointStart.y - m_PointEnd.y) <= 3))
	{
		//更新界面显示
		Invalidate(TRUE);
		CFormView::OnLButtonUp(nFlags, point);
		return;
	}
	if((abs(m_PointStart.x - m_PointEnd.x) <= 6) || (abs(m_PointStart.y - m_PointEnd.y) <= 6))
	{
		MessageBox(_T("框选的位置太小了。"));

		//更新界面显示
		Invalidate(TRUE);
		CFormView::OnLButtonUp(nFlags, point);
		return;
	}

	CRect * pNewMark = new CRect();
	pNewMark->left   = m_PointStart.x >= m_PointEnd.x ? m_PointEnd.x : m_PointStart.x;
	pNewMark->top    = m_PointStart.y >= m_PointEnd.y ? m_PointEnd.y : m_PointStart.y;
	pNewMark->right  = m_PointStart.x <= m_PointEnd.x ? m_PointEnd.x : m_PointStart.x;
	pNewMark->bottom = m_PointStart.y <= m_PointEnd.y ? m_PointEnd.y : m_PointStart.y;

	//将新框选的区域加入列表
	m_lstMark.push_back(pNewMark);

	//更新界面显示
	Invalidate(TRUE);

	CFormView::OnLButtonUp(nFlags, point);
}

//鼠标移动
void RadarViewMain::OnMouseMove(UINT nFlags, CPoint point)
{
	//如果第一视图没有通道图像，就不能操作
	if (NULL == m_pChannel1)
		return;

	//如果鼠标按下时移动，需要触发界面重新绘制
	if (m_bMouseDownFlag)
	{
		CPoint oPoint = GetScrollPosition();
		//鼠标移动时，跟新最新的位置点
		m_PointEnd.x = point.x + oPoint.x;
		m_PointEnd.y = point.y + oPoint.y;

		//更新界面显示
		Invalidate(TRUE);
	}

	CFormView::OnMouseMove(nFlags, point);
}

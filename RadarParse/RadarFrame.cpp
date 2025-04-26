
// RadarParseMainFrm.cpp: RadarFrame 类的实现
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


#include "RadarApp.h"
#include "RadarFrame.h"
#include "RadarViewMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RIGHT_VIEW_WIDTH 500

// RadarFrame

IMPLEMENT_DYNCREATE(RadarFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(RadarFrame, CFrameWndEx)
    ON_WM_CREATE()
    ON_COMMAND(ID_VIEW_CUSTOMIZE, &RadarFrame::OnViewCustomize)

    ON_WM_SETTINGCHANGE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // 状态行指示器
    ID_INDICATOR_PROJECTPATH,
    ID_INDICATOR_SWATH,
    ID_INDICATOR_CHANNEL,
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL
};

// RadarFrame 构造/析构
RadarFrame::RadarFrame() noexcept
{
    m_bInitSplitter = FALSE;
}

RadarFrame::~RadarFrame()
{
}

int RadarFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndMenuBar.Create(this))
    {
        TRACE0("未能创建菜单栏\n");
        return -1;      // 未能创建
    }
    m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

    // 防止菜单栏在激活时获得焦点
    CMFCPopupMenu::SetForceMenuFocus(FALSE);

    if (!m_wndStatusBar.Create(this))
    {
        TRACE0("未能创建状态栏\n");
        return -1;      // 未能创建
    }
    m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
    m_wndStatusBar.SetPaneInfo(0, ID_SEPARATOR            , SBPS_NORMAL, 100);
    m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_PROJECTPATH, SBPS_NORMAL, 450);
    m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_SWATH      , SBPS_NORMAL, 180);
    m_wndStatusBar.SetPaneInfo(3, ID_INDICATOR_CHANNEL    , SBPS_NORMAL, 120);

    // TODO: 如果您不希望工具栏和菜单栏可停靠，请删除这五行
    m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndMenuBar);

    // 启用 Visual Studio 2005 样式停靠窗口行为
    CDockingManager::SetDockingMode(DT_SMART);
    // 启用 Visual Studio 2005 样式停靠窗口自动隐藏行为
    EnableAutoHidePanes(CBRS_ALIGN_ANY);

    // 加载菜单项图像(不在任何标准工具栏上): 
    CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

    // 创建停靠窗口
    if (!CreateDockingWindows())
    {
        TRACE0("未能创建停靠窗口\n");
        return -1;
    }

    //显示左边的树
    m_wndSwathListView.EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndSwathListView);
    m_wndSwathListView.ShowPane(TRUE, FALSE, TRUE);

    //输出窗口的显示
    m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndOutput);
    m_wndOutput.ShowPane(TRUE, TRUE, TRUE);

    // 启用快速(按住 Alt 拖动)工具栏自定义
    CMFCToolBar::EnableQuickCustomization();

    // 启用菜单个性化(最近使用的命令)
    // TODO: 定义您自己的基本命令，确保每个下拉菜单至少有一个基本命令。
    CList<UINT, UINT> lstBasicCommands;

    lstBasicCommands.AddTail(ID_FILE_OPEN);
    lstBasicCommands.AddTail(ID_APP_EXIT);
    lstBasicCommands.AddTail(ID_APP_ABOUT);
    lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);

    //CMFCToolBar::SetBasicCommands(lstBasicCommands);

    return 0;
}

BOOL RadarFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWndEx::PreCreateWindow(cs) )
        return FALSE;
    // TODO: 在此处通过修改
    //  CREATESTRUCT cs 来修改窗口类或样式
    cs.style |= WS_MAXIMIZE;

    return TRUE;
}

BOOL RadarFrame::CreateDockingWindows()
{
    BOOL bNameValid;

    // 创建文件视图
    CString strSwathListView;
    bNameValid = strSwathListView.LoadString(IDS_SWATH_VIEW);
    ASSERT(bNameValid);
    if (!m_wndSwathListView.Create(strSwathListView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_SWATHLIST, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
    {
        TRACE0("未能创建“文件视图”窗口\n");
        return FALSE; // 未能创建
    }

    // 创建输出窗口
    CString strOutputWnd;
    bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
    ASSERT(bNameValid);
    if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
    {
        TRACE0("未能创建输出窗口\n");
        return FALSE; // 未能创建
    }

    SetDockingWindowIcons(theApp.m_bHiColorIcons);
    return TRUE;
}

void RadarFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
    HICON hSwathListViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_SWATH_VIEW_HC : IDI_SWATH_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_wndSwathListView.SetIcon(hSwathListViewIcon, FALSE);

    HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_wndOutput.SetIcon(hOutputBarIcon, FALSE);
}

// RadarFrame 诊断

#ifdef _DEBUG
void RadarFrame::AssertValid() const
{
    CFrameWndEx::AssertValid();
}

void RadarFrame::Dump(CDumpContext& dc) const
{
    CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

// RadarFrame 消息处理程序
void RadarFrame::OnViewCustomize()
{
    CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* 扫描菜单*/);
    pDlgCust->Create();
}
BOOL RadarFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
    // 基类将执行真正的工作

    if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
    {
        return FALSE;
    }

    return TRUE;
}
void RadarFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    CFrameWndEx::OnSettingChange(uFlags, lpszSection);
    m_wndOutput.UpdateFonts();
}
BOOL RadarFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
    //calculate client size 
    CRect cr;
    GetClientRect(&cr);

    if (!m_mainSplitter.CreateStatic(this, 1, 2))
    {
        MessageBox(_T("Error setting up splitter frames!"), _T("Init Error!"), MB_OK | MB_ICONERROR);
        return FALSE;
    }

    if (!m_mainSplitter.CreateView(0, 0, RUNTIME_CLASS(RadarViewMain), CSize(cr.Width() - RIGHT_VIEW_WIDTH, cr.Height()), pContext))
    {
        MessageBox(_T("Error setting up splitter frames!"), _T("Init Error!"), MB_OK | MB_ICONERROR);
        return FALSE;
    }
    if (!m_mainSplitter.CreateView(0, 1, RUNTIME_CLASS(RadarViewSetting), CSize(RIGHT_VIEW_WIDTH, cr.Height()), pContext))
    {
        MessageBox(_T("Error setting up splitter frames!"), _T("Init Error!"), MB_OK | MB_ICONERROR);
        return FALSE;
    }
    m_mainSplitter.SetColumnInfo(0, cr.Width() - RIGHT_VIEW_WIDTH, 10);
    //m_mainSplitter.SetColumnInfo(1,  RIGHT_VIEW_WIDTH, 10);

    //分割窗口已经创建
    m_bInitSplitter = TRUE;

    //主显示窗口
    m_pViewMain = (RadarViewMain *)m_mainSplitter.GetPane(0, 0);
    //设置窗口
    m_pSettingView = (RadarViewSetting*)m_mainSplitter.GetPane(0, 1);

    return TRUE;
    //return CFrameWndEx::OnCreateClient(lpcs, pContext);
}
void RadarFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWndEx::OnSize(nType, cx, cy);


    CRect cr;
    GetWindowRect(cr);
    int width = cr.Width();
    //
    if (m_bInitSplitter && nType != SIZE_MINIMIZED)
    {
        if (width < RIGHT_VIEW_WIDTH) width = RIGHT_VIEW_WIDTH;
        m_mainSplitter.SetRowInfo(0, cy, 0);
        m_mainSplitter.SetColumnInfo(0, width - RIGHT_VIEW_WIDTH, 50);
        m_mainSplitter.SetColumnInfo(1, RIGHT_VIEW_WIDTH, 50);
        m_mainSplitter.RecalcLayout();
    }

}

void RadarFrame::setStatusBar(CString strSwathName, CString strChannel)
{
    //在状态栏上显示Swath信息
    m_wndStatusBar.SetPaneText(2, strSwathName, 1);
    //在状态栏上显示Channel信息
    m_wndStatusBar.SetPaneText(3, strChannel, 1);
}

//打开工程
void RadarFrame::ProjectOpen( Project *pProject )
{
    if (nullptr == pProject)
        return;

    //记录工程对象
    m_pProject = pProject;

    char *ProjectPath = pProject->getProjectPath();
    char szOut[512] = { 0 };
    sprintf(szOut , "工程路径:%s", ProjectPath);
    CString strOut = ( CString )szOut;
    //在状态栏上显示工程目录
    m_wndStatusBar.SetPaneText(1, strOut, 1);

    //通知状态栏
    setStatusBar(_T(""), _T(""));

    //显示左边的Swath树
    m_wndSwathListView.SetProject(pProject);
}

//导出工程
void RadarFrame::ProjectExport(Project *pProject)
{
    if (nullptr == pProject)
        return;

    //判断雷达数据转为图片的目录是否存在，如果不存在就创建
    CString strRadarPicture;
    strRadarPicture.Format("%s\\RadarPic", pProject->getProjectPath());
    if (!PathIsDirectory(strRadarPicture))
    {
        CreateDirectory(strRadarPicture, NULL);
    }

    //获取所有SwathName
    std::vector<char *> lstSwath;
    int iResult = pProject->getAllSwathName(lstSwath);
    if (0 != iResult)
    {
        lstSwath.clear();
        return;
    }
	/*
    for (int i = 0; i < (int)lstSwath.size(); i++)
    {
        //获取通道名称，并且在列表中插入SwathName
        CString strSwathName = (CString)lstSwath[i];

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
            sprintf(szChannel, "A%02d", j);
            CString strChannel = (CString)szChannel;

            //附带通道数据
            SwathChannel * pChannel = pSwath->getChannel(j);
			pChannel->getChannelBlob()->loadTrace();

            CString strPathFile;
            strPathFile.Format("%s\\%s_%s", strRadarPicture.GetBuffer(strRadarPicture.GetLength()), strSwathName.GetBuffer(strSwathName.GetLength()), szChannel);
            //将通道数据存储为图片文件
            SaveAsJpg(strPathFile,pChannel);

			//释放数据空间
			pChannel->getChannelBlob()->unloadTrace();
        }
    }
	*/


	for (int i = 0; i < (int)lstSwath.size(); i++)
	{
		//获取通道名称，并且在列表中插入SwathName
		CString strSwathName = (CString)lstSwath[i];

		//获取Swath详细信息（channel）
		Swath *pSwath = pProject->getSwath(lstSwath[i]);
		int iCount = 0;
		iResult = pSwath->getChannelCount(iCount);
		if (0 != iResult)
			continue;

		SwathChannel * pChannel[32] = { NULL };
		int j = 1;
		//循环将通道插入Swath项目下面
		for (j = 1; j <= iCount; j++)
		{
			//附带通道数据
			pChannel[j] = pSwath->getChannel(j);
			pChannel[j]->getChannelBlob()->loadTrace();
		}

		CString strPathFile;
		strPathFile.Format("%s\\%s", strRadarPicture.GetBuffer(strRadarPicture.GetLength()), strSwathName.GetBuffer(strSwathName.GetLength()));
		//将通道数据存储为图片文件
		SaveAsJpgs(strPathFile, pChannel, iCount);

		for (j = 1; j <= iCount; j++)
		{
			//释放数据空间
			pChannel[j]->getChannelBlob()->unloadTrace();
		}
	}


    //清除临时数据
    lstSwath.clear();
	MessageBox("处理完成！");
}

//主视图第一视图
void RadarFrame::SetChannelViewMainOne(Swath* pSwath, SwathChannel* pChannel)
{
    if (NULL == m_pViewMain)
        return;

    m_pViewMain->SetProject(m_pProject);
    m_pViewMain->SetChannelViewOne(pSwath, pChannel);
}
//主视图第二视图
void RadarFrame::SetChannelViewMainTwo(Swath* pSwath, SwathChannel* pChannel)
{
    if (NULL == m_pViewMain)
        return;

    m_pViewMain->SetProject(m_pProject);
    m_pViewMain->SetChannelViewTwo(pSwath, pChannel);
}

//左视图第一视图
void RadarFrame::SetChannelViewLeftOne(Swath* pSwath, SwathChannel* pChannel)
{
    if (NULL == m_pViewLeft)
        return;

    m_pViewLeft->SetProject(m_pProject);
    m_pViewLeft->SetChannelViewOne(pSwath, pChannel);
}
//左视图第二视图
void RadarFrame::SetChannelViewLeftTwo(Swath* pSwath, SwathChannel* pChannel)
{
    if (NULL == m_pViewLeft)
        return;

    m_pViewLeft->SetProject(m_pProject);
    m_pViewLeft->SetChannelViewTwo(pSwath, pChannel);
}
//左视图第三视图
void RadarFrame::SetChannelViewLeftThree(Swath* pSwath, SwathChannel* pChannel)
{
    if (NULL == m_pViewLeft)
        return;

    m_pViewLeft->SetProject(m_pProject);
    m_pViewLeft->SetChannelViewThree(pSwath, pChannel);
}

//右视图第一视图
void RadarFrame::SetChannelViewRightOne(Swath* pSwath, SwathChannel* pChannel)
{
    if (NULL == m_pViewRight)
        return;

    m_pViewRight->SetProject(m_pProject);
    m_pViewRight->SetChannelViewOne(pSwath, pChannel);
}
//右视图第二视图
void RadarFrame::SetChannelViewRightTwo(Swath* pSwath, SwathChannel* pChannel)
{
    if (NULL == m_pViewRight)
        return;

    m_pViewRight->SetProject(m_pProject);
    m_pViewRight->SetChannelViewTwo(pSwath, pChannel);
}
//右视图第三视图
void RadarFrame::SetChannelViewRightThree(Swath* pSwath, SwathChannel* pChannel)
{
    if (NULL == m_pViewRight)
        return;

    m_pViewRight->SetProject(m_pProject);
    m_pViewRight->SetChannelViewThree(pSwath, pChannel);
}

//打开左视图
void RadarFrame::OpenViewLeft()
{
    if (NULL == m_pViewLeft)
    {
        m_pViewLeft = new RadarViewAssistant();
        m_pViewLeft->Create(IDD_VIEW_ASSISTANT, this);
        m_pViewLeft->SetWindowText(_T("左辅图"));
    }
    m_pViewLeft->ShowWindow(SW_SHOW);
}
//打开右视图
void RadarFrame::OpenViewRight()
{
    if (NULL == m_pViewRight)
    {
        m_pViewRight = new RadarViewAssistant();
        m_pViewRight->Create(IDD_VIEW_ASSISTANT, this);
        m_pViewRight->SetWindowText(_T("右辅图"));
    }
    m_pViewRight->ShowWindow(SW_SHOW);
}


//获取主视图的View
RadarViewMain *RadarFrame::GetViewMain()
{
    return m_pViewMain;
}
//获取左视图的View
RadarViewAssistant *RadarFrame::GetViewLeft()
{
    return m_pViewLeft;  //左视图
}
//获取右视图的View
RadarViewAssistant *RadarFrame::GetViewRight()
{
    return m_pViewRight;  //右视图
}

//更新所有界面显示
void RadarFrame::UpdateAllView(int iType)
{
    if ((VIEW_TYPE_MAIN == iType) && (NULL != m_pViewMain))
    {
        m_pViewMain->Invalidate(TRUE);
        return;
    }
    if ((VIEW_TYPE_LEFT == iType) && (NULL != m_pViewLeft))
    {
        m_pViewLeft->Invalidate(TRUE);
        return;
    }
    if ((VIEW_TYPE_RIGHT == iType) && (NULL != m_pViewRight))
    {
        m_pViewRight->Invalidate(TRUE);
        return;
    }

    if( NULL != m_pViewMain )
        m_pViewMain->Invalidate(TRUE);

    if (NULL != m_pViewLeft)
        m_pViewLeft->Invalidate(TRUE);

    if (NULL != m_pViewRight)
        m_pViewRight->Invalidate(TRUE);
}

//设置主窗口Title
void RadarFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    SetWindowText( _T("雷达数据解析-主控视图") );
    //CFrameWndEx::OnUpdateFrameTitle(bAddToTitle);
}

//打开测线树
void RadarFrame::OpenSwath()
{
    m_wndSwathListView.ShowPane(TRUE, FALSE, TRUE);
}

//将指定的通道数据存储为图片
void RadarFrame::SaveAsJpg( CString strPathFile, SwathChannel * pChannel)
{
    if (!pChannel)
        return;

    int iHeight = pChannel->getChannelHeader()->getSample();
    int iWidth  = pChannel->getChannelHeader()->getTraceCount();

    //===================构造一个测线的完整视图开始==================//
    //构造位图信息头(只构造显示区域的位图，超出部分不构造)
    BITMAPINFOHEADER oBIHSideView;
    memset(&oBIHSideView, 0, sizeof(oBIHSideView));
    oBIHSideView.biSize   = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
    oBIHSideView.biHeight = iHeight;                     //位图的高度，以像素为单位
    oBIHSideView.biWidth  = iWidth;                      //位图的宽度，以像素为单位
    oBIHSideView.biPlanes = 1;                           //位平面数，必须为1
    oBIHSideView.biBitCount = 32;                        //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
    oBIHSideView.biCompression = BI_RGB;                 //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
    oBIHSideView.biSizeImage = iHeight * iWidth * 4;     //位图的大小，以字节为单位

	//显示对比度
	int iContrast = RadarViewSetting::GetContrast();
	//增益
	int iTimeGain = RadarViewSetting::GetTimegain();
	//设置显示系数
	pChannel->getChannelHeader()->setCoef(iContrast, iTimeGain);

    //读取显示系数
    double* tgCoef = pChannel->getChannelHeader()->getCoef();
	//调色板
	PalSET palette;
    //分配像素空间，侧视图数据
    COLORREF* pPicSideView = new COLORREF[iHeight * iWidth];
    //获取需要的Trace像素数据
    std::map<int, Trace16*>* pTraceData = pChannel->getChannelBlob()->getTraceData();

    int iTraceCount = pChannel->getChannelBlob()->getTraceCount();

    //循环将像素数据转化为BMP(一列一列处理)
    for (int i = 0; i < iWidth; i++)
    {
        Trace16* pTraceTmp = (*pTraceData)[i];
        if (NULL == pTraceTmp)
            break;
        if (i >= iTraceCount)
            break;

        short* pData = pTraceTmp->getTrace();
        for (int jj = 0; jj < iHeight; jj++)
        {
            short sColor = int(pData[jj] * tgCoef[jj]) / 2048 + 32;

            if (sColor < 0)
                sColor = 0;
            else if (sColor > 63)
                sColor = 63;

            COLORREF* pBuf = pPicSideView + iWidth * (iHeight - jj -1) + i;
            *pBuf = palette.getColorref()[sColor];
        }
    }
	//===================构造测线的完整视图完成==================//

	//CClientDC oDCTmp(this);                                 //m_hwnd 创建客户区绘制内存
	CClientDC oDCTmp(NULL);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView, CBM_INIT, pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;    oDCImage.CreateCompatibleDC(&oDCTmp);    //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象


	//分片保存为对应的图片，每个图片的宽度最大为1280
	int iPices = (iWidth % 640) ? (iWidth / 640 + 1) : (iWidth / 640);
	//分片存为固定的长度
	for (int i = 1; i <= iPices; i++)
	{
		int iWithTmp = (i + 1) * 640 - (i - 1) * 640;
		int iStartX  = (i - 1) * 640;
		if((i + 1) * 640 > iWidth)
			iWithTmp = iWidth - (i - 1) * 640;

		CString strTmp;
		strTmp.Format("%s_%03d.jpg", strPathFile.GetBuffer(strPathFile.GetLength()), i);
		//测试保存为图片
		CImage image;
		image.Create(iWithTmp, iHeight, 32);
		BitBlt(image.GetDC(), 0, 0, iWithTmp, iHeight, oDCImage.m_hDC, iStartX, 0, SRCCOPY);
		HRESULT hResult = image.Save(strTmp);
		image.ReleaseDC();

		if ((i + 1) * 640 > iWidth)
			break;
	}

	oBitmap.DeleteObject();
	DeleteObject(hBitmap);

	//释放BMP像素空间
	delete[] pPicSideView;
}

#define PIC_SPACE_LEFT     150
#define PIC_SPACE_RIGHT    50
#define PIC_SPACE_TOP      200
#define PIC_SPACE_INTERVAL 20

void RadarFrame::SaveAsJpgs(CString strPathFile, SwathChannel * pChannel[32], int channelCount)
{
	//显示对比度
	int iContrast = RadarViewSetting::GetContrast();
	//增益
	int iTimeGain = RadarViewSetting::GetTimegain();

	//取得一个测线的最宽和最高
	int iMaxWith   = 0;
	int iMaxHeight = 0;
	for (int i = 1; i <= channelCount; i++)
	{
		if (NULL == pChannel[i])
			continue;

		int iHeight = pChannel[i]->getChannelHeader()->getSample();
		int iWidth  = pChannel[i]->getChannelHeader()->getTraceCount();

		if (iHeight > iMaxHeight)
			iMaxHeight = iHeight;
		if (iWidth > iMaxWith)
			iMaxWith = iWidth;
	}

	//为所有测线创建位图
	CString strTmp;
	strTmp.Format("%s_%03d.jpg", strPathFile.GetBuffer(strPathFile.GetLength()), 111);
	//测试保存为图片
	CImage image;
	//图片左边预留100画标尺，右边预留50空白
	//通道最上面空120画标尺，通道之间预留10空白
	image.Create(iMaxWith + PIC_SPACE_LEFT + PIC_SPACE_RIGHT, (iMaxHeight + PIC_SPACE_INTERVAL) * channelCount + PIC_SPACE_TOP, 32);


	//构造位图信息头(只构造显示区域的位图，超出部分不构造)
	BITMAPINFOHEADER oBIHSideView[20];
	//分配像素空间，侧视图数据
	COLORREF* pPicSideView[20] = { NULL };

	//所有通道数据拼接为一个图片
	for (int i = 1; i <= channelCount; i++)
	{
		if (NULL == pChannel[i])
			continue;

		int iHeight = pChannel[i]->getChannelHeader()->getSample();
		int iWidth  = pChannel[i]->getChannelHeader()->getTraceCount();

		//设置显示系数
		pChannel[i]->getChannelHeader()->setCoef(iContrast, iTimeGain);
		//读取显示系数
		double* tgCoef = pChannel[i]->getChannelHeader()->getCoef();

		memset(&oBIHSideView[i], 0, sizeof(oBIHSideView[i]));
		oBIHSideView[i].biSize     = sizeof(BITMAPINFOHEADER);     //本结构所占用字节数40字节
		oBIHSideView[i].biHeight   = iHeight;                      //位图的高度，以像素为单位
		oBIHSideView[i].biWidth    = iWidth;                       //位图的宽度，以像素为单位
		oBIHSideView[i].biPlanes   = 1;                            //位平面数，必须为1
		oBIHSideView[i].biBitCount = 32;                           //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
		oBIHSideView[i].biCompression = BI_RGB;                    //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
		oBIHSideView[i].biSizeImage = iHeight * iWidth * 4;        //位图的大小，以字节为单位

		//调色板
		PalSET palette;
		//分配像素空间，侧视图数据
		pPicSideView[i] = new COLORREF[iHeight * iWidth];
		//获取需要的Trace像素数据
		std::map<int, Trace16*>* pTraceData = pChannel[i]->getChannelBlob()->getTraceData();

		//循环将像素数据转化为BMP(一列一列处理)
		for (int j = 0; j < iWidth; j++)
		{
			Trace16* pTraceTmp = (*pTraceData)[j];
			if (NULL == pTraceTmp)
				break;

			short* pData = pTraceTmp->getTrace();
			for (int jj = 0; jj < iHeight; jj++)
			{
				short sColor = int(pData[jj] * tgCoef[jj]) / 2048 + 32;

				if (sColor < 0)
					sColor = 0;
				else if (sColor > 63)
					sColor = 63;

				COLORREF* pBuf = pPicSideView[i] + iWidth * (iHeight - jj - 1) + j;
				*pBuf = palette.getColorref()[sColor];
			}
		}

		CClientDC oDCTmp(NULL);                                   //m_hwnd 创建客户区绘制内存
		HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView[i], CBM_INIT, pPicSideView[i], (LPBITMAPINFO)&oBIHSideView[i], DIB_RGB_COLORS);
		CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
		CDC oDCImage;    oDCImage.CreateCompatibleDC(&oDCTmp);    //内存DC
		oDCImage.SelectObject(&oBitmap);                          //选取位图对象

		//分片保存为对应的图片，每个图片的宽度最大为1280
		int iPices = (iWidth % 640) ? (iWidth / 640 + 1) : (iWidth / 640);
		//分片存为固定的长度
		for (int j = 1; j <= iPices; j++)
		{
			int iWithTmp = (j + 1) * 640 - (j - 1) * 640;
			int iStartX  = (j - 1) * 640;
			if ((j + 1) * 640 > iWidth)
				iWithTmp = iWidth - (j - 1) * 640;

			BitBlt(image.GetDC(), PIC_SPACE_LEFT + (j-1)*1280 , PIC_SPACE_TOP + (iHeight + PIC_SPACE_INTERVAL)* (i - 1), iWithTmp, iHeight, oDCImage.m_hDC, iStartX, 0, SRCCOPY);

			if ((j + 1) * 640 > iWidth)
				break;
		}

		//释放BMP像素空间
		delete[] pPicSideView[i];

		oBitmap.DeleteObject();
		DeleteObject(hBitmap);

	}


	HRESULT hResult = image.Save(strTmp);
	image.ReleaseDC();
}

//void RadarFrame::CreateChannelDC(CDC oDCImage, SwathChannel * pChannel, int iChannelStart, int iChannelWidth, int iChannelHeight, int iPicWidth, int iPicHeight)
//{
//}

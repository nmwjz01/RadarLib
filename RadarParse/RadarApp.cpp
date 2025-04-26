
// RadarParse.cpp: 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
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

#include "RadarApp.h"
#include "RadarFrame.h"

#include "RadarDoc.h"
#include "RadarViewMain.h"
#include "AboutDlg.h"
#include "DialogZLevel.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// RadarApp
BEGIN_MESSAGE_MAP(RadarApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &RadarApp::OnAppAbout)
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_LEFT, OnOpenLeft)
	ON_COMMAND(ID_OPEN_RIGHT, OnFileRight)
	ON_COMMAND(ID_SETTING_ZLEVEL, OnSettingZLevel)
	ON_COMMAND(ID_OPEN_SWATH,OnOpenSwath)
	ON_COMMAND(ID_FILE_EXPORT, &RadarApp::OnFileExport)
END_MESSAGE_MAP()


// RadarApp 构造
RadarApp::RadarApp() noexcept
{
	m_bHiColorIcons = TRUE;

	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// 如果应用程序是利用公共语言运行时支持(/clr)构建的，则: 
	//     1) 必须有此附加设置，“重新启动管理器”支持才能正常工作。
	//     2) 在您的项目中，您必须按照生成顺序向 System.Windows.Forms 添加引用。
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("RadarParse.AppID.NoVersion"));

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	m_pProject = NULL;
}

// 唯一的 RadarApp 对象

RadarApp theApp;


// RadarApp 初始化

BOOL RadarApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// 使用 RichEdit 控件需要 AfxInitRichEdit2()
	// AfxInitRichEdit2();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(8);  // 加载标准 INI 文件选项(包括 MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。  文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(RadarDoc),
		RUNTIME_CLASS(RadarFrame),       // 主 SDI 框架窗口
		RUNTIME_CLASS(RadarViewMain));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// 分析标准 shell 命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// 调度在命令行中指定的命令。  如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// 唯一的一个窗口已初始化，因此显示它并对其进行更新
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

// RadarApp 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
// 用于运行对话框的应用程序命令
void RadarApp::OnAppAbout()
{
	AboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// RadarApp 自定义加载/保存方法

void RadarApp::PreLoadState()
{
	//BOOL bNameValid;
	CString strName;
	//bNameValid = strName.LoadString(IDS_EDIT_MENU);
	//ASSERT(bNameValid);
	//GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	//bNameValid = strName.LoadString(IDS_EXPLORER);
	//ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void RadarApp::LoadCustomState()
{
}

void RadarApp::SaveCustomState()
{
}

// RadarApp 消息处理程序
void RadarApp::OnFileOpen()
{
	CString	strPath;     //返回的路径串
	int iPathLen = 0;    //返回路径串的长度

	CString	strTitle = _T("请定位工程主目录");

	CFolderPickerDialog oDialog;
	oDialog.m_ofn.lpstrTitle = strTitle;
	oDialog.m_ofn.lpstrInitialDir = _T("");

	//读取路径对话框
	if (oDialog.DoModal() != IDOK)
		return;

	//读取用户选择的目录串
	strPath = oDialog.GetPathName();
	iPathLen = strPath.GetLength();
	if (0 >= iPathLen)
	{
		CString strOut = _T("加载工程失败，工程目录：") + strPath + _T( "；长度为0" );
		AfxMessageBox(strOut);
		return;
	}

	if (NULL != m_pProject)
	{
		delete m_pProject;
		m_pProject = NULL;
	}
	//记录工程根目录
	m_Path = strPath;

	CStringA strPathA = (CStringA)strPath;
	char *pPath = (char *)strPathA.GetBuffer(strPath.GetLength());
	m_pProject = new Project();
	int iResult = m_pProject->init(pPath);

	//初始化工程的结果
	if (0 != iResult)
	{
		delete m_pProject;
		m_pProject = NULL;

		CString strOut = _T("加载工程失败，工程目录：") + strPath;
		AfxMessageBox(strOut);
		return;
	}

	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->ProjectOpen(m_pProject);
}

//检测线数据导出为图片
void RadarApp::OnFileExport()
{
	//如果工程还没有打开，则不做任何处理
	if (NULL == m_pProject)
		return;

	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->ProjectExport(m_pProject);
}


//打开左辅图
void RadarApp::OnOpenLeft()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->OpenViewLeft();
}
//打开右辅图
void RadarApp::OnFileRight()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->OpenViewRight();
}
//Z-Level设置
void RadarApp::OnSettingZLevel()
{
	DialogZLevel oZLevel;
	oZLevel.DoModal();
}

void RadarApp::OnOpenSwath()
{
	RadarFrame* pFrame = (RadarFrame*)AfxGetMainWnd();
	pFrame->OpenSwath();
}


//读配置文件
void RadarApp::CfgFileRead()
{
	CString cfgFile = ".\\RadarParse.ini";
	FILE* fp = fopen(cfgFile, "r");
	DWORD ret;
	if (!fp)
	{
		m_PathRadar  = "";    //雷达目录
		m_PathCamera1 = "";    //摄像头1目录
		m_PathCamera2 = "";    //摄像头2目录
		m_PathCamera3 = "";    //摄像头3目录
		m_PathCamera4 = "";    //摄像头4目录
	}
	else
	{
		fclose(fp);

		char strTmp[512] = { 0 };

		//雷达目录
		ret = GetPrivateProfileString(_T("Radar"), _T("Path"), _T("!"), strTmp, MAX_PATH, cfgFile); m_PathRadar = strTmp;

		//摄像头目录
		ret = GetPrivateProfileString(_T("Camera1"), _T("Path"), _T(""), strTmp, MAX_PATH, cfgFile); m_PathCamera1 = strTmp;
		ret = GetPrivateProfileString(_T("Camera2"), _T("Path"), _T(""), strTmp, MAX_PATH, cfgFile); m_PathCamera2 = strTmp;
		ret = GetPrivateProfileString(_T("Camera3"), _T("Path"), _T(""), strTmp, MAX_PATH, cfgFile); m_PathCamera3 = strTmp;
		ret = GetPrivateProfileString(_T("Camera4"), _T("Path"), _T(""), strTmp, MAX_PATH, cfgFile); m_PathCamera4 = strTmp;
	}
}

//写配置文件
void RadarApp::CfgFileWrite()
{
	CString cfgFile = ".\\RadarParse.ini";
	FILE* fp = fopen(cfgFile, "w");
	if (fp)
		fclose(fp);

	DWORD ret;
	CString strTmp;

	//摄像头目录
	ret = WritePrivateProfileString(_T("Camera1"), _T("Path"), m_PathCamera1, cfgFile);
	ret = WritePrivateProfileString(_T("Camera2"), _T("Path"), m_PathCamera2, cfgFile);
	ret = WritePrivateProfileString(_T("Camera3"), _T("Path"), m_PathCamera3, cfgFile);
	ret = WritePrivateProfileString(_T("Camera4"), _T("Path"), m_PathCamera4, cfgFile);

	//雷达目录
	ret = WritePrivateProfileString(_T("Radar"), _T("Path"), m_PathRadar, cfgFile);
}

// PicProcess.cpp: 实现文件
//

#include "pch.h"
#include "RadarDemo2.h"
#include "PicProcess.h"
#include "afxdialogex.h"


// PicProcess 对话框

IMPLEMENT_DYNAMIC(PicProcess, CDialogEx)

PicProcess::PicProcess(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_PIC_PROCESS, pParent)
{

}

PicProcess::~PicProcess()
{
}

void PicProcess::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PicProcess, CDialogEx)
	ON_BN_CLICKED(IDC_PIC_BROWSER, &PicProcess::OnBnClickedPicBrowser)
	ON_BN_CLICKED(IDC_PIC_SPLIT_V, &PicProcess::OnBnClickedPicSplitV)
	ON_BN_CLICKED(IDC_PIC_SPLIT_H, &PicProcess::OnBnClickedPicSplitH)
END_MESSAGE_MAP()


// PicProcess 消息处理程序
//设置工程目录
extern "C" __declspec(dllimport) int setProjectPath(const char *strpath);
//初始化工程
extern "C" __declspec(dllimport) int initProject();
extern "C" __declspec(dllimport) int unInitProject();
//设置对比度
extern "C" __declspec(dllimport) int setParamContrast(BOOL autoEnable, int contrast);
//设置增益
extern "C" __declspec(dllimport) int setParamGain(BOOL autoEnable, int gain);

extern "C" __declspec(dllimport) int getSwathCount();
extern "C" __declspec(dllimport) int getSwathName(int index, char *swathName);
extern "C" __declspec(dllimport) int getChannelCount(const char * swathName);
extern "C" __declspec(dllimport) int getTraceCount(const char *  swathName, int channelID);

//切图处理
extern "C" __declspec(dllimport) int exportRadarPicV(const char * picFile, const char * swathName, BOOL withLine, int channelID, int traceBegin, int traceCount, int deep);
extern "C" __declspec(dllimport) int exportRadarPicH(const char * picFile, const char * swathName, BOOL withLine, int deep, int traceBegin, int traceCount);

void PicProcess::OnBnClickedPicBrowser()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_PATH_PIC_PROCESS);

	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = this->m_hWnd;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	TCHAR pszFolder[MAX_PATH * 2] = { 0 };
	LPITEMIDLIST pItemidList = ::SHBrowseForFolder(&bi);

	if (NULL != pItemidList)
	{
		if (!::SHGetPathFromIDList(pItemidList, pszFolder))
		{
			return;
		}

		pEditSrc->SetWindowText(pszFolder);
	}
}

//垂直切图
void PicProcess::OnBnClickedPicSplitV()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_PATH_PIC_PROCESS);
	CString strProject; pEditProject->GetWindowText(strProject);

	//输入的合法性
	if (strProject.IsEmpty())
	{
		MessageBox("请选择工程目录");
		return;
	}

	char pathProject[512] = { 0 };
	strcpy(pathProject, (char *)strProject.GetBuffer(strProject.GetLength()));

	int length = (int)strlen(pathProject);
	wchar_t* buffer = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)buffer, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathProject, length, buffer, length);

	//初始化工程
	int iResult = 0;
	iResult = setProjectPath((const char *)buffer);
	iResult = initProject();

	//设置对比度
	iResult = setParamContrast(FALSE, 200);
	//设置增益
	iResult = setParamGain(FALSE, 20);

	CString strOut;
	//获取测线数量
	int iSwathCount = getSwathCount();
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	//读取第一个测线名称
	wchar_t* szSwathName = (wchar_t*)malloc(256);
	//处理第一个测线
	getSwathName(0, (char *)szSwathName);

	char szVPic[512] = { "C:\\3D_TestData\\PicDstV\\1.jpg" };
	length = (int)strlen(szVPic);
	wchar_t* bufferVPic = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)bufferVPic, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVPic, length, bufferVPic, length);

	//切图处理
	exportRadarPicV((const char *)bufferVPic, (const char *)szSwathName, 1, 1, 1, 1240, 256);
	//=================================================================//

	strcpy(szVPic, "C:\\3D_TestData\\PicDstV\\2.jpg");
	length = (int)strlen(szVPic);
	wchar_t* bufferVPic2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)bufferVPic2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVPic, length, bufferVPic2, length);

	//切图处理
	exportRadarPicV((const char *)bufferVPic2, (const char *)szSwathName, 1, 2, 1, 1240, 256);
	//=================================================================//

	strcpy(szVPic, "C:\\3D_TestData\\PicDstV\\3.jpg");
	length = (int)strlen(szVPic);
	wchar_t* bufferVPic3 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)bufferVPic3, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVPic, length, bufferVPic3, length);

	//切图处理
	exportRadarPicV((const char *)bufferVPic3, (const char *)szSwathName, 1, 3, 1, 1240, 256);
	//=================================================================//

	strcpy(szVPic, "C:\\3D_TestData\\PicDstV\\4.jpg");
	length = (int)strlen(szVPic);
	wchar_t* bufferVPic4 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)bufferVPic4, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVPic, length, bufferVPic4, length);

	//切图处理
	exportRadarPicV((const char *)bufferVPic4, (const char *)szSwathName, 1, 4, 1, 1240, 256);
	//=================================================================//

	strcpy(szVPic, "C:\\3D_TestData\\PicDstV\\5.jpg");
	length = (int)strlen(szVPic);
	wchar_t* bufferVPic5 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)bufferVPic5, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVPic, length, bufferVPic5, length);

	//切图处理
	exportRadarPicV((const char *)bufferVPic5, (const char *)szSwathName, 1, 5, 1, 1240, 256);
	//=================================================================//

	strcpy(szVPic, "C:\\3D_TestData\\PicDstV\\6.jpg");
	length = (int)strlen(szVPic);
	wchar_t* bufferVPic6 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)bufferVPic6, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVPic, length, bufferVPic6, length);

	//切图处理
	exportRadarPicV((const char *)bufferVPic6, (const char *)szSwathName, 1, 6, 1, 1240, 256);
	//=================================================================//

	unInitProject();

	MessageBox("Finish ok pic");
}

//水平切图
void PicProcess::OnBnClickedPicSplitH()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_PATH_PIC_PROCESS);
	CString strProject; pEditProject->GetWindowText(strProject);

	//输入的合法性
	if (strProject.IsEmpty())
	{
		MessageBox("请选择工程目录");
		return;
	}

	char pathProject[512] = { 0 };
	strcpy(pathProject, (char *)strProject.GetBuffer(strProject.GetLength()));

	int length = (int)strlen(pathProject);
	wchar_t* buffer = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)buffer, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathProject, length, buffer, length);

	//初始化工程
	int iResult = 0;
	iResult = setProjectPath((const char *)buffer);
	iResult = initProject();

	//设置对比度
	iResult = setParamContrast(FALSE, 200);
	//设置增益
	iResult = setParamGain(FALSE, 10);

	CString strOut;
	//获取测线数量
	int iSwathCount = getSwathCount();
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	//读取第一个测线名称
	wchar_t* szSwathName = (wchar_t*)malloc(256);

	//处理第一个测线
	getSwathName(0, (char *)szSwathName);
	//strOut.Format("The first SwathName:%s", szSwathName);
	//MessageBox(strOut);

	char szVPic[512] = { "C:\\3D_TestData\\PicDstH\\1.jpg" };
	length = (int)strlen(szVPic);
	wchar_t* bufferVPic = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)bufferVPic, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVPic, length, bufferVPic, length);

	//切图处理
	exportRadarPicH((const char *)bufferVPic, (const char *)szSwathName, 1, 0, 1, 1240);

	unInitProject();

	MessageBox("Finish");
}

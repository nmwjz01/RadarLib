// AlgorithmSimple.cpp: 实现文件
//

#include "pch.h"
#include "RadarDemo2.h"
#include "AlgorithmSimple.h"
#include "afxdialogex.h"


// AlgorithmSimple 对话框

IMPLEMENT_DYNAMIC(AlgorithmSimple, CDialogEx)

AlgorithmSimple::AlgorithmSimple(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_ALGORITHM_SIMPLE, pParent)
{

}

AlgorithmSimple::~AlgorithmSimple()
{
}

void AlgorithmSimple::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(AlgorithmSimple, CDialogEx)
	ON_BN_CLICKED(IDC_SIMPLE_SIGPOSITION_BROWSER, &AlgorithmSimple::OnBnClickedSimpleSigpositionBrowser)
	ON_BN_CLICKED(IDC_SIMPLE_SIGPOSITION_GET, &AlgorithmSimple::OnBnClickedSimpleSigpositionGet)
	ON_BN_CLICKED(IDC_SIMPLE_SIGPOSITION_PIC, &AlgorithmSimple::OnBnClickedSimpleSigpositionPic)
	ON_BN_CLICKED(IDC_SIMPLE_SIGPOSITION_CUT, &AlgorithmSimple::OnBnClickedSimpleSigpositionCut)
	ON_BN_CLICKED(IDC_SIMPLE_GAININVDECAY, &AlgorithmSimple::OnBnClickedSimpleGaininvdecay)
	ON_BN_CLICKED(IDC_SIMPLE_BACKGROUND, &AlgorithmSimple::OnBnClickedSimpleBackground)
	ON_BN_CLICKED(IDC_SIMPLE_REMOVEDC, &AlgorithmSimple::OnBnClickedSimpleRemovedc)
END_MESSAGE_MAP()


extern "C" __declspec(dllimport) int setProjectPath(const char *strpath);
//初始化工程
extern "C" __declspec(dllimport) int initProject();
extern "C" __declspec(dllimport) int unInitProject();

extern "C" __declspec(dllimport) int getSwathCount();
extern "C" __declspec(dllimport) int getSwathName(int index, char *swathName);
extern "C" __declspec(dllimport) int getChannelCount(const char * swathName);
extern "C" __declspec(dllimport) int getTraceCount(const char *  swathName, int channelID);

extern "C" __declspec(dllimport) int initSwath(const char * swathName);
extern "C" __declspec(dllimport) int unInitSwath(const char * swathName);

//生成直达波图像
extern "C" __declspec(dllimport) int SigPositionPicByVende(const char * swathName, const int channelID, const char * sigPositionPic, const int traceNum);
//切除直达波
extern "C" __declspec(dllimport) int SigPositionCutEx2ByVende(const char * swathName, const int channelID);
extern "C" __declspec(dllimport) int SigPositionCutEx3ByVende(const char * swathName, const int channelID, int iZero);

//获取直达波位置
extern "C" __declspec(dllimport) int SimpleSigPositionGet(const char * swathName, const int channelID);
extern "C" __declspec(dllimport) int SimpleSigPositionGetEx(const char * swathName, const int channelID, int direct, const int waveNum);
//生成直达波图像
extern "C" __declspec(dllimport) int SimpleSigPositionPic(const char * swathName, const int channelID, const char * sigPositionPic);
//切除直达波
extern "C" __declspec(dllimport) int SimpleSigPositionCut(const char * swathName, const int channelID, int iZero);
extern "C" __declspec(dllimport) int SimpleSigPositionCutEx(const char * swathName, const int channelID);

//删除背景噪声
extern "C" __declspec(dllimport) int SimpleRemoveBackgr(const char * swathName, const int channelID);

//逆振幅算法
extern "C" __declspec(dllimport) int SimpleGainInvDecayCurve(const char * swathName, const int channelID, int k, int n);

//删除直流噪声
extern "C" __declspec(dllimport) int SimpleRemoveDC(const char * swathName, const int channelID);

// AlgorithmSimple 消息处理程序

void AlgorithmSimple::OnBnClickedSimpleSigpositionBrowser()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_PATH_SIMPLE_SIGPOSITION);

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

//直达波位置获取
void AlgorithmSimple::OnBnClickedSimpleSigpositionGet()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_PATH_SIMPLE_SIGPOSITION);
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

	//获取测线数量
	int iSwathCount = getSwathCount();
	CString strOut;
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	for (int i = 0; i < iSwathCount; i++)
	{
		//读取第一个测线名称
		wchar_t* szSwathName = (wchar_t*)malloc(256);
		getSwathName(i, (char *)szSwathName);

		int iChannelCount = getChannelCount((const char*)szSwathName);

		for (int j = 1; j <= iChannelCount; j++)
		{
			//直达波切除--自动
			int iZero = 0;
			//iZero = SigPositionCutEx2ByVende((const char *)szSwathName, j);
			//iZero = SimpleSigPositionGet((const char *)szSwathName, j);
			//iZero = SimpleSigPositionGetEx((const char *)szSwathName, j, 1, 2);
		}
		free(szSwathName);
	}

	unInitProject();

	strOut = "计算直达波位置完成";
	MessageBox(strOut);
}

//直达波图像生成
void AlgorithmSimple::OnBnClickedSimpleSigpositionPic()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_PATH_SIMPLE_SIGPOSITION);
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

	CString strOut;
	//初始化工程结果显示
	strOut.Format("The Result of initProject:%d", iResult);
	MessageBox(strOut);

	//获取测线数量
	int iSwathCount = getSwathCount();
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	//下面开始产生直达波图像
	wchar_t* bufferPic = (wchar_t*)malloc(256);
	memset((char *)bufferPic, 0, 256);
	char bfPic[128] = { 0 };
	for (int i = 0; i < iSwathCount; i++)
	{
		//读取第一个测线名称
		wchar_t* szSwathName = (wchar_t*)malloc(256);
		getSwathName(i, (char *)szSwathName);

		int iChannelCount = getChannelCount((const char*)szSwathName);

		for (int j = 1; j <= iChannelCount; j++)
		{
			sprintf(bfPic, "%s\\SigPosition\\Swath_%03d_%02d.jpg", pathProject, i, j);
			//strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_01.jpg");
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));

			//创建对应测线通道的直达波图像---当前vende使用的创建直达波图像的函数
			SigPositionPicByVende((const char *)szSwathName, j, (const char *)bufferPic, 10);

			//创建对应测线通道的直达波图像---将来的开源代码使用
			//SimpleSigPositionPic((const char *)szSwathName, j, (const char *)bufferPic);
		}

		free(szSwathName);
	}
	MessageBox("直达波图像生成OK");
	unInitProject();
}

//直达波切除
void AlgorithmSimple::OnBnClickedSimpleSigpositionCut()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_PATH_SIMPLE_SIGPOSITION);
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

	CString strOut;
	//初始化工程结果显示
	strOut.Format("The Result of initProject for Sigposition:%d", iResult);
	MessageBox(strOut);

	//获取测线数量
	int iSwathCount = getSwathCount();
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	for (int i = 0; i < iSwathCount; i++)
	{
		//读取第一个测线名称
		wchar_t* szSwathName = (wchar_t*)malloc(256);
		getSwathName(i, (char *)szSwathName);

		int iChannelCount = getChannelCount((const char*)szSwathName);

		for (int j = 1; j <= iChannelCount; j++)
		{
			//直达波切除--自动
			int iZero = SigPositionCutEx2ByVende((const char *)szSwathName, j);
		}
		free(szSwathName);
	}
	MessageBox("完成直达波切除");

	//釋放Radar库
	unInitProject();
}

//背景去噪
void AlgorithmSimple::OnBnClickedSimpleBackground()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_PATH_SIMPLE_SIGPOSITION);
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

	CString strOut;
	//初始化工程结果显示
	strOut.Format("the Result of initProject for RemoveBackground:%d", iResult);
	MessageBox(strOut);

	//设置对比度
	//iResult = setParamContrast(FALSE, 120);
	//strOut.Format("the Result of setParamContrast:%d", iResult);
	//MessageBox(strOut);

	//设置增益
	//iResult = setParamGain(FALSE, 120);
	//strOut.Format("the Result of setParamGain:%d", iResult);
	//MessageBox(strOut);

	//获取测线数量
	int iSwathCount = getSwathCount();
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	for (int i = 0; i < iSwathCount; i++)
	{
		//读取第一个测线名称
		wchar_t* szSwathName = (wchar_t*)malloc(256);
		getSwathName(i, (char *)szSwathName);

		int iChannelCount = getChannelCount((const char*)szSwathName);

		for (int j = 1; j <= iChannelCount; j++)
		{
			SimpleRemoveBackgr((const char *)szSwathName, j);
		}
		free(szSwathName);
	}

	unInitProject();

	MessageBox("Finish");
}

//逆振幅处理
void AlgorithmSimple::OnBnClickedSimpleGaininvdecay()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_PATH_SIMPLE_SIGPOSITION);
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

	CString strOut;
	//初始化工程结果显示
	strOut.Format("the Result of initProject:%d", iResult);
	MessageBox(strOut);

	//获取测线数量
	int iSwathCount = getSwathCount();
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	//读取第一个测线名称
	wchar_t* szSwathName = (wchar_t*)malloc(512);
	//处理第一个测线
	getSwathName(0, (char *)szSwathName);
	strOut.Format("The first SwathName:%s", szSwathName);
	MessageBox(strOut);
	//逆振幅衰減
	SimpleGainInvDecayCurve((const char *)szSwathName,  1, 25, 0);
	SimpleGainInvDecayCurve((const char *)szSwathName,  2, 25, 4);
	SimpleGainInvDecayCurve((const char *)szSwathName,  3, 25, 8);
	SimpleGainInvDecayCurve((const char *)szSwathName,  4, 25, 12);
	SimpleGainInvDecayCurve((const char *)szSwathName,  5, 25, 16);
	SimpleGainInvDecayCurve((const char *)szSwathName,  6, 25, 20);
	SimpleGainInvDecayCurve((const char *)szSwathName,  7, 25, 24);
	SimpleGainInvDecayCurve((const char *)szSwathName,  8, 25, 28);
	SimpleGainInvDecayCurve((const char *)szSwathName,  9, 25, 24);
	SimpleGainInvDecayCurve((const char *)szSwathName, 10, 25, 20);
	SimpleGainInvDecayCurve((const char *)szSwathName, 11, 25, 16);
	SimpleGainInvDecayCurve((const char *)szSwathName, 12, 25, 12);
	SimpleGainInvDecayCurve((const char *)szSwathName, 13, 25, 8);
	SimpleGainInvDecayCurve((const char *)szSwathName, 14, 25, 4);
	SimpleGainInvDecayCurve((const char *)szSwathName, 15, 25, 0);
	//SimpleGainInvDecayCurve((const char *)szSwathName, 6);

	MessageBox("完成逆振幅衰減-Simple");

	free(szSwathName);

	unInitProject();
}

//删除直流噪声
void AlgorithmSimple::OnBnClickedSimpleRemovedc()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_PATH_SIMPLE_SIGPOSITION);
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

	CString strOut;
	//初始化工程结果显示
	strOut.Format("the Result of initProject for RemoveBackground:%d", iResult);
	MessageBox(strOut);

	//设置对比度
	//iResult = setParamContrast(FALSE, 120);
	//strOut.Format("the Result of setParamContrast:%d", iResult);
	//MessageBox(strOut);

	//设置增益
	//iResult = setParamGain(FALSE, 120);
	//strOut.Format("the Result of setParamGain:%d", iResult);
	//MessageBox(strOut);

	//获取测线数量
	int iSwathCount = getSwathCount();
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	for (int i = 0; i < iSwathCount; i++)
	{
		//读取第一个测线名称
		wchar_t* szSwathName = (wchar_t*)malloc(256);
		getSwathName(i, (char *)szSwathName);

		int iChannelCount = getChannelCount((const char*)szSwathName);

		for (int j = 1; j <= iChannelCount; j++)
		{
			iResult = SimpleRemoveDC((const char *)szSwathName, j );
		}

		free(szSwathName);
	}

	unInitProject();

	MessageBox("Finish");
}


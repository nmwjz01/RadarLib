// AlgorithmMatlab.cpp: 实现文件
//

#include "pch.h"
#include "RadarDemo2.h"
#include "AlgorithmMatlab.h"
#include "afxdialogex.h"


// AlgorithmMatlab 对话框

IMPLEMENT_DYNAMIC(AlgorithmMatlab, CDialogEx)

AlgorithmMatlab::AlgorithmMatlab(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_ALGORITHM_MATLAB, pParent)
{

}

AlgorithmMatlab::~AlgorithmMatlab()
{
}

void AlgorithmMatlab::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(AlgorithmMatlab, CDialogEx)
	ON_BN_CLICKED(IDC_MATLAB_SIGPOSITION_PIC, &AlgorithmMatlab::OnBnClickedMatlabSigpositionPic)
	ON_BN_CLICKED(IDC_MATLAB_SIGPOSITION_CUT, &AlgorithmMatlab::OnBnClickedMatlabSigpositionCut)
	ON_BN_CLICKED(IDC_MATLAB_SIGPOSITION_GET, &AlgorithmMatlab::OnBnClickedMatlabSigpositionGet)
	ON_BN_CLICKED(IDC_MATLAB_SIGPOSITION_BROWSER, &AlgorithmMatlab::OnBnClickedMatlabSigpositionBrowser)
	ON_BN_CLICKED(IDC_MATLAB_BACKGROUND, &AlgorithmMatlab::OnBnClickedMatlabBackground)
	ON_BN_CLICKED(IDC_MATLAB_GAININVDECAY, &AlgorithmMatlab::OnBnClickedMatlabGaininvdecay)
	ON_BN_CLICKED(IDC_MATLAB_FILTER_BANDPASS, &AlgorithmMatlab::OnBnClickedMatlabFilterBandpass)
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

extern "C" __declspec(dllimport) bool RadarMathInit();
extern "C" __declspec(dllimport) void RadarMathUninit();

extern "C" __declspec(dllimport) int SigPositionPic(const char * swathName, const int channelID, const char * sigPositionPic);
extern "C" __declspec(dllimport) int SigPositionNum(const char * swathName, const int channelID);
extern "C" __declspec(dllimport) int SigPositionNumEx(const char * swathName, const int channelID, int direct);
extern "C" __declspec(dllimport) int SigPositionCut(const char * swathName, const int channelID, int iZero);

//extern "C" __declspec(dllimport) int RemoveBackgr(const char * swathName, const int channelID, const int offerset);
extern "C" __declspec(dllimport) int RemoveBackgr(const char * swathName, const int channelID );
extern "C" __declspec(dllimport) int RemoveBackgr450M(const char * swathName, const int channelID, int scaleBase);
extern "C" __declspec(dllimport) int RemoveDC(const char * swathName, const int channelID);
extern "C" __declspec(dllimport) int GainInvDecay(const char * swathName, const int channelID);
extern "C" __declspec(dllimport) int GainInvDecayEx(const char * swathName, const int channelID, int curve, int order);

extern "C" __declspec(dllimport) int FilterButterworth(const char * swathName, const int channelID, int freqStart, int freqEnd);

// AlgorithmMatlab 消息处理程序

void AlgorithmMatlab::OnBnClickedMatlabSigpositionBrowser()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_SIGPOSITION);

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
void AlgorithmMatlab::OnBnClickedMatlabSigpositionGet()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_EDIT_SIGPOSITION);
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

	//初始化Radar库
	RadarMathInit();

	//读取第一个测线名称
	wchar_t* szSwathName = (wchar_t*)malloc(64);
	int positionNum = 0;
	CString strPosNum = "";

	CString strOut;
	strOut.Format("Finish initial matraptor");
	MessageBox(strOut);

	//获取第一个测线的直达波位置
	{
		getSwathName(0, (char *)szSwathName);
		strOut.Format("The first SwathName:%s", szSwathName);
		MessageBox(strOut);

		//创建对应测线通道的直达波图像
		positionNum = SigPositionNum((const char *)szSwathName, 1);
		strPosNum.Format("1 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNum((const char *)szSwathName, 2);
		strPosNum.Format("2 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNum((const char *)szSwathName, 3);
		strPosNum.Format("3 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNum((const char *)szSwathName, 4);
		strPosNum.Format("4 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNumEx((const char *)szSwathName, 5, 0);
		strPosNum.Format("5 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNumEx((const char *)szSwathName, 6, -1);
		strPosNum.Format("6 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNumEx((const char *)szSwathName, 7, 1);
		strPosNum.Format("7 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNumEx((const char *)szSwathName, 8, 0);
		strPosNum.Format("8 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNumEx((const char *)szSwathName, 9, -1);
		strPosNum.Format("9 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		positionNum = SigPositionNumEx((const char *)szSwathName, 10, 1);
		strPosNum.Format("10 PositionNum:%d", positionNum);
		MessageBox(strPosNum);

		//positionNum = SigPositionNum((const char *)szSwathName, 11);
		//strPosNum.Format("11 PositionNum:%d", positionNum);
		//MessageBox(strPosNum);

		//positionNum = SigPositionNum((const char *)szSwathName, 12);
		//strPosNum.Format("12 PositionNum:%d", positionNum);
		//MessageBox(strPosNum);
	}

	//釋放Radar库
	RadarMathUninit();

	unInitProject();
}

//创建通道测线的直达波图像
void AlgorithmMatlab::OnBnClickedMatlabSigpositionPic()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_EDIT_SIGPOSITION);
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

	//初始化Radar库
	RadarMathInit();
	strOut.Format("Finish initial matraptor");
	MessageBox(strOut);

	//读取第一个测线名称
	wchar_t* szSwathName = (wchar_t*)malloc(64);

	//下面开始产生直达波图像
	wchar_t* bufferPic = (wchar_t*)malloc(128);
	memset((char *)bufferPic, 0, 128);
	char bfPic[64] = { 0 };

	//获取第一个测线的直达波
	{
		getSwathName(0, (char *)szSwathName);
		strOut.Format("The first SwathName:%s", szSwathName);
		MessageBox(strOut);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_01.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 1, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_02.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 2, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_03.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 3, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_04.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 4, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_05.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 5, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_06.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 6, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_07.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 7, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_08.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 8, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_09.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 9, (const char *)bufferPic);

		strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_10.jpg");
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		SigPositionPic((const char *)szSwathName, 10, (const char *)bufferPic);

		//strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_11.jpg");
		//MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		//SigPositionPic((const char *)szSwathName, 11, (const char *)bufferPic);

		//strcpy(bfPic, "C:\\3D_TestData\\SigPosition\\001_12.jpg");
		//MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bfPic, (int)strlen(bfPic), bufferPic, (int)strlen(bfPic));
		//创建对应测线通道的直达波图像
		//SigPositionPic((const char *)szSwathName, 12, (const char *)bufferPic);
	}

	strOut.Format("生成直达波图像。Call SigPositionPic for swath:%d", iResult);
	MessageBox(strOut);

	//釋放Radar库
	RadarMathUninit();

	unInitProject();
}

//直达波位置切除
void AlgorithmMatlab::OnBnClickedMatlabSigpositionCut()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_EDIT_SIGPOSITION);
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

	CString strOut = "";
	//初始化工程结果显示
	strOut.Format("the Result of initProject:%d", iResult);
	MessageBox(strOut);

	//获取测线数量
	int iSwathCount = getSwathCount();
	strOut.Format("the count of iSwathCount:%d", iSwathCount);
	MessageBox(strOut);

	//读取第一个测线名称
	wchar_t* szSwathName = (wchar_t*)malloc(64);
	getSwathName(1, (char *)szSwathName);
	//strOut.Format("The first SwathName:%s", szSwathName);
	//MessageBox(strOut);

	//初始化Radar库
	RadarMathInit();

	strOut.Format("Finish initial matraptor:%s", szSwathName);
	MessageBox(strOut);

	//处理第一个测线
	getSwathName(0, (char *)szSwathName);
	strOut.Format("The first SwathName:%s", szSwathName);
	MessageBox(strOut);
	//直达波切除
	SigPositionCut((const char *)szSwathName, 1, 36);
	SigPositionCut((const char *)szSwathName, 2, 41);
	SigPositionCut((const char *)szSwathName, 3, 37);
	SigPositionCut((const char *)szSwathName, 4, 41);
	SigPositionCut((const char *)szSwathName, 5, 37);
	SigPositionCut((const char *)szSwathName, 6, 40);
	SigPositionCut((const char *)szSwathName, 7, 36);
	SigPositionCut((const char *)szSwathName, 8, 41);
	SigPositionCut((const char *)szSwathName, 9, 36);
	SigPositionCut((const char *)szSwathName, 10, 40);
	SigPositionCut((const char *)szSwathName, 11, 37);
	SigPositionCut((const char *)szSwathName, 12, 42);


	//处理第二个测线
	getSwathName(1, (char *)szSwathName);
	strOut.Format("The Second SwathName:%s", szSwathName);
	MessageBox(strOut);
	//直达波切除
	SigPositionCut((const char *)szSwathName, 1, 37);
	SigPositionCut((const char *)szSwathName, 2, 41);
	SigPositionCut((const char *)szSwathName, 3, 37);
	SigPositionCut((const char *)szSwathName, 4, 41);
	SigPositionCut((const char *)szSwathName, 5, 37);
	SigPositionCut((const char *)szSwathName, 6, 40);
	SigPositionCut((const char *)szSwathName, 7, 36);
	SigPositionCut((const char *)szSwathName, 8, 41);
	SigPositionCut((const char *)szSwathName, 9, 36);
	SigPositionCut((const char *)szSwathName, 10, 40);
	SigPositionCut((const char *)szSwathName, 11, 38);
	SigPositionCut((const char *)szSwathName, 12, 42);

	//处理第三个测线
	getSwathName(2, (char *)szSwathName);
	strOut.Format("The Third SwathName:%s", szSwathName);
	MessageBox(strOut);
	//直达波切除
	SigPositionCut((const char *)szSwathName, 1, 37);
	SigPositionCut((const char *)szSwathName, 2, 41);
	SigPositionCut((const char *)szSwathName, 3, 37);
	SigPositionCut((const char *)szSwathName, 4, 41);
	SigPositionCut((const char *)szSwathName, 5, 37);
	SigPositionCut((const char *)szSwathName, 6, 40);
	SigPositionCut((const char *)szSwathName, 7, 36);
	SigPositionCut((const char *)szSwathName, 8, 41);
	SigPositionCut((const char *)szSwathName, 9, 36);
	SigPositionCut((const char *)szSwathName, 10, 40);
	SigPositionCut((const char *)szSwathName, 11, 37);
	SigPositionCut((const char *)szSwathName, 12, 41);


	//处理第四个测线
	getSwathName(3, (char *)szSwathName);
	strOut.Format("The Forth SwathName:%s", szSwathName);
	MessageBox(strOut);
	//直达波切除
	SigPositionCut((const char *)szSwathName, 1, 37);
	SigPositionCut((const char *)szSwathName, 2, 42);
	SigPositionCut((const char *)szSwathName, 3, 37);
	SigPositionCut((const char *)szSwathName, 4, 41);
	SigPositionCut((const char *)szSwathName, 5, 38);
	SigPositionCut((const char *)szSwathName, 6, 40);
	SigPositionCut((const char *)szSwathName, 7, 36);
	SigPositionCut((const char *)szSwathName, 8, 41);
	SigPositionCut((const char *)szSwathName, 9, 36);
	SigPositionCut((const char *)szSwathName, 10, 40);
	SigPositionCut((const char *)szSwathName, 11, 38);
	SigPositionCut((const char *)szSwathName, 12, 42);
	MessageBox("完成直达波切除");

	//釋放Radar库
	RadarMathUninit();

	unInitProject();
}


//背景去噪
void AlgorithmMatlab::OnBnClickedMatlabBackground()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_EDIT_SIGPOSITION);
	CString strProject; pEditProject->GetWindowText(strProject);

	//输入的合法性
	if (strProject.IsEmpty())
	{
		MessageBox("请选择工程目录");
		return;
	}

	char pathProject[512] = { 0 };
	strcpy(pathProject, (char *)strProject.GetBuffer(strProject.GetLength()));

	//const char *project = "C:\\3D_TestData\\";

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

	//初始化Radar库
	RadarMathInit();

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
			RemoveBackgr((const char *)szSwathName, j);
			//RemoveBackgr450M((const char *)szSwathName, j, 1);
		}
		free(szSwathName);
	}

	MessageBox("完成背景去噪");
	//釋放Radar库
	RadarMathUninit();

	unInitProject();
}


void AlgorithmMatlab::OnBnClickedMatlabGaininvdecay()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_EDIT_SIGPOSITION);
	CString strProject; pEditProject->GetWindowText(strProject);

	//输入的合法性
	if (strProject.IsEmpty())
	{
		MessageBox("请选择工程目录");
		return;
	}

	char pathProject[512] = { 0 };
	strcpy(pathProject, (char *)strProject.GetBuffer(strProject.GetLength()));

	//const char *project = "C:\\3D_TestData\\";

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

	//设置对比度
	//iResult = setParamContrast(FALSE, 120);
	//设置增益
	//iResult = setParamGain(FALSE, 120);

	//初始化Radar库
	RadarMathInit();

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
			GainInvDecayEx((const char *)szSwathName, j, 1, 10);
		}

		free(szSwathName);
	}
	MessageBox("完成逆振幅衰減");

	//釋放Radar库
	RadarMathUninit();

	unInitProject();
}


void AlgorithmMatlab::OnBnClickedMatlabFilterBandpass()
{
	CEdit *pEditProject = (CEdit *)GetDlgItem(IDC_EDIT_SIGPOSITION);
	CString strProject; pEditProject->GetWindowText(strProject);

	//输入的合法性
	if (strProject.IsEmpty())
	{
		MessageBox("请选择工程目录");
		return;
	}

	char pathProject[512] = { 0 };
	strcpy(pathProject, (char *)strProject.GetBuffer(strProject.GetLength()));

	//const char *project = "C:\\3D_TestData\\";

	int length = (int)strlen(pathProject);
	wchar_t* buffer = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)buffer, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathProject, length, buffer, length);

	//初始化工程
	int iResult = 0;
	iResult = setProjectPath((const char *)buffer);
	iResult = initProject();

	CString strOut;
	strOut.Format("The Result of initProject:%d", iResult);
	MessageBox(strOut);

	//初始化Radar库
	RadarMathInit();

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

		/*
		for (int j = 1; j <= 10; j++)
		{
			iResult = FilterButterworth((const char *)szSwathName, j, 200, 600 * 2);
		}
		for (int j = 11; j <= 29; j++)
		{
			iResult = FilterButterworth((const char *)szSwathName, j, 20, 390);
		}
		*/
		for (int j = 1; j <= iChannelCount; j++)
		{
			iResult = FilterButterworth((const char *)szSwathName, j, 150, 900);
		}

		free(szSwathName);
	}

	//釋放Radar库
	RadarMathUninit();
	unInitProject();

	strOut.Format("The Result of FilterButterworth:%d", iResult);
	MessageBox(strOut);
}

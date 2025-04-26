// DataTransfer.cpp: 实现文件
//

#include "pch.h"
#include "RadarDemo2.h"
#include "DataTransfer.h"
#include "afxdialogex.h"


// DataTransfer 对话框

IMPLEMENT_DYNAMIC(DataTransfer, CDialogEx)

DataTransfer::DataTransfer(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_TRANSFER, pParent)
{

}

DataTransfer::~DataTransfer()
{
}

void DataTransfer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DataTransfer, CDialogEx)
	ON_BN_CLICKED(IDC_BROWSER_INPUT, &DataTransfer::OnBnClickedBrowserInput)
	ON_BN_CLICKED(IDC_BROWSER_OUTPUT, &DataTransfer::OnBnClickedBrowserOutput)
	ON_BN_CLICKED(IDC_TRANSFER_SEGY, &DataTransfer::OnBnClickedTransferSegy)
	ON_BN_CLICKED(IDC_TRANSFER_DT, &DataTransfer::OnBnClickedTransferDt)
	ON_BN_CLICKED(IDC_TRANSFER_IDS, &DataTransfer::OnBnClickedTransferIds)
	ON_BN_CLICKED(IDC_TRANSFER_IDS16, &DataTransfer::OnBnClickedTransferIds16)
	ON_BN_CLICKED(IDC_TRANSFER_MALA, &DataTransfer::OnBnClickedTransferMala)
	ON_BN_CLICKED(IDC_TRANSFER_MALA_EX, &DataTransfer::OnBnClickedTransferMalaEx)
	ON_BN_CLICKED(IDC_TRANSFER_3DRADAR, &DataTransfer::OnBnClickedTransfer3dradar)
	ON_BN_CLICKED(IDC_TRANSFER_GSSI, &DataTransfer::OnBnClickedTransferGssi)
	ON_BN_CLICKED(IDC_TRANSFER_IDS08, &DataTransfer::OnBnClickedTransferIds08)
END_MESSAGE_MAP()


void DataTransfer::OnBnClickedBrowserInput()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);

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

void DataTransfer::OnBnClickedBrowserOutput()
{
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

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

		pEditDst->SetWindowText(pszFolder);
	}
}

// DataTransfer 消息处理程序
extern "C" __declspec(dllimport) int tranfersIDSData08(const char * pathIDS, const char * pathDst, int iSample);
extern "C" __declspec(dllimport) int tranfersIDSData16(const char * pathIDS, const char * pathDst);
extern "C" __declspec(dllimport) int tranfersDT(const char * swathPath, int freq, float separation, const char * swathPathDst);
extern "C" __declspec(dllimport) int tranfersIDS(const char * pathIDS, const char * pathDst, int freq, float separation);

extern "C" __declspec(dllimport) int tranfersMala(const char * pathMala, const char * pathDst);
extern "C" __declspec(dllimport) int tranfersMala32Ex(const char * pathMala, const char * pathDst);
extern "C" __declspec(dllimport) int tranfers3DRadar(const char * path3DRadar, const char * pathDst);
extern "C" __declspec(dllimport) int tranfersSegy(const char * path, const char * pathDst);
extern "C" __declspec(dllimport) int tranfersGSSI(const char * pathGSSI, const char * pathDst);

extern "C" __declspec(dllimport) bool RadarMathInit();


void DataTransfer::OnBnClickedTransferIds08()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

	CString strSrc; pEditSrc->GetWindowText(strSrc);
	CString strDst; pEditDst->GetWindowText(strDst);

	//输入的合法性
	if (strSrc.IsEmpty() || strDst.IsEmpty())
	{
		MessageBox("请选择输入和输出目录");
		return;
	}

	char pathSrc[512] = { 0 };
	char pathDst[512] = { 0 };

	strcpy(pathSrc, (char *)strSrc.GetBuffer(strSrc.GetLength()));
	strcpy(pathDst, (char *)strDst.GetBuffer(strDst.GetLength()));

	//MessageBox(_T(pathSrc));
	//const char *pathSrc = "C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\segy\\";  //"C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\欧美大地\\3D-Radar20240411数据\\SGY格式数据\\";  //
	//const char *pathDst = "C:\\3D_TestData\\RadarData\\";

	//3DRadar原始文件路径
	int length = (int)strlen(pathSrc);
	wchar_t* pathSrc2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathSrc2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathSrc, length, pathSrc2, length);

	int iSample = 640;
	{
		//Impluse目标文件路径
		length = (int)strlen(pathDst);
		wchar_t* pathDst2 = (wchar_t*)malloc(length * 2 + 2);
		memset((char *)pathDst2, 0, length * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathDst, length, pathDst2, length);

		int iResult = tranfersIDSData08((const char *)pathSrc2, (const char *)pathDst2, iSample);
		//char szResult[512] = { 0 };
		//sprintf(szResult, "转化完成，返回结果:%d", iResult);
		//MessageBox(szResult);
		iSample++;
	}

}

//处理IDS数据(swath)转化----Data文件格式
void DataTransfer::OnBnClickedTransferIds16()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

	CString strSrc; pEditSrc->GetWindowText(strSrc);
	CString strDst; pEditDst->GetWindowText(strDst);

	//输入的合法性
	if (strSrc.IsEmpty() || strDst.IsEmpty())
	{
		MessageBox("请选择输入和输出目录");
		return;
	}

	char pathSrc[512] = { 0 };
	char pathDst[512] = { 0 };

	strcpy(pathSrc, (char *)strSrc.GetBuffer(strSrc.GetLength()));
	strcpy(pathDst, (char *)strDst.GetBuffer(strDst.GetLength()));

	//MessageBox(_T(pathSrc));
	//const char *pathSrc = "C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\segy\\";  //"C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\欧美大地\\3D-Radar20240411数据\\SGY格式数据\\";  //
	//const char *pathDst = "C:\\3D_TestData\\RadarData\\";

	//3DRadar原始文件路径
	int length = (int)strlen(pathSrc);
	wchar_t* pathSrc2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathSrc2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathSrc, length, pathSrc2, length);

	//Impluse目标文件路径
	length = (int)strlen(pathDst);
	wchar_t* pathDst2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathDst2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathDst, length, pathDst2, length);

	int iResult = tranfersIDSData16((const char *)pathSrc2, (const char *)pathDst2 );
	char szResult[512] = { 0 };
	sprintf(szResult, "转化完成，返回结果:%d", iResult);
	MessageBox(szResult);
}

//处理DT数据(dt)转化----废弃
void DataTransfer::OnBnClickedTransferDt()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

	CString strSrc; pEditSrc->GetWindowText(strSrc);
	CString strDst; pEditDst->GetWindowText(strDst);

	//输入的合法性
	if (strSrc.IsEmpty() || strDst.IsEmpty())
	{
		MessageBox("请选择输入和输出目录");
		return;
	}

	char pathSrc[512] = { 0 };
	char pathDst[512] = { 0 };

	strcpy(pathSrc, (char *)strSrc.GetBuffer(strSrc.GetLength()));
	strcpy(pathDst, (char *)strDst.GetBuffer(strDst.GetLength()));

	//MessageBox(_T(pathSrc));
	//const char *pathSrc = "C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\segy\\";  //"C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\欧美大地\\3D-Radar20240411数据\\SGY格式数据\\";  //
	//const char *pathDst = "C:\\3D_TestData\\RadarData\\";

	//3DRadar原始文件路径
	int length = (int)strlen(pathSrc);
	wchar_t* pathSrc2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathSrc2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathSrc, length, pathSrc2, length);

	//Impluse目标文件路径
	length = (int)strlen(pathDst);
	wchar_t* pathDst2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathDst2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathDst, length, pathDst2, length);

	int iResult = tranfersDT((const char *)pathSrc2, 600, (float)0.2, (const char *)pathDst2);
	char szResult[512] = { 0 };
	sprintf(szResult, "转化完成，返回结果:%d", iResult);
	MessageBox(szResult);
}

//处理IDS数据(Scan)转化----Scan文件转化
void DataTransfer::OnBnClickedTransferIds()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

	CString strSrc; pEditSrc->GetWindowText(strSrc);
	CString strDst; pEditDst->GetWindowText(strDst);

	//输入的合法性
	if (strSrc.IsEmpty() || strDst.IsEmpty())
	{
		MessageBox("请选择输入和输出目录");
		return;
	}

	char pathSrc[512] = { 0 };
	char pathDst[512] = { 0 };

	strcpy(pathSrc, (char *)strSrc.GetBuffer(strSrc.GetLength()));
	strcpy(pathDst, (char *)strDst.GetBuffer(strDst.GetLength()));

	//MessageBox(_T(pathSrc));
	//const char *pathSrc = "C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\segy\\";  //"C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\欧美大地\\3D-Radar20240411数据\\SGY格式数据\\";  //
	//const char *pathDst = "C:\\3D_TestData\\RadarData\\";

	//3DRadar原始文件路径
	int length = (int)strlen(pathSrc);
	wchar_t* pathSrc2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathSrc2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathSrc, length, pathSrc2, length);

	//Impluse目标文件路径
	length = (int)strlen(pathDst);
	wchar_t* pathDst2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathDst2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathDst, length, pathDst2, length);

	int iResult = tranfersIDS((const char *)pathSrc2, (const char *)pathDst2, 600, (float)0.2);
	char szResult[512] = { 0 };
	sprintf(szResult, "转化完成，返回结果:%d", iResult);
	MessageBox(szResult);
}


//经典Mala数据（32位三维mcor、rd7，16位三维cor、rd3，16位二维R_*_B*.rd3）转化为iprh、iprb
void DataTransfer::OnBnClickedTransferMala()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

	CString strSrc; pEditSrc->GetWindowText(strSrc);
	CString strDst; pEditDst->GetWindowText(strDst);

	//输入的合法性
	if (strSrc.IsEmpty() || strDst.IsEmpty())
	{
		MessageBox("请选择输入和输出目录");
		return;
	}

	char pathSrc[512] = { 0 };
	char pathDst[512] = { 0 };

	strcpy(pathSrc, (char *)strSrc.GetBuffer(strSrc.GetLength()));
	strcpy(pathDst, (char *)strDst.GetBuffer(strDst.GetLength()));

	//MessageBox(_T(pathSrc));
	//const char *pathSrc = "C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\segy\\";  //"C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\欧美大地\\3D-Radar20240411数据\\SGY格式数据\\";  //
	//const char *pathDst = "C:\\3D_TestData\\RadarData\\";

	//3DRadar原始文件路径
	int length = (int)strlen(pathSrc);
	wchar_t* pathSrc2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathSrc2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathSrc, length, pathSrc2, length);

	//Impluse目标文件路径
	length = (int)strlen(pathDst);
	wchar_t* pathDst2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathDst2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathDst, length, pathDst2, length);

	int iResult = tranfersMala((const char *)pathSrc2, (const char *)pathDst2);
	char szResult[512] = { 0 };
	sprintf(szResult, "转化完成，返回结果:%d", iResult);
	MessageBox(szResult);
}

//一种奇怪Mala32位数据（32位二维雷达）转化为iprh、iprb
void DataTransfer::OnBnClickedTransferMalaEx()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

	CString strSrc; pEditSrc->GetWindowText(strSrc);
	CString strDst; pEditDst->GetWindowText(strDst);

	//输入的合法性
	if (strSrc.IsEmpty() || strDst.IsEmpty())
	{
		MessageBox("请选择输入和输出目录");
		return;
	}

	char pathSrc[512] = { 0 };
	char pathDst[512] = { 0 };

	strcpy(pathSrc, (char *)strSrc.GetBuffer(strSrc.GetLength()));
	strcpy(pathDst, (char *)strDst.GetBuffer(strDst.GetLength()));

	//MessageBox(_T(pathSrc));
	//const char *pathSrc = "C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\segy\\";  //"C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\欧美大地\\3D-Radar20240411数据\\SGY格式数据\\";  //
	//const char *pathDst = "C:\\3D_TestData\\RadarData\\";

	//3DRadar原始文件路径
	int length = (int)strlen(pathSrc);
	wchar_t* pathSrc2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathSrc2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathSrc, length, pathSrc2, length);

	//Impluse目标文件路径
	length = (int)strlen(pathDst);
	wchar_t* pathDst2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathDst2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathDst, length, pathDst2, length);

	int iResult = tranfersMala32Ex((const char *)pathSrc2, (const char *)pathDst2);
	char szResult[512] = { 0 };
	sprintf(szResult, "转化完成，返回结果:%d", iResult);
	MessageBox(szResult);
}

//欧美大地数据转化
void DataTransfer::OnBnClickedTransfer3dradar()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

	CString strSrc; pEditSrc->GetWindowText(strSrc);
	CString strDst; pEditDst->GetWindowText(strDst);

	//输入的合法性
	if (strSrc.IsEmpty() || strDst.IsEmpty())
	{
		MessageBox("请选择输入和输出目录");
		return;
	}

	char pathSrc[512] = { 0 };
	char pathDst[512] = { 0 };

	strcpy(pathSrc, (char *)strSrc.GetBuffer(strSrc.GetLength()));
	strcpy(pathDst, (char *)strDst.GetBuffer(strDst.GetLength()));

	//MessageBox(_T(pathSrc));
	//const char *pathSrc = "C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\segy\\";  //"C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\欧美大地\\3D-Radar20240411数据\\SGY格式数据\\";  //
	//const char *pathDst = "C:\\3D_TestData\\RadarData\\";

	//3DRadar原始文件路径
	int length = (int)strlen(pathSrc);
	wchar_t* pathSrc2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathSrc2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathSrc, length, pathSrc2, length);

	//Impluse目标文件路径
	length = (int)strlen(pathDst);
	wchar_t* pathDst2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathDst2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathDst, length, pathDst2, length);

	int iResult = tranfers3DRadar((const char *)pathSrc2, (const char *)pathDst2);
	char szResult[512] = { 0 };
	sprintf(szResult, "转化完成，返回结果:%d", iResult);
	MessageBox(szResult);
}

//Segy数据转化
void DataTransfer::OnBnClickedTransferSegy()
{
	CEdit *pEditSrc = (CEdit *)GetDlgItem(IDC_EDIT_INPUT);
	CEdit *pEditDst = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

	CString strSrc; pEditSrc->GetWindowText(strSrc);
	CString strDst; pEditDst->GetWindowText(strDst);

	//输入的合法性
	if (strSrc.IsEmpty() || strDst.IsEmpty())
	{
		MessageBox("请选择输入和输出目录");
		return;
	}

	char pathSrc[512] = { 0 };
	char pathDst[512] = { 0 };

	strcpy(pathSrc, (char *)strSrc.GetBuffer(strSrc.GetLength()));
	strcpy(pathDst, (char *)strDst.GetBuffer(strDst.GetLength()));

	//MessageBox(_T(pathSrc));
	//const char *pathSrc = "C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\segy\\";  //"C:\\D盘开发备份\\BAK.开发和学习\\AllDevelop\\3D系统\\数据\\欧美大地\\3D-Radar20240411数据\\SGY格式数据\\";  //
	//const char *pathDst = "C:\\3D_TestData\\RadarData\\";

	//3DRadar原始文件路径
	int length = (int)strlen(pathSrc);
	wchar_t* pathSrc2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathSrc2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathSrc, length, pathSrc2, length);

	//Impluse目标文件路径
	length = (int)strlen(pathDst);
	wchar_t* pathDst2 = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)pathDst2, 0, length * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pathDst, length, pathDst2, length);

	//MessageBox("OK?");
	//转化Segy数据
	int iResult = tranfersSegy((const char *)pathSrc2, (char *)pathDst2);
	//MessageBox( "Yes" );
	char szResult[512] = { 0 };
	sprintf(szResult, "转化完成，返回结果:%d", iResult);
	MessageBox(szResult);

	/*
	bool bResult = RadarMathInit();
	if (bResult)
		MessageBox(_T("Matlab OK"), _T("结果"), 0);
	else
		MessageBox(_T("Matlab NOK"), _T("结果"), 0);
	*/
}

//GSSI数据转化
void DataTransfer::OnBnClickedTransferGssi()
{
	MessageBox("暂不支持这种数据格式");
}

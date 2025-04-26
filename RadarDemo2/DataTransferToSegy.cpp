// DataTransferToSegy.cpp: 实现文件
//

#include "pch.h"
#include "RadarDemo2.h"
#include "DataTransferToSegy.h"
#include "afxdialogex.h"


// DataTransferToSegy 对话框

IMPLEMENT_DYNAMIC(DataTransferToSegy, CDialogEx)

DataTransferToSegy::DataTransferToSegy(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_TRANSFER_TOSEGY, pParent)
{

}

DataTransferToSegy::~DataTransferToSegy()
{
}

void DataTransferToSegy::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DataTransferToSegy, CDialogEx)
	ON_BN_CLICKED(IDC_TRANSFER_SPLIT, &DataTransferToSegy::OnBnClickedTransferSplit)
END_MESSAGE_MAP()


// DataTransferToSegy 消息处理程序
extern "C" __declspec(dllimport) int tranfersSegySplit(const char * pathFile, const char * pathDst);


void DataTransferToSegy::OnBnClickedTransferSplit()
{
	char pathSrc[512] = { 0 }; strcpy(pathSrc, "C:\\3D_TestData\\2024-11-28-004 - Region1-124.sgy");
	char pathDst[512] = { 0 }; strcpy(pathDst, "C:\\3D_TestData\\segy\\");

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

	//拆分
	tranfersSegySplit((const char *)pathSrc2, (const char *)pathDst2);
}

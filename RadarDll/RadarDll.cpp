// RadarDll.cpp: 定义 DLL 的初始化例程。
//


#include "framework.h"
#include <io.h>
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

//#include "FSize.h"

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


#include "Project.h"
#include "RadarDll.h"
#include "RadarMath.h"

//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为以下项中的第一个语句:
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CRadarDllApp

BEGIN_MESSAGE_MAP(CRadarDllApp, CWinApp)
END_MESSAGE_MAP()


// CRadarDllApp 构造

CRadarDllApp::CRadarDllApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}
CRadarDllApp::~CRadarDllApp()
{
	//释放Matlab
	mclTerminateApplication();
}

// 唯一的 CRadarDllApp 对象

CRadarDllApp theApp;


// CRadarDllApp 初始化

BOOL CRadarDllApp::InitInstance()
{
	CWinApp::InitInstance();

	//Swath数据加載完成标志
	m_SwathLoaded = FALSE;

	//设定色板的默认值
	m_Color = 1;
	m_ColorMask = 0;

	//Matlab初始化，必須的一步
	mclInitializeApplication(NULL, 0);

	return TRUE;
}

/*
Fun:将通道数据转存为图片文件----垂直图片
Parameter：
	strPathFile，存储的目标文件，带有路径
	pChannel，   包含有一个完整数据的通道对象
	iStart，     开始TraceNum
	iCount，     需要绘制的Trace数量
*/
int CRadarDllApp::SaveAsPicV(CString strPathFile, SwathChannel * pChannel, int iStart, int iCount, int iHeight)
{
	//目标文件需要确定，根据路径和文件名长度简单判断一下
	if (strPathFile.GetLength() <= 4)
		return -1;
	//需要有确定的通道来源数据
	if (!pChannel)
		return -2;

	//获取通道数据的深度和宽度
	int iWidth = pChannel->getChannelHeader()->getTraceCount();
	//int iHeight = pChannel->getChannelHeader()->getSample();
	if (iHeight > pChannel->getChannelHeader()->getSample())
		iHeight = pChannel->getChannelHeader()->getSample();

	//使用用户设定的对比度和增益数据，设置显示系数
	pChannel->getChannelHeader()->setCoef(m_Contrast, m_Gain);
	//读取显示系数
	double* tgCoef = pChannel->getChannelHeader()->getCoef();

	//===================构造一个测线的完整视图开始==================//
	//构造位图信息头(只构造显示区域的位图，超出部分不构造)
	BITMAPINFOHEADER oBIHSideView;
	memset(&oBIHSideView, 0, sizeof(oBIHSideView));
	oBIHSideView.biSize = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
	oBIHSideView.biHeight = iHeight;                     //位图的高度，以像素为单位
	oBIHSideView.biWidth = iWidth;                      //位图的宽度，以像素为单位
	oBIHSideView.biPlanes = 1;                           //位平面数，必须为1
	oBIHSideView.biBitCount = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHSideView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHSideView.biSizeImage = iHeight * iWidth * 4;        //位图的大小，以字节为单位

	//调色板
	PalSET palette;
	//设置颜色
	palette.setColor((PaletteColor)m_Color, m_ColorMask);
	//分配像素空间，侧视图数据
	COLORREF* pPicSideView = new COLORREF[iHeight * iWidth];

	//获取需要的Trace像素数据
	std::map<int, Trace16*>* pTraceData = pChannel->getChannelBlob()->getTraceData16();

	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	//循环将像素数据转化为BMP(一列一列处理)
	for (int i = iStart; i < iWidth; i++)
	{
		Trace16* pTraceTmp = (*pTraceData)[i];
		if (NULL == pTraceTmp)
			break;

		short* pData = pTraceTmp->getTrace();
		for (int jj = 0; jj < iHeight; jj++)
		{
			short sColor = int(pData[jj] * tgCoef[jj]) / 2048 + 64;

			if (sColor < 0)
				sColor = 0;
			else if (sColor > 127)
				sColor = 127;

			COLORREF* pBuf = pPicSideView + iWidth * (iHeight - jj - 1) + i;
			*pBuf = palette.getColorref()[sColor];
		}
	}

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->unloadTrace();

	//===================构造测线的完整视图完成==================//

	//创建 目标 图形的内存DC
	CClientDC oDCTmp(NULL);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView, CBM_INIT, pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;
	oDCImage.CreateCompatibleDC(&oDCTmp);                     //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象

	//保存图形成为文件
	CImage image;
	image.Create(iCount, iHeight, 32);
	BitBlt(image.GetDC(), 0, 0, iCount, iHeight, oDCImage.m_hDC, iStart, 0, SRCCOPY);
	HRESULT hResult = image.Save(strPathFile);
	image.ReleaseDC();

	//释放空间
	oBitmap.DeleteObject();
	DeleteObject(hBitmap);

	//释放BMP像素空间
	delete[] pPicSideView;

	return 0;
}

/*
Fun:将通道数据转存为图片文件----水平图片
Parameter：
	strPathFile，存储的目标文件，带有路径
	pChannel，   包含有一个完整数据的通道对象
	iStart，     开始TraceNum
	iCount，     需要绘制的Trace数量
*/
int CRadarDllApp::SaveAsPicH(CString strPathFile, Swath * pSwath, int iDeep, int iStart, int iCount)
{
	//目标文件需要确定，根据路径和文件名长度简单判断一下
	if (strPathFile.GetLength() <= 4)
		return -1;
	//需要有确定的通道来源数据
	if (!pSwath)
		return -2;

	//获取通道数目
	int iChannelCount = 0;
	pSwath->getChannelCount(iChannelCount);
	//俯视图数据显示的总高度和宽度
	int iDataDisplayWidth = iCount;

	//构造单个俯视图的位图信息头(俯视图长宽大小都一样，所以构造一个即可)
	BITMAPINFOHEADER oBIHTopView;
	memset(&oBIHTopView, 0, sizeof(oBIHTopView));
	oBIHTopView.biSize = sizeof(BITMAPINFOHEADER);        //本结构所占用字节数40字节
	oBIHTopView.biHeight = (iChannelCount - 1) * 3 + 1;   //位图的高度，以像素为单位
	oBIHTopView.biWidth = iDataDisplayWidth;              //位图的宽度，以像素为单位
	oBIHTopView.biPlanes = 1;                             //位平面数，必须为1
	oBIHTopView.biBitCount = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHTopView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHTopView.biSizeImage = ((iChannelCount - 1) * 3 + 1) * iDataDisplayWidth * 4;      //位图的大小，以字节为单位

	//获取得到第一个Channel
	SwathChannel* pChannel = pSwath->getChannel(1);
	//计算需要显示俯视图的深度(cm)
	if (iDeep > pChannel->getChannelHeader()->getSample())
		iDeep = pChannel->getChannelHeader()->getSample();
	if (iDeep < 0)
		iDeep = 0;

	//调色板
	PalSET palette;
	palette.setColor((PaletteColor)m_Color, m_ColorMask);
	//每个平面的颜色数据，分配颜色空间数据（各个平面重复使用）
	COLORREF* pPicTopView = new COLORREF[((iChannelCount - 1) * 3 + 1) * iDataDisplayWidth];

	//为每个俯视图平面分配空间，并且在分配的俯视图颜色空间上填充数据
	//x:平行道路水平向右
	//y:垂直道路水平方向

	//加载所有通道数据
	for (int y = 0; y < iChannelCount; y++)
	{
		//读取对应通道的系数
		SwathChannel* pChannel = pSwath->getChannel(y + 1);

		//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
		pChannel->getChannelBlob()->loadTrace();
	}


	//下面从channel中取出数据填充到Buff
	for (int y = 0; y < iChannelCount; y++)
	{
		//读取对应通道的系数
		SwathChannel* pChannel = pSwath->getChannel(y + 1);

		//使用用户设定的对比度和增益数据，设置显示系数
		pChannel->getChannelHeader()->setCoef(m_Contrast, m_Gain);
		double* tgCoef = pChannel->getChannelHeader()->getCoef();

		//获取需要的Trace像素数据
		std::map<int, Trace16*> *pTraceData = pChannel->getChannelBlob()->getTraceData16();

		//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
		//pChannel->getChannelBlob()->loadTrace();

		int m_iWidth = pChannel->getChannelBlob()->getTraceCount();
		//获取需要的Trace像素数据
		for (int x = iStart; x < (int)(iStart + iDataDisplayWidth); x++)
		{
			Trace16 *pData = (*pTraceData)[x];
			if (NULL == pData)
				break;
			if (x > m_iWidth)
				break;

			short sColor = (short)(pData->getTrace()[iDeep] * tgCoef[iDeep]) / 2048 + 64;
			if (sColor < 0)
				sColor = 0;
			else if (sColor > 128)
				sColor = 128;

			*(pPicTopView + 3 * y * iDataDisplayWidth + x - iStart) = palette.getColorref()[sColor];
		}

		//========================//
		if (y >= iChannelCount - 1)
			continue;

		//读取对应通道的系数
		SwathChannel* pChannel2 = pSwath->getChannel(y + 1 + 1);

		//使用用户设定的对比度和增益数据，设置显示系数
		pChannel2->getChannelHeader()->setCoef(m_Contrast, m_Gain);
		double* tgCoef2 = pChannel2->getChannelHeader()->getCoef();

		//获取需要的Trace像素数据
		std::map<int, Trace16*> *pTraceData2 = pChannel2->getChannelBlob()->getTraceData16();
		//插入構造的数据
		for (int x = iStart; x < (int)(iStart + iDataDisplayWidth); x++)
		{
			Trace16 *pData1 = (*pTraceData)[x];
			if (NULL == pData1)
				break;
			Trace16 *pData2 = (*pTraceData2)[x];
			if (NULL == pData2)
				break;
			if (x > m_iWidth)
				break;

			short sColor1 = (short)(pData1->getTrace()[iDeep] * tgCoef[iDeep]) / 2048 + 64;
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

			*(pPicTopView + (3 * y + 1) * iDataDisplayWidth + x - iStart) = palette.getColorref()[sColor];
			*(pPicTopView + (3 * y + 2) * iDataDisplayWidth + x - iStart) = palette.getColorref()[sColor];
		}
		//unload图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
		//pChannel->getChannelBlob()->unloadTrace();
	}

	//释放所有通道数据
	for (int y = 0; y < iChannelCount; y++)
	{
		//读取对应通道的系数
		SwathChannel* pChannel = pSwath->getChannel(y + 1);

		//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
		pChannel->getChannelBlob()->unloadTrace();
	}
	//===================构造测线的完整视图完成==================//

	//创建 目标 图形的内存DC
	CClientDC oDCTmp(NULL);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHTopView, CBM_INIT, pPicTopView, (LPBITMAPINFO)&oBIHTopView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;
	oDCImage.CreateCompatibleDC(&oDCTmp);                     //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象

	//保存图形成为文件
	CImage image;
	image.Create(iCount, ((iChannelCount - 1) * 3 + 1), 32);
	BitBlt(image.GetDC(), 0, 0, iCount, ((iChannelCount - 1) * 3 + 1), oDCImage.m_hDC, 0, 0, SRCCOPY);
	HRESULT hResult = image.Save(strPathFile);
	image.ReleaseDC();

	//释放BMP像素空间
	delete[] pPicTopView;

	return 0;
}

/*
Fun:根据测线名，Trace号，获取当是的抓拍图像
Param:
	pProject   工程对象指针
	picFileFront    目标图像路径---前置摄像头抓拍文件
	picFileBack     目标图像路径---后置摄像头抓拍文件
	picFileLeft     目标图像路径---左置摄像头抓拍文件
	picFileRight    目标图像路径---右置摄像头抓拍文件
	swathName  测线名
	traceNum   Trace号
Return: 成功返回0，失败对应的错误码
*/
int CRadarDllApp::TakePhotoPic(Project *pProject, char * picFileFront, char * picFileBack, char * picFileLeft, char * picFileRight, Swath* swath, int traceNum, int iOffsetTime)
{
	if (!picFileFront)
		return -1;
	if (!pProject)
		return -2;
	if (!swath)
		return -3;

	if (!picFileFront)
		return -1;
	if (!pProject)
		return -2;
	if (!swath)
		return -3;
	SwathChannel* pChannel = swath->getChannel(1);
	if (!pChannel)
		return -4;
	SwathChannelHeader*  pHeader = pChannel->getChannelHeader();
	if (!pHeader)
		return -4;
	int iTraceCount = pHeader->getTraceCount();
	//沒有這个TraceNum
	if (iTraceCount <= traceNum)
		return 0;

	//雷达扫描时间
	SwathTimeData *timeData = swath->getSwathTime()->getData(traceNum);
	if (!timeData)
		return -5;
	char szPictureTime[64] = { 0 };
	sprintf(szPictureTime, "%s %s", timeData->getDateString(), timeData->getTimeString());
	//将时间转化为北京时间,同时增加一个时间偏移量
	TransferTime(szPictureTime, iOffsetTime);

	int iResult = 0;

	//取得抓拍照片文件名-前
	char szPicturePathFileFront[512] = { 0 };
	char szPicturePathFileBack[512] = { 0 };
	char szPicturePathFileLeft[512] = { 0 };
	char szPicturePathFileRight[512] = { 0 };

	iResult = pProject->getCameraFront()->getPictureByTime(szPictureTime, szPicturePathFileFront);  strcpy(picFileFront, szPicturePathFileFront);
	iResult = pProject->getCameraBack()->getPictureByTime(szPictureTime, szPicturePathFileBack);    strcpy(picFileBack, szPicturePathFileBack);
	iResult = pProject->getCameraLeft()->getPictureByTime(szPictureTime, szPicturePathFileLeft);    strcpy(picFileLeft, szPicturePathFileLeft);
	iResult = pProject->getCameraRight()->getPictureByTime(szPictureTime, szPicturePathFileRight);  strcpy(picFileRight, szPicturePathFileRight);

	return iResult;
}

/*
Fun:初始化，读取指定测线和通道数据
Param:pSwath测线对象
*/
int CRadarDllApp::InitSwath(Swath * pSwath)
{
	//测线已經被加載，無需再次加載
	if (m_SwathLoaded)
		return -1;

	//获取通道数目
	int iChannelCount = 0;
	pSwath->getChannelCount(iChannelCount);

	//下面从channel中取出数据填充到Buff
	for (int y = 0; y < iChannelCount; y++)
	{
		//读取对应通道的系数
		SwathChannel* pChannel = pSwath->getChannel(y + 1);
		//load图像数据
		pChannel->getChannelBlob()->loadTrace();
	}

	m_SwathLoaded = TRUE;

	return 0;
}

/*
Fun:去初始化，读取指定测线和通道数据
Param:pSwath测线对象
*/
int CRadarDllApp::UnInitSwath(Swath * pSwath)
{
	//测线已經被卸載，無需再次卸載
	if (!m_SwathLoaded)
		return -1;

	//获取通道数目
	int iChannelCount = 0;
	pSwath->getChannelCount(iChannelCount);

	//下面从channel中取出数据填充到Buff
	for (int y = 0; y < iChannelCount; y++)
	{
		//读取对应通道的系数
		SwathChannel* pChannel = pSwath->getChannel(y + 1);
		//unload图像数据
		pChannel->getChannelBlob()->unloadTrace();
	}

	m_SwathLoaded = FALSE;

	return 0;
}

/*
//备份原有的测线通道数据
bool CRadarDllApp::BackupData(char *fileData, char *fileHeader, char *path)
{
	//输入参数
	mwArray STR_FILE_DATA(fileData);
	mwArray STR_FILE_HEAD(fileHeader);
	mwArray STR_FILE_PATH(path);

	//备份原有的测线通道数据
	IBackup(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileData:%s; fileHeader:%s; path:%s", fileData, fileHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);

	return true;
}
*/

/*
* Fun：删除无效Trace
* Param：
*      szSwathName  测线名
*      path         存储目录
*      pSwath       测线对象
* Return：成功返回TRUE，失败返回FALSE
*/
bool CRadarDllApp::deleteInvalidTrace(const char * pSwathName, int channelID, const char *path, Swath * pSwath)
{
	//构造存储路径和文件名
	char szPathFileIPRB[512] = { 0 };
	char szPathFileIPRH[512] = { 0 };
	sprintf(szPathFileIPRB, "%s\\%s_A%02d.iprb", path, pSwathName, channelID);
	sprintf(szPathFileIPRH, "%s\\%s_A%02d.iprh", path, pSwathName, channelID);

	//获取对应的通道对象
	SwathChannel* pChannel = pSwath->getChannel(channelID);
	if (!pChannel)
		return false;

	//取得通道对象的数据
	SwathChannelBlob * channelBlob = pChannel->getChannelBlob();
	if (!channelBlob)
		return false;

	//取得通道对象的头信息
	SwathChannelHeader * channelHeader = pChannel->getChannelHeader();
	if (!channelHeader)
		return false;

	//装入数据
	channelBlob->loadTrace();
	int dataType = channelHeader->getDataVersion();
	//删除无效Trace
	int deleteCount = DeleteInvalidTrace(channelBlob, dataType);
	if (0 == deleteCount)
	{
		channelBlob->unloadTrace();
		return false;
	}

	//存储数据文件
	channelBlob->saveTrace(szPathFileIPRB);
	//卸载数据
	channelBlob->unloadTrace();

	//设置Trace数量
	int count = channelHeader->getTraceCount();
	channelHeader->setTraceCount(count - deleteCount);
	//存储通道头
	channelHeader->saveHeader(szPathFileIPRH);

	return true;
}




//==========================重新生成一组接口函数，用于开源计算==========================//
/*
	Fun:去直达波--产生直达波图形
	Parameter：
		pChannel，   channel对象
		filePic，    存储直达波图像的目标文件，带有路径
		traceNum，   指定直达波的Trace号
*/
int CRadarDllApp::SigPosPicureByVende(SwathChannel * pChannel, char *filePic, int traceNum)
{
	//目标文件需要确定，根据路径和文件名长度简单判断一下
	if (strlen(filePic) <= 4)
		return -1;
	//需要有确定的通道来源数据
	if (!pChannel)
		return -2;

	//获取通道数据的深度和宽度
	int iWidth = pChannel->getChannelHeader()->getTraceCount();
	int iHeight = pChannel->getChannelHeader()->getSample();

	//判断参数是否溢出
	if (traceNum >= iWidth)
		return -3;

	//获取所有的Trace数据
	std::map<int, Trace16*>* pTraceData = pChannel->getChannelBlob()->getTraceData16();
	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	//===================构造一个测线的完整视图开始===============//
	//构造位图信息头(只构造显示区域的位图，超出部分不构造)
	BITMAPINFOHEADER oBIHSideView;
	memset(&oBIHSideView, 0, sizeof(oBIHSideView));
	oBIHSideView.biSize = sizeof(BITMAPINFOHEADER);    //本结构所占用字节数40字节
	oBIHSideView.biHeight = iHeight;                     //位图的高度，以像素为单位---1000像素
	oBIHSideView.biWidth = 300;                         //位图的宽度，以像素为单位---300像素
	oBIHSideView.biPlanes = 1;                           //位平面数，必须为1
	oBIHSideView.biBitCount = 32;                          //每个像素所需的位数，必须是1（双色）、4（16色）、8（256色）或24（真彩色）之一
	oBIHSideView.biCompression = BI_RGB;                      //位图压缩类型，必须是 0（BI_RGB不压缩）、1（BI_RLE8压缩类型）或2（BI_RLE压缩类型）之一
	oBIHSideView.biSizeImage = iHeight * 300 * 4;           //位图的大小，以字节为单位
	//分配像素空间，侧视图数据
	COLORREF* pPicSideView = new COLORREF[iHeight * 300];
	memset((unsigned char *)pPicSideView, 0xff, sizeof(COLORREF) * iHeight * 300);    //初始化为白色背景

	//===================构造测线的完整视图完成==================//
	//创建 目标 图形的内存DC
	CClientDC oDCTmp(NULL);                                   //m_hwnd 创建客户区绘制内存
	HBITMAP hBitmap = CreateDIBitmap(oDCTmp.m_hDC, &oBIHSideView, CBM_INIT, pPicSideView, (LPBITMAPINFO)&oBIHSideView, DIB_RGB_COLORS);
	CBitmap oBitmap; oBitmap.Attach(hBitmap);                 //关联位图对象
	CDC oDCImage;
	oDCImage.CreateCompatibleDC(&oDCTmp);                     //内存DC
	oDCImage.SelectObject(&oBitmap);                          //选取位图对象

	//最大的数据和最小的数据
	int dataMax = 0;
	int dataMin = 0;
	int dataWidth = 0; //数据宽度


	//构造一个Trace,该Trace得到原有各个Trace的平均值，然后用于计算直达波
	Trace16 oTraceDst;
	short* pDataDst = (short *)malloc(iHeight * 2);    //此处分配的空间会在Trace16中自动释放
	oTraceDst.setTrace(pDataDst, iHeight);

	//记录参与统计的Trace数量
	int iCount = 0;
	int iSum[1024] = { 0 };
	for (int j = traceNum; j < iWidth; j++)
	{
		//记录一个Trace总强度的合计值
		int traceValueSum = 0;

		for (int i = 0; i < iHeight; i++)
		{
			Trace16* pTraceTmp = (*pTraceData)[j];
			iSum[i] = iSum[i] + pTraceTmp->getTrace()[i];

			//统计一个Trace总强度的合计
			traceValueSum = traceValueSum + abs(pTraceTmp->getTrace()[i]);
		}
		//强度达到一定程度，我们认为是有效值
		if (traceValueSum > 200)
			iCount++;

		//总的Trace数量超过256，就不用计算了
		if (iCount > 256)
			break;
	}
	for (int i = 0; i < iHeight; i++)
	{
		if (iCount > 0)
			//计算平均值
			pDataDst[i] = (short)(iSum[i] / iCount);
		else
		{
			Trace16* pTraceTmp = (*pTraceData)[traceNum];
			pDataDst[i] = pTraceTmp->getTrace()[i];
		}
	}

	//获取一个trace的数据
	//Trace16* pTraceTmp = (*pTraceData)[traceNum];
	//short* pData = pTraceTmp->getTrace();
	short* pData = oTraceDst.getTrace();
	for (int ii = 0; ii < iHeight; ii++)
	{
		int sColor = pData[ii];

		if (sColor > dataMax)
			dataMax = sColor;
		if (sColor < dataMin)
			dataMin = sColor;
	}
	dataWidth = dataMax - dataMin;

	//制图--画线方式
	for (int i = 0; i < iHeight; i++)
	{
		int sColor = pData[i];

		unsigned int position = (sColor - dataMin) * 300 / dataWidth;
		for (int j = 0; j < 300; j++)
		{
			if ((i == 0) && (j == position))
			{
				//绘制曲线
				POINT oPoint;
				oPoint.x = j; oPoint.y = 0;
				oDCImage.MoveTo(oPoint);
			}
			else if ((i != 0) && (j == position))
			{
				//oDCImage.SetDCPenColor(0x0);
				//每个点之间画线
				POINT oPoint;
				oPoint.x = j; oPoint.y = i;
				oDCImage.LineTo(oPoint);
			}
		}
	}

	//===================保存图形成为文件==================//
	//保存图形成为文件
	CImage image;
	image.Create(150, 1000, 32);
	//BitBlt(image.GetDC(), 0, 0, 150, 1000, oDCImage.m_hDC, 0, 0, SRCCOPY);   //copy图像
	StretchBlt(image.GetDC(), 0, 0, 150, 1000, oDCImage.m_hDC, 0, 0, 300, iHeight, SRCCOPY);   //copy图像，同时对图像进行缩放
	HRESULT hResult = image.Save(filePic);
	image.ReleaseDC();

	//释放空间
	oBitmap.DeleteObject();
	DeleteObject(hBitmap);

	//释放BMP像素空间
	delete[] pPicSideView;

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->unloadTrace();

	return 0;
}

/*
	Fun:去直达波--获取直达波位置
	Parameter：
	  pChannel，   channel对象
	  direct       直达波方向 -1：左边直达波的波峰； 0：两边直达波的波峰； 1：右边直达波的波峰；
	  waveNum      指定从上向下的第几个波峰
*/
int CRadarDllApp::SigPosNumByVende(SwathChannel * pChannel, int direct, int waveNum)
{
	//需要有确定的通道来源数据
	if (!pChannel)
		return -2;

	//获取通道数据的深度和宽度
	int iWidth  = pChannel->getChannelHeader()->getTraceCount();
	int iHeight = pChannel->getChannelHeader()->getSample();

	int traceNum = 0;
	//判断参数是否溢出
	if (0 == iWidth)
		return -3;

	//获取所有的Trace数据
	std::map<int, Trace16*>* pTraceData = pChannel->getChannelBlob()->getTraceData16();
	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	//最大的数据和最小的数据
	int dataMax = 0;
	int dataMin = 0;

	//构造一个Trace,该Trace得到原有各个Trace的平均值，然后用于计算直达波
	Trace16 oTraceDst;
	short* pDataDst = (short *)malloc(iHeight * 2);    //此处分配的空间会在Trace16中自动释放
	oTraceDst.setTrace(pDataDst, iHeight);

	//记录参与统计的Trace数量
	int iCount = 0;
	int iSum[1024] = { 0 };
	for (int j = traceNum; j < iWidth; j++)
	{
		//记录一个Trace总强度的合计值
		int traceValueSum = 0;

		for (int i = 0; i < iHeight; i++)
		{
			Trace16* pTraceTmp = (*pTraceData)[j];
			iSum[i] = iSum[i] + pTraceTmp->getTrace()[i];

			//统计一个Trace总强度的合计
			traceValueSum = traceValueSum + abs(pTraceTmp->getTrace()[i]);
		}
		//强度达到一定程度，我们认为是有效值
		if (traceValueSum > 200)
			iCount++;

		//总的Trace数量超过256，就不用计算了
		if (iCount > 256)
			break;
	}
	for (int i = 0; i < iHeight; i++)
	{
		if (iCount > 0)
			//计算平均值
			pDataDst[i] = (short)(iSum[i] / iCount);
		else
		{
			Trace16* pTraceTmp = (*pTraceData)[traceNum];
			pDataDst[i] = pTraceTmp->getTrace()[i];
		}
	}

	//获取一个trace的数据
	//Trace16* pTraceTmp = (*pTraceData)[traceNum];
	//short* pData = pTraceTmp->getTrace();
	short* pData = oTraceDst.getTrace();
	for (int ii = 0; ii < iHeight; ii++)
	{
		//这里循环获取最大的波峰
		int sColor = pData[ii];

		if (sColor > dataMax)
		{
			dataMax = sColor;
		}
		if (sColor < dataMin)
		{
			dataMin = sColor;
		}
	}

	//int direct = 0;
	//================第一个直达波的波峰获取--开始=============//
	//记录直达波的位置
	int sigPosition = 0;
	//取波峰
	for (int ii = 1; ii < iHeight - 2; ii++)
	{
		int tmpData = pData[ii];

		//如果波峰向右，那么不取左边数据---切左边
		if ((1 == direct) && (pData[ii] < 0))
			continue;
		//如果波峰向左，那么不取右边数据---切右边
		if ((-1 == direct) && (pData[ii] > 0))
			continue;

		int dataSig = abs(pData[ii]);
		//读取得到的数据，还没有最大数据的四分之一，所以不作为可用数据
		if ((dataSig < abs(dataMax / 4)) || (dataSig < abs(dataMin / 4)))
			continue;

		//前一个点
		int dataPrev = abs(pData[ii - 1]);
		//后一个点
		int dataNext = abs(pData[ii + 1]);

		/*
		//寻找前一个元素的前一个元素
		int dataPrevPrev = 0;
		for (int j = ii - 2; j >= 1; j--)
		{
			if (pData[j] != pData[ii - 1])
			{
				dataPrevPrev = pData[j];
				break;
			}
		}
		//寻找后一个元素的后一个
		int dataNextNext = 0;
		for (int j = ii + 2; j < iHeight - 2; j++)
		{
			if (pData[j] != pData[ii + 1])
			{
				dataNextNext = pData[j];
				break;
			}
		}
		*/

		//如果不符合条件，那么直接找下一个数据
		if ((dataPrev > dataSig) || (dataNext > dataSig))
			continue;

		//取到第一个波峰
		if ((dataPrev < dataSig) && (dataNext < dataSig))
		{
			sigPosition = ii;
		}

		//
		waveNum--;

		if (waveNum <= 0)
			break;
	}
	//================第一个直达波的波峰获取--完成=============//

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->unloadTrace();

	return sigPosition;
}

/*
	Fun:去直达波--获取直达波位置---取最大波峰的位置
	Parameter：
		pChannel，   channel对象
		traceNum，   指定直达波的Trace号
		direct,      直达波方向
		sigPos,      输出参数，直达波位置
*/
int CRadarDllApp::SigPosNumExByVende(SwathChannel * pChannel, int traceNum, int direct)
{
	//需要有确定的通道来源数据
	if (!pChannel)
		return -2;

	//获取通道数据的深度和宽度
	int iWidth = pChannel->getChannelHeader()->getTraceCount();
	int iHeight = pChannel->getChannelHeader()->getSample();

	//判断参数是否溢出
	if (traceNum >= iWidth)
		return -3;

	//获取所有的Trace数据
	std::map<int, Trace16*>* pTraceData = pChannel->getChannelBlob()->getTraceData16();
	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	//记录左边和右边的波峰值
	int left = 0;
	int right = 0;

	//最大的数据和最小的数据
	int dataMax = 0;
	int dataMin = 0;

	//获取一个trace的数据
	Trace16* pTraceTmp = (*pTraceData)[traceNum];
	short* pData = pTraceTmp->getTrace();
	for (int ii = 0; ii < iHeight; ii++)
	{
		int sColor = pData[ii];

		if (sColor > dataMax)
		{
			right = ii;
			dataMax = sColor;
		}
		if (sColor < dataMin)
		{
			left = ii;
			dataMin = sColor;
		}
	}

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->unloadTrace();

	if (1 == direct)	//如果是1，则取右边的直达波
		return right;
	else if (-1 == direct)	//如果是-1，则取左边的直达波
		return left;
	else if (0 == direct)
		return abs(dataMin) > dataMax ? left : right;
	else
		return -4;
}

//去直达波--裁剪直达波图形
bool CRadarDllApp::SimpleSigPositionCut(char *fileData, char *fileHeader, char *path, SwathChannel * pChannel, int iZero)
{
	//需要有确定的通道来源数据
	if (!pChannel)
		return false;

	//获取通道数据的深度和宽度
	int iWidth = pChannel->getChannelHeader()->getTraceCount();
	int iHeight = pChannel->getChannelHeader()->getSample();

	//判断参数是否溢出
	if (iZero >= iHeight)
		return false;

	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	char szFileHeader[512] = { 0 }; sprintf(szFileHeader, "%s%s", path, fileHeader);
	char szFileData[512] = { 0 }; sprintf(szFileData, "%s%s", path, fileData);

	//更新采样点数
	pChannel->getChannelHeader()->setSample(pChannel->getChannelHeader()->getSample() - iZero);
	//写入Header文件
	pChannel->getChannelHeader()->saveHeader(szFileHeader);
	//裁剪数据
	pChannel->getChannelBlob()->cutTraceData(iZero);
	//写入数据文件
	pChannel->getChannelBlob()->saveTrace(szFileData);

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->unloadTrace();

	//直达波取绝对值最大的一个
	return true;
}

/*
	Fun:去直达波--获取直达波位置
	Parameter：
		pChannel，   channel对象
		traceNum，   指定直达波的Trace号
		waveNum,     返回从上向下数，对应波的位置
	例如：
	下面参数表示在第一通道，第一Trace中或取直达波的位置，并且从上向下第二个波峰
	pChannel = 1
	traceNum = 1
	waveNum  = 2
*/
///////这个函数暂时屏蔽
int CRadarDllApp::MiniSigPosNum2(SwathChannel * pChannel, int traceNum, int waveNum)
{
	//需要有确定的通道来源数据
	if (!pChannel)
		return -2;

	//获取通道数据的深度和宽度
	int iWidth = pChannel->getChannelHeader()->getTraceCount();
	int iHeight = pChannel->getChannelHeader()->getSample();

	//判断参数是否溢出
	if (traceNum >= iWidth)
		return -3;

	//获取所有的Trace数据
	std::map<int, Trace16*>* pTraceData = pChannel->getChannelBlob()->getTraceData16();
	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	//最大的数据和最小的数据
	int dataMax = 0;
	int dataMin = 0;

	//构造一个Trace,该Trace得到原有各个Trace的平均值，然后用于计算直达波
	Trace16 oTraceDst;
	short* pDataDst = (short *)malloc(iHeight * 2);    //此处分配的空间会在Trace16中自动释放
	oTraceDst.setTrace(pDataDst, iHeight);

	//记录参与统计的Trace数量
	int iCount = 0;
	int iSum[1024] = { 0 };
	for (int j = traceNum; j < iWidth; j++)
	{
		//记录一个Trace总强度的合计值
		int traceValueSum = 0;

		for (int i = 0; i < iHeight; i++)
		{
			Trace16* pTraceTmp = (*pTraceData)[j];
			iSum[i] = iSum[i] + pTraceTmp->getTrace()[i];

			//统计一个Trace总强度的合计
			traceValueSum = traceValueSum + abs(pTraceTmp->getTrace()[i]);
		}
		//强度达到一定程度，我们认为是有效值
		if (traceValueSum > 200)
			iCount++;

		//总的Trace数量超过256，就不用计算了
		if (iCount > 256)
			break;
	}
	for (int i = 0; i < iHeight; i++)
	{
		if (iCount > 0)
			//计算平均值
			pDataDst[i] = (short)(iSum[i] / iCount);
		else
		{
			Trace16* pTraceTmp = (*pTraceData)[traceNum];
			pDataDst[i] = pTraceTmp->getTrace()[i];
		}
	}

	//获取一个trace的数据
	//Trace16* pTraceTmp = (*pTraceData)[traceNum];
	//short* pData = pTraceTmp->getTrace();
	short* pData = oTraceDst.getTrace();
	for (int ii = 0; ii < iHeight; ii++)
	{
		//这里循环获取最大的波峰
		int sColor = pData[ii];

		if (sColor > dataMax)
		{
			dataMax = sColor;
		}
		if (sColor < dataMin)
		{
			dataMin = sColor;
		}
	}

	int direct = 0;
	//================第一个直达波的波峰获取--开始=============//
	//记录直达波的位置
	int sigPosition = 0;
	//取波峰
	for (int ii = 1; ii < iHeight - 2; ii++)
	{
		int tmpData = pData[ii];

		//如果波峰向右，那么不取左边数据---切左边
		if ((1 == direct) && (pData[ii] < 0))
			continue;
		//如果波峰向左，那么不取右边数据---切右边
		if ((-1 == direct) && (pData[ii] > 0))
			continue;

		int dataSig = abs(pData[ii]);
		//读取得到的数据，还没有最大数据的四分之一，所以不作为可用数据
		if ((dataSig < abs(dataMax / 4)) || (dataSig < abs(dataMin / 4)))
			continue;

		//前一个点
		int dataPrev = abs(pData[ii - 1]);
		//后一个点
		int dataNext = abs(pData[ii + 1]);

		/*
		//寻找前一个元素的前一个元素
		int dataPrevPrev = 0;
		for (int j = ii - 2; j >= 1; j--)
		{
			if (pData[j] != pData[ii - 1])
			{
				dataPrevPrev = pData[j];
				break;
			}
		}
		//寻找后一个元素的后一个
		int dataNextNext = 0;
		for (int j = ii + 2; j < iHeight - 2; j++)
		{
			if (pData[j] != pData[ii + 1])
			{
				dataNextNext = pData[j];
				break;
			}
		}
		*/

		//如果不符合条件，那么直接找下一个数据
		if ((dataPrev > dataSig) || (dataNext > dataSig))
			continue;

		//取到第一个波峰
		if ((dataPrev < dataSig) && (dataNext < dataSig))
		{
			sigPosition = ii;
		}
		//如果还不知道波峰在哪边就首先判断波峰的方向
		if (0 == direct)
		{
			//这判断 第一个波峰 是左边还是右边
			if (pData[ii] > 0)
				direct = 1;
			else
				direct = -1;
		}

		//
		waveNum--;

		if (waveNum <= 0)
			break;
	}
	//================第一个直达波的波峰获取--完成=============//

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->unloadTrace();

	return sigPosition;
}
int CRadarDllApp::MiniSigPosNum(SwathChannel * pChannel, int traceNum, int waveNum)
{
	//需要有确定的通道来源数据
	if (!pChannel)
		return -2;

	//获取通道数据的深度和宽度
	int iWidth = pChannel->getChannelHeader()->getTraceCount();
	int iHeight = pChannel->getChannelHeader()->getSample();

	//判断参数是否溢出
	if (traceNum >= iWidth)
		return -3;

	//获取所有的Trace数据
	std::map<int, Trace16*>* pTraceData = pChannel->getChannelBlob()->getTraceData16();
	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	//最大的数据和最小的数据
	int dataMax = 0;
	int dataMin = 0;

	//构造一个Trace,该Trace得到原有各个Trace的平均值，然后用于计算直达波
	Trace16 oTraceDst;
	short* pDataDst = (short *)malloc(iHeight * 2);    //此处分配的空间会在Trace16中自动释放
	oTraceDst.setTrace(pDataDst, iHeight);

	//记录参与统计的Trace数量
	int iCount = 0;
	int iSum[1024] = { 0 };
	for (int j = traceNum; j < iWidth; j++)
	{
		//记录一个Trace总强度的合计值
		int traceValueSum = 0;

		for (int i = 0; i < iHeight; i++)
		{
			Trace16* pTraceTmp = (*pTraceData)[j];
			iSum[i] = iSum[i] + pTraceTmp->getTrace()[i];

			//统计一个Trace总强度的合计
			traceValueSum = traceValueSum + abs(pTraceTmp->getTrace()[i]);
		}
		//强度达到一定程度，我们认为是有效值
		if (traceValueSum > 200)
			iCount++;

		//总的Trace数量超过256，就不用计算了
		if (iCount > 256)
			break;
	}
	for (int i = 0; i < iHeight; i++)
	{
		if (iCount > 0)
			//计算平均值
			pDataDst[i] = (short)(iSum[i] / iCount);
		else
		{
			Trace16* pTraceTmp = (*pTraceData)[traceNum];
			pDataDst[i] = pTraceTmp->getTrace()[i];
		}
	}

	//获取一个trace的数据
	short* pData = oTraceDst.getTrace();
	for (int ii = 0; ii < iHeight; ii++)
	{
		//这里循环获取最大的波峰
		int sColor = pData[ii];

		if (sColor > dataMax)
		{
			dataMax = sColor;
		}
		if (sColor < dataMin)
		{
			dataMin = sColor;
		}
	}

	//================第一个直达波的波峰获取--开始=============//
	int dataPrevPrev = pData[0];
	//记录直达波的位置
	int sigPosition = 0;
	//取波峰
	for (int ii = 1; ii < iHeight - 2; ii++)
	{
		int tmpData = pData[ii];

		//波峰向右，那么不取左边数据---切左边
		if (pData[ii] < 0)
			continue;

		int dataSig = pData[ii];
		//读取得到的数据，还没有最大数据的四分之一，所以不作为可用数据
		if (dataSig < dataMax / 4)
			continue;

		//前一个点
		int dataPrev = pData[ii - 1];
		//后一个点
		int dataNext = pData[ii + 1];

		//最近一个有变化的点
		if (dataPrev != dataSig)
			dataPrevPrev = dataPrev;

		//如果不符合条件，那么直接找下一个数据
		if ((dataPrev > dataSig) || (dataNext > dataSig))
			continue;

		//取到一个波峰
		if ((dataPrev < dataSig) && (dataNext < dataSig))
		{
			sigPosition = ii;
			waveNum--;
		}
		if ((dataPrev == dataSig) && (dataNext < dataSig))
		{
			if (dataPrevPrev < dataSig)
			{
				sigPosition = ii;
				waveNum--;
			}
		}

		if (waveNum <= 0)
			break;
	}
	//================第一个直达波的波峰获取--完成=============//

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->unloadTrace();

	return sigPosition;
}

//去直达波--裁剪直达波图形
bool CRadarDllApp::SigPositionCutByVende(char *fileData, char *fileHeader, char *path, SwathChannel * pChannel, int iZero)
{
	//需要有确定的通道来源数据
	if (!pChannel)
		return false;

	//获取通道数据的深度和宽度
	int iWidth = pChannel->getChannelHeader()->getTraceCount();
	int iHeight = pChannel->getChannelHeader()->getSample();

	//判断参数是否溢出
	if (iZero >= iHeight)
		return false;

	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	char szFileHeader[512] = { 0 }; sprintf(szFileHeader, "%s%s", path, fileHeader);
	char szFileData[512] = { 0 }; sprintf(szFileData, "%s%s", path, fileData);

	//更新采样点数
	pChannel->getChannelHeader()->setSample(pChannel->getChannelHeader()->getSample() - iZero);
	//写入Header文件
	pChannel->getChannelHeader()->saveHeader(szFileHeader);
	//裁剪数据
	pChannel->getChannelBlob()->cutTraceData(iZero);
	//写入数据文件
	pChannel->getChannelBlob()->saveTrace(szFileData);

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->unloadTrace();

	//直达波取绝对值最大的一个
	return true;
}




//==========================下面一组接口：逆振幅：不使用Matlab的库，由C++直接实现，待完成==========================//
//实现一个逆振幅增益函数，按照参数进行增大偏移
//char *fileData，数据文件名
//char *path，    存储路径
int CRadarDllApp::GainInvDecayConst(char *fileData, char *path, SwathChannel * channel, int order, int *coef)
{
	//写入的目标文件
	char szFileData[512] = { 0 };
	sprintf(szFileData, "%s%s", path, fileData);

	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->loadTrace();

	int sample = channel->getChannelHeader()->getSample();  //用于后续计算
	int offset[1024] = { 0 };								//由上到下进行排列

	//匹配增益系数
	for (int i = 0; i < sample; i++)
	{
		bool found = false;
		int j = 0;
		for (; j < order; j++)
		{
			if ((((float)j) / ((float)order)) >= (((float)i) / ((float)sample)))
			{
				offset[i] = coef[j];
				found = true;
				break;
			}
		}
		if (!found)
			offset[i] = coef[j - 1];
	}

	//具体实现逆振幅增益
	int ret = channel->getChannelBlob()->GainInvDecayConst(offset);

	//写入数据文件
	channel->getChannelBlob()->saveTrace(szFileData);

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->unloadTrace();

	return 0;
}

//实现一个逆振幅增益函数，按照参数进行增大偏移
//char *fileData，数据文件名
//char *path，    存储路径
int CRadarDllApp::GainInvDecayCoef(char *fileData, char *path, SwathChannel * channel, int order, int *coef)
{
	//写入的目标文件
	char szFileData[512] = { 0 };
	sprintf(szFileData, "%s%s", path, fileData);

	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->loadTrace();

	int sample = channel->getChannelHeader()->getSample();  //用于后续计算
	int offset[1024] = { 0 };								//由上到下进行排列

	//匹配增益系数
	for (int i = 0; i < sample; i++)
	{
		bool found = false;
		int j = 0;
		for (; j < order; j++)
		{
			if ((((float)j) / ((float)order)) >= (((float)i) / ((float)sample)))
			{
				offset[i] = coef[j];
				found = true;
				break;
			}
		}
		if (!found)
			offset[i] = coef[j - 1];
	}

	//具体实现逆振幅增益
	int ret = channel->getChannelBlob()->GainInvDecayCoef(offset);

	//写入数据文件
	channel->getChannelBlob()->saveTrace(szFileData);

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->unloadTrace();

	return 0;
}

//实现一个逆振幅增益函数，如果原有的信号为线性衰减，则采用该方法进行逆振幅。 信号衰减模型: y = b - k * ( x ^ n )
//char *fileData，数据文件名
//char *path，    存储路径
//	k : 取值范围为1 ... ... 100
//  n:  取值范围为 1 ... ... 10
int CRadarDllApp::GainInvDecayCurve(char *fileData, char *path, SwathChannel * channel, int k, int n)
{
	//写入的目标文件
	char szFileData[512] = { 0 };
	sprintf(szFileData, "%s%s", path, fileData);

	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->loadTrace();

	//具体实现逆振幅增益
	int ret = channel->getChannelBlob()->GainInvDecayCurve(k, n);

	//写入数据文件
	channel->getChannelBlob()->saveTrace(szFileData);

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->unloadTrace();

	return 0;
}
//==========================上面一组接口：逆振幅：不使用Matlab的库，由C++直接实现，待完成==========================//

//==========================下面一组接口：去背景噪声：不使用Matlab的库，由C++直接实现，待完成========================//
//实现一个逆振幅增益函数，如果原有的信号为线性衰减，则采用该方法进行逆振幅。 信号衰减模型: y = b - k * ( x ^ n )
//char *fileData，数据文件名
//char *path，    存储路径
//channel :带有头信息和数据信息的通道对象
int CRadarDllApp::SimpleRemoveBackgr(char *fileData, char *path, SwathChannel * channel )
{
	//写入的目标文件
	char szFileData[512] = { 0 };
	sprintf(szFileData, "%s%s", path, fileData);

	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->loadTrace();

	//具体实现去背景噪声
	int ret = channel->getChannelBlob()->RemoveBackgr();

	//写入数据文件
	channel->getChannelBlob()->saveTrace(szFileData);

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->unloadTrace();

	return 0;
}
//==========================上面一组接口：去背景噪声：不使用Matlab的库，由C++直接实现，待完成========================//

//==========================下面一组接口：去直流噪声：不使用Matlab的库，由C++直接实现，待完成========================//
int CRadarDllApp::SimpleRemoveDC(char *fileData, char *path, SwathChannel * channel)
{
	//写入的目标文件
	char szFileData[512] = { 0 };
	sprintf(szFileData, "%s%s", path, fileData);

	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->loadTrace();

	//具体实现去直流噪声
	int ret = channel->getChannelBlob()->RemoveDC();

	//写入数据文件
	channel->getChannelBlob()->saveTrace(szFileData);

	//unloadTrace图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	channel->getChannelBlob()->unloadTrace();

	return 0;
}
//==========================上面一组接口：去直流噪声：不使用Matlab的库，由C++直接实现，待完成========================//

/*
 * Fun:计算Trace中数据的平均值，用于数据合法性判断
 * Parameter：
 *    pChannel，   channel对象
 *    traceNum，   指定直达波的Trace号
 *    traceValue,  返回值，Trace中数据的平均值
 */
int CRadarDllApp::TraceAvg(SwathChannel * pChannel, int traceNum, int &traceValue)
{
	//需要有确定的通道来源数据
	if (!pChannel)
		return -2;

	//获取通道数据的深度和宽度
	int iWidth = pChannel->getChannelHeader()->getTraceCount();
	int iHeight = pChannel->getChannelHeader()->getSample();

	//判断参数是否溢出
	if (traceNum >= iWidth)
		return -3;

	//获取所有的Trace数据
	std::map<int, Trace16*>* pTraceData = pChannel->getChannelBlob()->getTraceData16();
	//load图像数据(爲了提升性能，load和unload单独在init和uninit中调用)
	pChannel->getChannelBlob()->loadTrace();

	//数据和
	long long dataSum = 0;

	//获取一个trace的数据
	Trace16* pTraceTmp = (*pTraceData)[traceNum];
	short* pData = pTraceTmp->getTrace();
	for (int ii = 0; ii < iHeight; ii++)
	{
		dataSum = dataSum + abs(pData[ii]);
	}

	traceValue = (int)(dataSum / iHeight);

	return 0;
}

//将UTC时间转化为北京时区,同时增加一个时间偏移
void CRadarDllApp::TransferTime(char * pTime, int iOffsetTime)
{
	//把UTC时间转化为北京时间
	int   iYear, iMonth, iDay, iHour, iMinute, iSecond, iMilliSecond;
	sscanf(pTime, "%d-%d-%d	%d:%d:%d:%d", &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond, &iMilliSecond);
	CTime oTime(iYear, iMonth, iDay, iHour, iMinute, iSecond);
	//加8小时，转为北京时间
	oTime = oTime + CTimeSpan(0, 8, 0, 0);

	iYear = oTime.GetYear();
	iMonth = oTime.GetMonth();
	iDay = oTime.GetDay();
	iHour = oTime.GetHour();
	iMinute = oTime.GetMinute();
	iSecond = oTime.GetSecond();

	//将毫秒多餘的时间加到秒上
	iSecond = iSecond + (iMilliSecond + iOffsetTime) / 1000;
	iMilliSecond = (iMilliSecond + iOffsetTime) % 1000;

	//将秒多餘的时间加到分鐘上
	iMinute = iMinute + iSecond / 60;
	iSecond = iSecond % 60;

	//将分鐘上多餘的时间加到小时上
	iHour = iHour + iMinute / 60;
	iMinute = iMinute % 60;

	if (iHour >= 24)
	{
		iHour = 0;
		iDay = iDay + 1;
	}

	sprintf(pTime, "%04d-%02d-%02d %02d:%02d:%02d:%03d", iYear, iMonth, iDay, iHour, iMinute, iSecond, iMilliSecond);
}

//删除无效Trace，返回删除数量
int CRadarDllApp::DeleteInvalidTrace(SwathChannelBlob * channelBlob, int dataType)
{
	if (!channelBlob)
		return 0;
	if ((16 != dataType) && (32 != dataType))
		return 0;

	//正式删除
	int deleteCount = 0;
	if (16 == dataType)
	{
		deleteCount = channelBlob->deleteTrace16(255);
	}
	if (32 == dataType)
	{
		deleteCount = channelBlob->deleteTrace32(255);
	}

	return deleteCount;
}


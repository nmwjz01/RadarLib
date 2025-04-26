// DialogCard.cpp: 实现文件
//
#include <io.h>
#include "stdafx.h"

#include "resource.h"


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

#include "Project.h"
#include "DialogCard.h"


// DialogCard 对话框

IMPLEMENT_DYNAMIC(DialogCard, CDialogEx)

DialogCard::DialogCard(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CARD, pParent)
	, m_CardNo(_T(""))
	, m_B(_T(""))
	, m_L(_T(""))
	, m_N(_T(""))
	, m_E(_T(""))
	, m_Size(_T(""))
	, m_Deep(_T(""))
	, m_Position(_T(""))
	, m_Time(_T(""))
	, m_Height(_T(""))
	, m_PictureFront(_T(""))
	, m_PictureBack(_T(""))
	, m_PictureLeft(_T(""))
	, m_PictureRight(_T(""))
	, m_LadarPicture1(_T(""))
	, m_LadarPicture2(_T(""))
	, m_LadarPicture3(_T(""))
	, m_LadarPicture4(_T(""))
{
}

DialogCard::~DialogCard()
{
}

void DialogCard::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CARD_NO, m_CardNo);
	DDX_Text(pDX, IDC_CARD_B, m_B);
	DDX_Text(pDX, IDC_CARD_L, m_L);
	DDX_Text(pDX, IDC_CARD_N, m_N);
	DDX_Text(pDX, IDC_CARD_E, m_E);
	DDX_Text(pDX, IDC_CARD_SIZE, m_Size);
	DDX_Text(pDX, IDC_CARD_DEEP, m_Deep);
	DDX_Text(pDX, IDC_CARD_POSITION, m_Position);
	DDX_Text(pDX, IDC_CARD_TIME, m_Time);
	DDX_Text(pDX, IDC_CARD_HEIGHT, m_Height);
	DDX_Control(pDX, IDC_CARD_LEVEL, m_oLevel);
	DDX_Control(pDX, IDC_CARD_TYPE, m_oType);
	DDX_Text(pDX, IDC_PICTURE_NAME_FRONT, m_PictureFront);
	DDX_Text(pDX, IDC_PICTURE_NAME_BACK, m_PictureBack);
	DDX_Text(pDX, IDC_PICTURE_NAME_LEFT, m_PictureLeft);
	DDX_Text(pDX, IDC_PICTURE_NAME_RIGHT, m_PictureRight);
	DDX_Text(pDX, IDC_LADAR_PICTURE1, m_LadarPicture1);
	DDX_Text(pDX, IDC_LADAR_PICTURE2, m_LadarPicture2);
	DDX_Text(pDX, IDC_LADAR_PICTURE3, m_LadarPicture3);
	DDX_Text(pDX, IDC_LADAR_PICTURE4, m_LadarPicture4);
}


BEGIN_MESSAGE_MAP(DialogCard, CDialogEx)
	ON_BN_CLICKED(ID_CARD_CREATE, &DialogCard::OnBnClickedCardCreate)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// DialogCard 消息处理程序
void DialogCard::SetParam(Project* pProject, Swath* pSwath, SwathChannel* pChannel, CRect oRect)
{
	//参数合法性判断
	if ((NULL == pProject) || (NULL == pSwath) || (NULL == pChannel))
		return;

	m_pProject = pProject;
	m_pSwath   = pSwath;
	m_pChannel = pChannel;
	m_iCurrent = ( oRect.left + oRect.right ) / 2;

	//工程，测线，通道信息
	USES_CONVERSION;
	CString strProjectDir = pProject->getProjectPath();    //编码转换
	CString strSwathName  = pSwath->getSwathID();
	m_Position.Format ( _T( "工程目录:%s; 测线名:%s; 通道号:%d" ) , strProjectDir, strSwathName, pChannel->getNo() );

	//雷达扫描时间
	SwathTimeData *oTimeData = pSwath->getSwathTime()->getData(m_iCurrent);
	char szPictureTime[64] = { 0 };
	sprintf(szPictureTime, "%s %s", oTimeData->getDateString(), oTimeData->getTimeString());
	//将时间转化为北京时间
	TransferTime(szPictureTime);
	m_Time = A2W(szPictureTime);

	//深度
	int iDeep = (oRect.top + oRect.bottom) / 2;
	int iDeepCM = ( int )( 1 / m_pChannel->getChannelHeader()->getFrequency() * m_pChannel->getChannelHeader()->getSoilVel() / 2 * iDeep * 100 ) - pChannel->getChannelHeader()->getZeroLevel();
	m_Deep.Format(_T("%d"), iDeepCM);

	//净高
	int iHeigh = oRect.bottom - oRect.top;
	int iHeighCM = ( int )(1 / m_pChannel->getChannelHeader()->getFrequency() * m_pChannel->getChannelHeader()->getSoilVel() / 2 * iHeigh * 100) - pChannel->getChannelHeader()->getZeroLevel();
	m_Height.Format( _T("%d") , iHeighCM);

	//净宽
	int iWidth = oRect.right - oRect.left;
	int iWidthCM = (int)(1 / m_pChannel->getChannelHeader()->getFrequency() * m_pChannel->getChannelHeader()->getSoilVel() / 2 * iWidth * 100) - pChannel->getChannelHeader()->getZeroLevel();

	float fSize = ((float)iHeighCM / 100) * ((float)iWidthCM / 100);
	m_Size.Format( _T("%3.3f(%2.2fx%2.2f)") , fSize, ((float)iHeighCM / 100) , ((float)iWidthCM / 100));

	//坐标信息
	SwathCorData *pCorData = pSwath->getSwathCor()->getData(m_iCurrent);
	if (NULL == pCorData)
	{
		m_N.Format(_T("位置丢失") );
		m_E.Format(_T("位置丢失") );
	}
	else
	{
		m_N.Format(_T("%4.11f"), pCorData->getLat());
		m_E.Format(_T("%4.11f"), pCorData->getLon());
	}

	//取得抓拍照片文件名-前
	char szPicturePathFileFront[512] = { 0 };
	m_pProject->getCameraFront()->getPictureByTime(szPictureTime, szPicturePathFileFront);
	if (0 == strlen(szPicturePathFileFront))
		strcpy(szPicturePathFileFront, "缺少图像");
	m_PictureFront = A2W(szPicturePathFileFront);
	m_PictureFront = m_PictureFront + _T("(前)");

	//取得抓拍照片文件名-后
	char szPicturePathFileBack[512] = { 0 };
	m_pProject->getCameraBack()->getPictureByTime(szPictureTime, szPicturePathFileBack);
	if (0 == strlen(szPicturePathFileBack))
		strcpy(szPicturePathFileBack, "缺少图像");
	m_PictureBack = A2W(szPicturePathFileBack);
	m_PictureBack = m_PictureBack + _T("(后)");

	//取得抓拍照片文件名-左
	char szPicturePathFileLeft[512] = { 0 };
	m_pProject->getCameraLeft()->getPictureByTime(szPictureTime, szPicturePathFileLeft);
	if (0 == strlen(szPicturePathFileLeft))
		strcpy(szPicturePathFileLeft, "缺少图像");
	m_PictureLeft = A2W(szPicturePathFileLeft);
	m_PictureLeft = m_PictureLeft + _T("(左)");

	//取得抓拍照片文件名-右
	char szPicturePathFileRight[512] = { 0 };
	m_pProject->getCameraRight()->getPictureByTime(szPictureTime, szPicturePathFileRight);
	if (0 == strlen(szPicturePathFileRight))
		strcpy(szPicturePathFileRight, "缺少图像");
	m_PictureRight = A2W(szPicturePathFileRight);
	m_PictureRight = m_PictureRight + _T("(右)");
}

//在指定的通道上创建开始trace到结束trace的侧视图
void DialogCard::MakeCardSideView(int iTraceStart, int iTraceEnd, SwathChannel* pChannel)
{}
//指定开始Trace和结束Trace，指定深度，创建俯视图
void DialogCard::MakeCardTopView(int iTraceStart, int iTraceEnd, Swath* pSwath, int iDeep)
{}
//在指定Trace的位置，创建截视图
void DialogCard::MakeCardCutView(int iTrace, Swath* pSwath)
{}

//这是一个Demo，用于创建word文档(可用)
void DialogCard::MakeCardDoc( CString strCardFile )
{
	// ******************* Declare Some Variables ********************
	// Variables that will be used and re-used in our calls
	DISPPARAMS dpNoArgs = { NULL, NULL, 0, 0 };
	VARIANT vResult;
	OLECHAR FAR* szFunction;
	BSTR bstrTemp;
	// IDispatch pointers for Word's objects
	IDispatch* pDispDocs;      //Documents collection
	IDispatch* pDispSel;       //Selection object
	IDispatch* pDispActiveDoc; //ActiveDocument object
	// DISPID's
	DISPID dispid_Docs;        //Documents property of Application object
	DISPID dispid_DocsAdd;     //Add method of Documents collection
							   //object
	DISPID dispid_Sel;         //Selection property of Applicaiton object
	DISPID dispid_TypeText;    //TypeText method of Selection object
	DISPID dispid_TypePara;    //TypeParagraph method of Selection object
	DISPID dispid_ActiveDoc;   //ActiveDocument property of Application
							   //obj
	DISPID dispid_SaveAs;      //SaveAs method of the Document object
	DISPID dispid_Quit;        //Quit method of the Application object
	// ******************** Start Automation ***********************
	//Initialize the COM libraries
	::CoInitialize(NULL);
	// Create an instance of the Word application and obtain the pointer
	// to the application's IDispatch interface.
	CLSID clsid;
	CLSIDFromProgID(L"Word.Application", &clsid);
	IUnknown* pUnk;
	HRESULT hr = ::CoCreateInstance(clsid, NULL, CLSCTX_SERVER, IID_IUnknown, (void**)&pUnk);
	IDispatch* pDispApp;
	hr = pUnk->QueryInterface(IID_IDispatch, (void**)&pDispApp);
	// Get IDispatch* for the Documents collection object
	szFunction = OLESTR("Documents");
	hr = pDispApp->GetIDsOfNames(IID_NULL, &szFunction, 1, LOCALE_USER_DEFAULT, &dispid_Docs);
	hr = pDispApp->Invoke(dispid_Docs, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dpNoArgs, &vResult, NULL, NULL);
	pDispDocs = vResult.pdispVal;
	// Invoke the Add method on the Documents collection object
	// to create a new document in Word
	// Note that the Add method can take up to 3 arguments, all of which
	// are optional. We are not passing it any so we are using an empty
	// DISPPARAMS structure
	szFunction = OLESTR("Add");
	hr = pDispDocs->GetIDsOfNames(IID_NULL, &szFunction, 1, LOCALE_USER_DEFAULT, &dispid_DocsAdd);
	hr = pDispDocs->Invoke(dispid_DocsAdd, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpNoArgs, &vResult, NULL, NULL);

	// Get IDispatch* for the Selection object
	szFunction = OLESTR("Selection");
	hr = pDispApp->GetIDsOfNames(IID_NULL, &szFunction, 1, LOCALE_USER_DEFAULT, &dispid_Sel);
	hr = pDispApp->Invoke(dispid_Sel, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dpNoArgs, &vResult, NULL, NULL);
	pDispSel = vResult.pdispVal;

	// Get the DISPIDs of the TypeText and TypeParagraph methods of the
	// Selection object.  We'll use these DISPIDs multiple times.
	szFunction = OLESTR("TypeText");
	hr = pDispSel->GetIDsOfNames(IID_NULL, &szFunction, 1, LOCALE_USER_DEFAULT, &dispid_TypeText);
	szFunction = OLESTR("TypeParagraph");
	hr = pDispSel->GetIDsOfNames(IID_NULL, &szFunction, 1, LOCALE_USER_DEFAULT, &dispid_TypePara);
	// The TypeText method has and requires only one argument, a string,
	// so set up the DISPPARAMS accordingly
	VARIANT vArgsTypeText[1];
	DISPPARAMS dpTypeText;
	bstrTemp = ::SysAllocString(OLESTR("One"));
	vArgsTypeText[0].vt = VT_BSTR;
	vArgsTypeText[0].bstrVal = bstrTemp;
	dpTypeText.cArgs = 1;
	dpTypeText.cNamedArgs = 0;
	dpTypeText.rgvarg = vArgsTypeText;
	//Invoke the first TypeText and TypeParagraph pair
	hr = pDispSel->Invoke(dispid_TypeText, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpTypeText, NULL, NULL, NULL);
	hr = pDispSel->Invoke(dispid_TypePara, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpNoArgs, NULL, NULL, NULL);
	::SysFreeString(bstrTemp);
	//Invoke the second TypeText and TypeParagraph pair
	bstrTemp = ::SysAllocString(OLESTR("Two"));
	hr = pDispSel->Invoke(dispid_TypeText, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpTypeText, NULL, NULL, NULL);
	hr = pDispSel->Invoke(dispid_TypePara, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpNoArgs, NULL, NULL, NULL);
	::SysFreeString(bstrTemp);
	//Invoke the third TypeText and TypeParagraph pair
	bstrTemp = ::SysAllocString(OLESTR("Three"));
	hr = pDispSel->Invoke(dispid_TypeText, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpTypeText, NULL, NULL, NULL);
	hr = pDispSel->Invoke(dispid_TypePara, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpNoArgs, NULL, NULL, NULL);
	::SysFreeString(bstrTemp);
	// Get IDispatch* for the ActiveDocument object
	szFunction = OLESTR("ActiveDocument");
	hr = pDispApp->GetIDsOfNames(IID_NULL, &szFunction, 1, LOCALE_USER_DEFAULT, &dispid_ActiveDoc);
	hr = pDispApp->Invoke(dispid_ActiveDoc, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dpNoArgs, &vResult, NULL, NULL);
	pDispActiveDoc = vResult.pdispVal;
	//Set up the DISPPARAMS for the SaveAs method (11 arguments)
	VARIANT vArgsSaveAs[11];
	DISPPARAMS dpSaveAs;
	dpSaveAs.cArgs = 11;
	dpSaveAs.cNamedArgs = 0;
	dpSaveAs.rgvarg = vArgsSaveAs;
	BSTR bstrEmptyString;
	bstrEmptyString = ::SysAllocString(OLESTR(""));
	VARIANT vFalse;
	vFalse.vt = VT_BOOL;
	vFalse.boolVal = FALSE;
	//TRY THIS:
	//To see the error handler in action, change the following
	//line to:
	//
	//     bstrTemp = ::SysAllocString(OLESTR("c:\\badpath\\doc1.doc"));
	char szPathFile[512] = { 0 };
	strcpy(szPathFile, strCardFile.GetBuffer(strCardFile.GetLength()));
	bstrTemp = _com_util::ConvertStringToBSTR(szPathFile);

	//TRY THIS:
	//To see the error handler in action, change the following
	//line to:
	//
	//   vArgsSaveAs[10].vt = VT_I4;       
	vArgsSaveAs[10].vt = VT_BSTR;
	vArgsSaveAs[10].bstrVal = bstrTemp;        //Filename
	vArgsSaveAs[9].vt = VT_I4;
	vArgsSaveAs[9].lVal = 0;                   //FileFormat
	vArgsSaveAs[8] = vFalse;                   //LockComments
	vArgsSaveAs[7].vt = VT_BSTR;
	vArgsSaveAs[7].bstrVal = bstrEmptyString;  //Password
	vArgsSaveAs[6].vt = VT_BOOL;
	vArgsSaveAs[6].boolVal = TRUE;             //AddToRecentFiles
	vArgsSaveAs[5].vt = VT_BSTR;
	vArgsSaveAs[5].bstrVal = bstrEmptyString;  //WritePassword
	vArgsSaveAs[4] = vFalse;                   //ReadOnlyRecommended
	vArgsSaveAs[3] = vFalse;                   //EmbedTrueTypeFonts
	vArgsSaveAs[2] = vFalse;                   //SaveNativePictureFormat
	vArgsSaveAs[1] = vFalse;                   //SaveFormsData
	vArgsSaveAs[0] = vFalse;                   //SaveAsOCELetter

	//Invoke the SaveAs method
	szFunction = OLESTR("SaveAs");
	hr = pDispActiveDoc->GetIDsOfNames(IID_NULL, &szFunction, 1, LOCALE_USER_DEFAULT, &dispid_SaveAs);
	EXCEPINFO excep;
	hr = pDispActiveDoc->Invoke(dispid_SaveAs, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpSaveAs, NULL, &excep, NULL);
	if (FAILED(hr))
	{
		//ErrHandler(hr, excep);
		return;
	}
	::SysFreeString(bstrEmptyString);
	//Invoke the Quit method
	szFunction = OLESTR("Quit");
	hr = pDispApp->GetIDsOfNames(IID_NULL, &szFunction, 1, LOCALE_USER_DEFAULT, &dispid_Quit);
	hr = pDispApp->Invoke(dispid_Quit, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpNoArgs, NULL, NULL, NULL);
	//Clean-up
	::SysFreeString(bstrTemp);
	pDispActiveDoc->Release();
	pDispSel->Release();
	pDispDocs->Release();
	pDispApp->Release();
	pUnk->Release();
	::CoUninitialize();
}

//计算开始trace和结束trace
void DialogCard::GetStartEndTrace(Project* pProject, Swath* pSwath, SwathChannel* pChannel, int iCurrent, int Length, int& iStart, int& iEnd)
{
	double cx = pChannel->getChannelHeader()->getIntervalDist();
	double cy = 1 / pChannel->getChannelHeader()->getFrequency() * pChannel->getChannelHeader()->getSoilVel() / 2;
	int iMaxTrace = pChannel->getChannelBlob()->getTraceCount();

	FSize units = FSize(cx, cy);

	iStart = (int)(iCurrent * units.getCX() - Length); iStart = (int)(iStart > 0) ? (int)(iStart / units.getCX()) : 0;
	iEnd = (int)(iCurrent * units.getCX() + Length); iEnd = (int)(iEnd < iMaxTrace) ? (int)(iEnd / units.getCX()) : iMaxTrace;
}

//处理界面上的按钮点击事件
void DialogCard::OnBnClickedCardCreate()
{
	//浏览系统，设置目标存储的文件
	USES_CONVERSION;
	CString strFileName;  strFileName = m_CardNo + ".doc";
	CString strFilePath;
	TCHAR szFilter[] = _T("Word文件(*.doc)|*.doc|所有文件(*.*)|*.*||");
	CFileDialog fileDlg(FALSE, _T("doc"), strFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	if (IDOK != fileDlg.DoModal())
		return;

	//目标
	CString strCardFile = fileDlg.GetPathName();

	//1、取得目标时间点

	//2、取得目标时间点的现场图像

	//3、取得病害卡的文件名（包含路径）

	//4、通过当前Trace号和距离，计算开始trace和结束trace
	int iStart = 0;
	int iEnd = 0;
	GetStartEndTrace(m_pProject, m_pSwath, m_pChannel, m_iCurrent, 5, iStart, iEnd);

	//5、生成病害卡需要的雷达图像的侧视图
	//5.1、问题点前后5米范围内的侧视图（包含不同的通道）
	//5.2、问题点前后20米范围内的侧视图（包含不同的通道）

	//6、生成病害卡需要的雷达图像的俯视图
	//6.1、问题点前后5米范围内的俯视图（包含不同的深度）
	//6.2、问题点前后20米范围内的俯视图（包含不同的深度）

	//7、生成病害卡需要的雷达图像的截视图
	//7.1、问题点前后5米范围内的截视图
	//7.2、问题点前后20米范围内的截视图

	//8、将图像数据和输入的数据组装生成Word文档
	MakeCardDoc(strCardFile);

	MessageBox( (CString) "建立完成" , (CString)"提示" , MB_OK );
}

//在绘制界面时可，将抓拍图片显示出来
void DialogCard::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()

	//显示前置抓拍机的照片
	DisplayPhotoFront();

	//显示后置抓拍机的照片
	DisplayPhotoBack();
}
void DialogCard::DisplayPhotoFront()
{
	//前后照片的位置
	USES_CONVERSION;
	CString strProjectDir = m_pProject->getProjectPath();

	char szPicturePathFileFront[512] = { 0 };
	char szPictureTime[64] = { 0 };

	//雷达扫描时间
	SwathTimeData *oTimeData = m_pSwath->getSwathTime()->getData(m_iCurrent);
	sprintf(szPictureTime, "%s %s", oTimeData->getDateString(), oTimeData->getTimeString());
	//将时间转化为北京时间
	TransferTime(szPictureTime);

	m_pProject->getCameraFront()->getPictureByTime(szPictureTime, szPicturePathFileFront);
	if (0 == strlen(szPicturePathFileFront))
		return;

	//显示前置抓拍机的照片
	char szPictureFile[128] = { 0 };
	sprintf(szPictureFile, "\\%s\\%s", "CameraFront", szPicturePathFileFront);
	CString strFile = strProjectDir + CA2CT(szPictureFile);
	CRect	oRect;
	CImage	oImageFront;
	oImageFront.Load(strFile);

	CWnd *pWnd = GetDlgItem(IDC_CARD_PHOTO_FRONT);
	pWnd->GetClientRect(&oRect);
	CDC *pDc = pWnd->GetDC();
	pDc->SetStretchBltMode(COLORONCOLOR);
	oImageFront.Draw(pDc->m_hDC, oRect);//将图片绘制到picture表示的区域内
	ReleaseDC(pDc);
}
void DialogCard::DisplayPhotoBack()
{
	//前后照片的位置
	USES_CONVERSION;
	CString strProjectDir = m_pProject->getProjectPath();

	char szPicturePathFileBack[512] = { 0 };
	char szPictureTime[64] = { 0 };

	//雷达扫描时间
	SwathTimeData *oTimeData = m_pSwath->getSwathTime()->getData(m_iCurrent);
	sprintf(szPictureTime, "%s %s", oTimeData->getDateString(), oTimeData->getTimeString());
	//将时间转化为北京时间
	TransferTime(szPictureTime);

	m_pProject->getCameraBack()->getPictureByTime(szPictureTime, szPicturePathFileBack);
	if (0 == strlen(szPicturePathFileBack))
		return;

	//显示后置抓拍机的照片
	char szPictureFile[128] = { 0 };
	sprintf(szPictureFile, "\\%s\\%s", "CameraFront", szPicturePathFileBack);
	CString strFile = strProjectDir + CA2CT(szPictureFile);
	CRect	oRect;
	CImage	oImageBack;
	oImageBack.Load(strFile);

	CWnd *pWnd = GetDlgItem(IDC_CARD_PHOTO_BACK);
	pWnd->GetClientRect(&oRect);
	CDC *pDc = pWnd->GetDC();
	pDc->SetStretchBltMode(COLORONCOLOR);
	oImageBack.Draw(pDc->m_hDC, oRect);//将图片绘制到picture表示的区域内
	ReleaseDC(pDc);
}

void DialogCard::DisplayPhotoLeft()
{
	//前后照片的位置
	USES_CONVERSION;
	CString strProjectDir = m_pProject->getProjectPath();

	char szPicturePathFileLeft[512] = { 0 };
	char szPictureTime[64] = { 0 };

	//雷达扫描时间
	SwathTimeData *oTimeData = m_pSwath->getSwathTime()->getData(m_iCurrent);
	sprintf(szPictureTime, "%s %s", oTimeData->getDateString(), oTimeData->getTimeString());
	//将时间转化为北京时间
	TransferTime(szPictureTime);

	m_pProject->getCameraLeft()->getPictureByTime(szPictureTime, szPicturePathFileLeft);
	if (0 == strlen(szPicturePathFileLeft))
		return;

	//显示后置抓拍机的照片
	char szPictureFile[128] = { 0 };
	sprintf(szPictureFile, "\\%s\\%s", "CameraFront", szPicturePathFileLeft);
	CString strFile = strProjectDir + CA2CT(szPictureFile);
	CRect	oRect;
	CImage	oImageLeft;
	oImageLeft.Load(strFile);

	CWnd *pWnd = GetDlgItem(IDC_CARD_PHOTO_LEFT);
	pWnd->GetClientRect(&oRect);
	CDC *pDc = pWnd->GetDC();
	pDc->SetStretchBltMode(COLORONCOLOR);
	oImageLeft.Draw(pDc->m_hDC, oRect);//将图片绘制到picture表示的区域内
	ReleaseDC(pDc);
}

void DialogCard::DisplayPhotoRight()
{
	//前后照片的位置
	USES_CONVERSION;
	CString strProjectDir = m_pProject->getProjectPath();

	char szPicturePathFileRight[512] = { 0 };
	char szPictureTime[64] = { 0 };

	//雷达扫描时间
	SwathTimeData *oTimeData = m_pSwath->getSwathTime()->getData(m_iCurrent);
	sprintf(szPictureTime, "%s %s", oTimeData->getDateString(), oTimeData->getTimeString());
	//将时间转化为北京时间
	TransferTime(szPictureTime);

	m_pProject->getCameraRight()->getPictureByTime(szPictureTime, szPicturePathFileRight);
	if (0 == strlen(szPicturePathFileRight))
		return;

	//显示后置抓拍机的照片
	char szPictureFile[128] = { 0 };
	sprintf(szPictureFile, "\\%s\\%s", "CameraFront", szPicturePathFileRight);
	CString strFile = strProjectDir + CA2CT(szPictureFile);
	CRect	oRect;
	CImage	oImageRight;
	oImageRight.Load(strFile);

	CWnd *pWnd = GetDlgItem(IDC_CARD_PHOTO_RIGHT);
	pWnd->GetClientRect(&oRect);
	CDC *pDc = pWnd->GetDC();
	pDc->SetStretchBltMode(COLORONCOLOR);
	oImageRight.Draw(pDc->m_hDC, oRect);//将图片绘制到picture表示的区域内
	ReleaseDC(pDc);
}


//将UTC时间转化为北京时区
void DialogCard::TransferTime(char * pTime)
{
	//把UTC时间转化为北京时间
	int   iYear, iMonth, iDay, iHour, iMinute, iSecond, iMilliSecond;
	sscanf(pTime, "%d-%d-%d	%d:%d:%d:%d", &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond, &iMilliSecond);
	CTime oTime(iYear, iMonth, iDay, iHour, iMinute, iSecond);
	//加8小时，转为北京时间
	oTime = oTime + CTimeSpan(0, 8, 0, 0);

	sprintf(pTime, "%04d-%02d-%02d %02d:%02d:%02d:%03d", oTime.GetYear(), oTime.GetMonth(), oTime.GetDay(), oTime.GetHour(), oTime.GetMinute(), oTime.GetSecond(), iMilliSecond);
}

//初始化界面
BOOL DialogCard::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	m_oLevel.AddString(_T("一般"));
	m_oLevel.AddString(_T("中等"));
	m_oLevel.AddString(_T("严重"));
	m_oLevel.AddString(_T("很严重"));
	m_oLevel.SetCurSel(0);

	m_oType.AddString(_T("空洞"));
	m_oType.AddString(_T("涵管"));
	m_oType.AddString(_T("竖井"));
	m_oType.AddString(_T("其他"));
	m_oType.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



// RadarDoc.cpp: RadarDoc 类的实现
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

#include "RadarDoc.h"
#include "SystemInfo.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// RadarDoc

IMPLEMENT_DYNCREATE(RadarDoc, CDocument)

BEGIN_MESSAGE_MAP(RadarDoc, CDocument)
END_MESSAGE_MAP()


//在这里，完成鉴权操作：
// 1、如果鉴权失败，则退出程序。
// 2、如果鉴权成功，并且还在一个月的评估期内，则提示用户缴费，继续运行
// 3、如果鉴权成功，并且超出一个月的评估期，则提示用户缴费，退出程序。
// 4、如果鉴权成功，并且已经缴费，则继续运行
// RadarDoc 构造/析构
RadarDoc::RadarDoc() noexcept
{
	return;
}

RadarDoc::~RadarDoc()
{
}

BOOL RadarDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}


// RadarDoc 序列化
void RadarDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void RadarDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void RadarDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void RadarDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// RadarDoc 诊断
#ifdef _DEBUG
void RadarDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void RadarDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// RadarDoc 命令

//

BOOL RadarDoc::GetDiskID(char *pID)
{
	return TRUE;
}


BOOL RadarDoc::CalculateSN(char *pAllID)
{
	return TRUE;
}

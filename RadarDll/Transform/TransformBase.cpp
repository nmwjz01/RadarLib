
#include <io.h>
#include <atlimage.h>

#include "..\\Utils\\PalSET.h"
#include "..\\Utils\\Utils.h"
#include "..\\Utils\\RadarConst.h"

#include "..\\Impluse\\ImpluseTrace16.h"
#include "..\\Impluse\\ImpluseTrace32.h"
#include "..\\Impluse\\ImpluseCor.h"
#include "..\\Impluse\\ImpluseTime.h"
#include "..\\Impluse\\ImpluseChannelHeader.h"
#include "..\\Impluse\\ImpluseChannelBlob.h"
#include "..\\Impluse\\ImpluseChannel.h"
#include "..\\Impluse\\ImpluseSwath.h"

//#include "FSize.h"

#include "..\\IDS\\IDSChannel.h"
#include "..\\IDS\\IDSChannelBlob.h"
#include "..\\IDS\\IDSChannelHeader.h"
#include "..\\IDS\\IDSSwath.h"
#include "..\\IDS\\IDSSwathFragment.h"
#include "..\\IDS\\IDSTrace16.h"
#include "..\\IDS\\IDSTrace32.h"

#include "..\\Mala\\MalaChannel.h"
#include "..\\Mala\\MalaChannelBlob.h"
#include "..\\Mala\\MalaChannelHeader.h"
#include "..\\Mala\\MalaSwath.h"
#include "..\\Mala\\MalaTime.h"
#include "..\\Mala\\MalaTrace16.h"
#include "..\\Mala\\MalaTrace32.h"

#include "TransformBase.h"

int TransformBase::CreateCor(const char *pathDst, const char *swathName, int swathIndex)
{
	try
	{
		char szFile[512] = { 0 };
		sprintf(szFile, "%s\\%s_%03d.cor", pathDst, swathName, swathIndex);

		//产生一个cor文件
		SwathCor oSwathCor;
		oSwathCor.saveCor(szFile);

		return ERROR_CODE_SUCCESS;
	}
	catch (...)
	{
		return ERROR_CODE_OTHER;
	}
}

int TransformBase::CreateGps(const char *pathDst, const char *swathName, int swathIndex)
{
	try
	{
		char szBuff[128] = { "" };
		char szFile[512] = { 0 };
		sprintf(szFile, "%s\\%s_%03d.gps", pathDst, swathName, swathIndex);

		FILE * srcFile = fopen(szFile, "w");
		//fwrite(szBuff, 0, strlen(szBuff), srcFile);
		fclose(srcFile);

		return ERROR_CODE_SUCCESS;
	}
	catch (...)
	{
		return ERROR_CODE_OTHER;
	}
}

int TransformBase::CreateMrk(const char *pathDst, const char *swathName, int swathIndex)
{
	try
	{
		char szBuff[128] = { "" };  strcpy(szBuff, "HEADER VERSION:100");
		char szFile[512] = { 0 };
		sprintf(szFile, "%s\\%s_%03d.mrk", pathDst, swathName, swathIndex);

		FILE * srcFile = fopen(szFile, "w");
		fwrite(szBuff, 0, strlen(szBuff), srcFile);
		fclose(srcFile);

		return ERROR_CODE_SUCCESS;
	}
	catch (...)
	{
		return ERROR_CODE_OTHER;
	}
}

int TransformBase::CreateOrd(const char *pathDst, const char *swathName, int swathIndex)
{
	try
	{
		char szBuff[128] = { "" };
		char szFile[512] = { 0 };
		sprintf(szFile, "%s\\%s_%03d.ord", pathDst, swathName, swathIndex);

		FILE * srcFile = fopen(szFile, "w");
		//fwrite(szBuff, 0, strlen(szBuff), srcFile);
		fclose(srcFile);

		return ERROR_CODE_SUCCESS;
	}
	catch (...)
	{
		return ERROR_CODE_OTHER;
	}
}

int TransformBase::CreateTime(const char *pathDst, const char *swathName, int swathIndex)
{
	try
	{
		char szBuff[128] = { "" };
		char szFile[512] = { 0 };
		sprintf(szFile, "%s\\%s_%03d.time", pathDst, swathName, swathIndex);

		//FILE * srcFile = fopen(szFile, "w");
		//fwrite(szBuff, 0, strlen(szBuff), srcFile);
		//fclose(srcFile);
		SwathTime oSwathTime;
		oSwathTime.saveTime(szFile);

		return ERROR_CODE_SUCCESS;
	}
	catch (...)
	{
		return ERROR_CODE_OTHER;
	}
}

int TransformBase::CreateIprh(const char *pathDst, const char *swathName, int swathIndex, int iChannel)
{
	try
	{
		memset(szIprh, 0, 512);
		sprintf(szIprh, "%s\\%s_%03d_A%02d.iprh", pathDst, swathName, swathIndex, iChannel);

		//FILE * srcFile = fopen(szFile, "w");
		//fwrite(szBuff, 0, strlen(szBuff), srcFile);
		//fclose(srcFile);
		oIprh.init(szIprh);

		return ERROR_CODE_SUCCESS;
	}
	catch (...)
	{
		return ERROR_CODE_OTHER;
	}
}
int TransformBase::SaveIprh()
{
	return oIprh.saveHeader(szIprh);
}

int TransformBase::CreateIprb(const char *pathDst, const char *swathName, int swathIndex, int iChannel)
{
	try
	{
		memset(szIprb, 0, 512);
		sprintf(szIprb, "%s\\%s_%03d_A%02d.iprb", pathDst, swathName, swathIndex, iChannel);

		//FILE * srcFile = fopen(szFile, "w");
		//fwrite(szBuff, 0, strlen(szBuff), srcFile);
		//fclose(srcFile);
		oIprb.saveTrace(szIprb);

		return ERROR_CODE_SUCCESS;
	}
	catch (...)
	{
		return ERROR_CODE_OTHER;
	}
}

int TransformBase::SaveIprb()
{
	return oIprb.saveTrace(szIprb);
}


//将Impluse的Channel存入文件
int TransformBase::SaveIprb2File(std::map<long, SwathChannel*> * lstData, const char *swathName, int swathID, const char *swathPathDst)
{
	if (!lstData)
		return 1;
	if (!swathPathDst)
		return 2;
	if (0 == strlen(swathPathDst))
		return 2;

	int channelID = 1;
	//释放所有分配的空间
	for (std::map<long, SwathChannel*>::iterator iter = lstData->begin(); iter != lstData->end(); iter++)
	{
		SwathChannel* pImpluse = iter->second;
		if (!pImpluse)
			continue;

		SwathChannelBlob* impluseBlob = pImpluse->getChannelBlob();
		SwathChannelHeader* impluseHeader = pImpluse->getChannelHeader();

		char iprhFile[512] = { 0 };
		char iprbFile[512] = { 0 };
		sprintf(iprhFile, "%s\\%s_A%02d.iprh", swathPathDst, swathName, channelID);
		sprintf(iprbFile, "%s\\%s_A%02d.iprb", swathPathDst, swathName, channelID);
		impluseHeader->saveHeader(iprhFile);
		impluseBlob->saveTrace(iprbFile);

		channelID++;
	}

	return 0;
}

//将Impluse的time存入文件
int TransformBase::SaveImpluseTime(SwathTime *timeInfo, const char *swathName, int swathID, const char *swathPathDst)
{
	if (!timeInfo)
		return 1;

	char szPathFile[512] = { 0 };
	sprintf(szPathFile, "%s\\%s.time", swathPathDst, swathName);
	timeInfo->saveTime(szPathFile);
	return 0;
}
//将Impluse的cor存入文件
int TransformBase::SaveImpluseCor(SwathCor *corInfo, const char *swathName, int swathID, const char *swathPathDst)
{
	if (!corInfo)
		return 1;

	char szPathFile[512] = { 0 };
	sprintf(szPathFile, "%s\\%s.cor", swathPathDst, swathName);

	corInfo->saveCor(szPathFile);

	return 0;
}


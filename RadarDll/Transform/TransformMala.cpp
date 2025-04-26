/*
* Fun:将Segy数据转化为iprb、iprh格式
*/

//#include "pch.h"
//#include "framework.h"
#include <io.h>
#include <atlimage.h>

//#include "resource.h"		// 主符号

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

//#include "Project.h"
#include "TransformBase.h"
#include "TransformMala.h"

//将Mala的channel数据转化为Impluse的Channel
int TransformMala::ChannelMala2Implus(std::map<long, MalaChannel*> * lstDataMala, std::map<long, SwathChannel*> * lstDataImplus)
{
	if (!lstDataMala)
		return 1;
	if (!lstDataImplus)
		return 2;

	//转化所有分配的空间
	for (std::map<long, MalaChannel*>::iterator iter = lstDataMala->begin(); iter != lstDataMala->end(); iter++)
	{
		MalaChannel* pMala = iter->second;
		if (!pMala)
			continue;

		//获取测线中的Mala通道信息
		MalaChannelBlob *malaBlob = pMala->getBlob();
		MalaChannelHeader *malaHeader = pMala->getHeader();
		int id = pMala->getID();

		SwathChannel *pImpluse = new SwathChannel();
		pImpluse->setNo(id);
		SwathChannelBlob* impluseBlob = pImpluse->getChannelBlob();
		SwathChannelHeader* impluseHeader = pImpluse->getChannelHeader();

		//将GHz转化位MHz处理
		char szAntenna[32] = { 0 };
		strncpy(szAntenna, malaHeader->GetAntennas(), 32);
		char *pUnit = strstr(szAntenna, "GHz");
		if (pUnit)
		{
			char szTmp[16] = { 0, 0, 0, 0, 0, 0, 0, 0 };
			pUnit = pUnit - 4;
			szTmp[0] = pUnit[0];
			szTmp[1] = pUnit[2];
			szTmp[2] = '0';
			szTmp[3] = '0';
			szTmp[4] = ' ';
			szTmp[5] = 'M';
			szTmp[6] = 'H';
			szTmp[7] = 'z';
			szTmp[8] = 0;

			strcpy(szAntenna, szTmp);
		}
		//得到和设置转化后的天线频率
		impluseHeader->setAntenna(szAntenna);

		impluseHeader->setDataVersion(malaBlob->getDataVersion());
		impluseHeader->setAntennaSeparation(malaHeader->GetAntennaSeparation());
		impluseHeader->setSample(malaHeader->GetSample());
		impluseHeader->setFrequency(malaHeader->GetFrequency());
		impluseHeader->setTimeWindow(malaHeader->GetTimeWindow());
		impluseHeader->setTraceCount(malaHeader->GetTraceCount());
		impluseHeader->setIntervalTime(malaHeader->GetTimeInterval());
		impluseHeader->setIntervalDist(malaHeader->GetDistanceInterval());
		impluseHeader->setChannels(malaHeader->GetChannelCount());
		impluseHeader->setDate("2023-03-19");
		impluseHeader->setSoilVel(100);    //波速默认设定100

		int traceCount = 1;
		if (16 == malaBlob->getDataVersion())
		{
			impluseHeader->setDataVersion(16);

			std::map<long, MalaTrace16*> * lstMala = malaBlob->getTrace16List();
			for (std::map<long, MalaTrace16*>::iterator iterMala = lstMala->begin(); iterMala != lstMala->end(); iterMala++)
			{
				MalaTrace16* traceMala = iterMala->second;
				if (!traceMala)
					continue;

				int traceNum = traceMala->getTraceNum();
				int traceSample = traceMala->getSamples();
				short *traceData = traceMala->getTrace();

				short *traceImpluse = (short *)malloc(traceSample * 2);
				memcpy(traceImpluse, traceData, traceSample * 2);

				Trace16 *traceObj = new Trace16();
				traceObj->setTrace(traceImpluse, traceSample);

				impluseBlob->AppendTraceData(traceNum, traceObj, malaBlob->getDataVersion());
				impluseBlob->setChannelParam(16, malaHeader->GetSample(), traceCount);
				traceCount++;
			}
		}
		else if (32 == malaBlob->getDataVersion())
		{
			impluseHeader->setDataVersion(32);

			std::map<long, MalaTrace32*> * lstMala = malaBlob->getTrace32List();
			for (std::map<long, MalaTrace32*>::iterator iterMala = lstMala->begin(); iterMala != lstMala->end(); iterMala++)
			{
				MalaTrace32* traceMala = iterMala->second;
				if (!traceMala)
					continue;

				int traceNum = traceMala->getTraceNum();
				int traceSample = traceMala->getSamples();
				long *traceData = traceMala->getTrace();

				long *traceImpluse = (long *)malloc(traceSample * 4);
				memcpy(traceImpluse, traceData, traceSample * 4);

				Trace32 *traceObj = new Trace32();
				traceObj->setTrace(traceImpluse, traceSample);

				impluseBlob->AppendTraceData(traceNum, traceObj, malaBlob->getDataVersion());
				impluseBlob->setChannelParam(32, malaHeader->GetSample(), traceCount);
				traceCount++;
			}
		}
		lstDataImplus->insert(std::pair<long, SwathChannel*>(id, pImpluse));
	}

	return 0;
}

//将Mala的时间格式转化为Impluse的时间格式
int TransformMala::TimeMala2Implus(SwathTime *timeInfoImpluse, MalaTime *timeInfoMala)
{
	int count = timeInfoMala->getTraceCount();
	for (int i = 1; i <= count; i++)
	{
		MalaTimeData * malaData = timeInfoMala->getData(i);
		if (!malaData)
			continue;

		SwathTimeData * impluseData = new SwathTimeData();
		impluseData->setData(i, malaData->getDateString(), malaData->getTimeString());
		timeInfoImpluse->addData(i, impluseData);
	}

	return 0;
}


/*
* Fun：将指定目录下的Mala数据转化为IPRH、IPRD，存储到指定目录下
* Param：
*      pathMala    Mala数据目录
*      pathDst     IPRH、IPRD目标目录
*      freq        频率
*      separation  通道间隔
* Return：成功返回TRUE，失败返回FALSE
*/
int TransformMala::transformMala32(const char * pathMala, const char * pathDst)
{
	//搜索雷达目录下，指定文件,Rd数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
	char szPathMcor[512] = { 0 };

	//首先寻找Mala32的标志文件
	sprintf(szPathMcor, "%s\\*.mcor", pathMala);
	intptr_t hFileMcor;
	struct _finddata_t oFileInfoMcor;
	if ((hFileMcor = (intptr_t)_findfirst(szPathMcor, &oFileInfoMcor)) == -1L)
	{
		_findclose(hFileMcor);
		printf("没有发现Mala32的测线数据\n");
		return -1;
	}

	int swathIndex = 1;

	//1、循环加载Mala的所有测线
	while (true)
	{
		MalaSwath *malaSwath = new MalaSwath();
		MalaTime *malaTime = NULL;

		//获取测线名称
		char swathName[256] = { 0 };
		if (strlen(oFileInfoMcor.name) > 5)
			strncpy(swathName, oFileInfoMcor.name, strlen(oFileInfoMcor.name) - 5);

		/*----------------------这里处理32位Mala数据的情况-------------------*/
		//搜索雷达目录下，指定文件Mala数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRd7[512] = { 0 };
		sprintf(szPathRd7, "%s\\%s_001.rd7", pathMala, swathName);
		intptr_t hFileRd7;
		struct _finddata_t oFileInfoRd7;

		//如果找到rd7，则说明是32位
		if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) != -1L)
		{
			//这里转化得到的rd7和Rad文件
			malaSwath->init((char *)pathMala, swathName, 32);

			malaTime = new MalaTime();
			//初始化Mala times文件对象
			malaTime->init((char *)pathMala, swathName);
		}
		//关闭rd7文件搜索器
		_findclose(hFileRd7);

		/*----------------------这里将搜索到的数据另存为Impluse-------------------*/
		//Mala测线的通道数据
		std::map<long, MalaChannel*> *m_listDataMala = malaSwath->getData();

		//存放Impluse 的Channel数据
		std::map<long, SwathChannel*> *m_listDataImpluse = new std::map<long, SwathChannel*>();

		//将Mala的Channel转化为Implus
		ChannelMala2Implus(m_listDataMala, m_listDataImpluse);
		//将Impluse数据存储为IPRH\IPRB
		SaveIprb2File(m_listDataImpluse, swathName, swathIndex, (char *)pathDst);
		//释放空间
		for (std::map<long, MalaChannel*>::iterator iter = m_listDataMala->begin(); iter != m_listDataMala->end(); iter++)
		{
			MalaChannel* pMala = iter->second;
			if (!pMala)
				continue;
			delete pMala;
		}
		m_listDataMala->clear();
		for (std::map<long, SwathChannel*>::iterator iterImpluse = m_listDataImpluse->begin(); iterImpluse != m_listDataImpluse->end(); iterImpluse++)
		{
			SwathChannel* pImpluse = iterImpluse->second;
			if (!pImpluse)
				continue;
			delete pImpluse;
		}
		m_listDataImpluse->clear();

		//获取Cor文件内容信息，并且存储
		SwathCor swathCor;//先创建空的cor文件
		SaveImpluseCor(&swathCor, swathName, swathIndex, (char *)pathDst);

		//RD7有对应的Time文件，RD3没有对应的Time文件
		if (malaTime)
		{
			//获取Time文件内容，并且存储
			SwathTime swathTime;
			//将Mala的时间文件转化为Impluse格式
			TimeMala2Implus(&swathTime, malaTime);
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
			delete malaTime;
		}
		else
		{
			//产生空的Time文件
			SwathTime swathTime;
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
		}

		//释放数据
		delete m_listDataImpluse;
		delete malaSwath;

		swathIndex++;
		/*----------------------搜索下一个-------------------*/
		//搜索下一个
		if (_findnext(hFileMcor, &oFileInfoMcor) == 0)
			continue;

		printf("没有Mala的测线数据\n");
		_findclose(hFileMcor);
		break;
	}

	return 0;
}

/*
* Fun：将指定目录下的Mala数据转化为IPRH、IPRD，存储到指定目录下
* Param：
*      pathMala    一种特殊的32位Mala数据目录
*      pathDst     IPRH、IPRD目标目录
*      freq        频率
*      separation  通道间隔
* Return：成功返回TRUE，失败返回FALSE
*/
int TransformMala::transformMala32Ex(const char * pathMala, const char * pathDst)
{
	//搜索雷达目录下，指定文件,Rd数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
	char szPathMcor[512] = { 0 };

	//首先寻找Mala32的标志文件
	sprintf(szPathMcor, "%s\\*.cor", pathMala);
	intptr_t hFileMcor;
	struct _finddata_t oFileInfoMcor;
	if ((hFileMcor = (intptr_t)_findfirst(szPathMcor, &oFileInfoMcor)) == -1L)
	{
		_findclose(hFileMcor);
		printf("没有发现Mala32的测线数据\n");
		return -1;
	}

	int swathIndex = 1;

	//1、循环加载Mala的所有测线
	while (true)
	{
		MalaSwath *malaSwath = new MalaSwath();
		MalaTime *malaTime = NULL;

		//获取测线名称
		char swathName[256] = { 0 };
		if (strlen(oFileInfoMcor.name) > 4)
			strncpy(swathName, oFileInfoMcor.name, strlen(oFileInfoMcor.name) - 4);

		/*----------------------这里处理32位Mala数据的情况-------------------*/
		//搜索雷达目录下，指定文件Mala数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRd7[512] = { 0 };
		sprintf(szPathRd7, "%s\\%s.rd7", pathMala, swathName);
		intptr_t hFileRd7;
		struct _finddata_t oFileInfoRd7;

		//如果找到rd7，则说明是32位
		if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) != -1L)
		{
			//这里转化得到的rd7和Rad文件
			malaSwath->init((char *)pathMala, swathName, 33);

			malaTime = new MalaTime();
			//初始化Mala times文件对象
			malaTime->init((char *)pathMala, swathName);
		}
		//关闭rd7文件搜索器
		_findclose(hFileRd7);

		/*----------------------这里将搜索到的数据另存为Impluse-------------------*/
		//Mala测线的通道数据
		std::map<long, MalaChannel*> *m_listDataMala = malaSwath->getData();

		//存放Impluse 的Channel数据
		std::map<long, SwathChannel*> *m_listDataImpluse = new std::map<long, SwathChannel*>();

		//将Mala的Channel转化为Implus
		ChannelMala2Implus(m_listDataMala, m_listDataImpluse);
		//将Impluse数据存储为IPRH\IPRB
		SaveIprb2File(m_listDataImpluse, swathName, swathIndex, (char *)pathDst);
		//释放空间
		for (std::map<long, MalaChannel*>::iterator iter = m_listDataMala->begin(); iter != m_listDataMala->end(); iter++)
		{
			MalaChannel* pMala = iter->second;
			if (!pMala)
				continue;
			delete pMala;
		}
		m_listDataMala->clear();
		for (std::map<long, SwathChannel*>::iterator iterImpluse = m_listDataImpluse->begin(); iterImpluse != m_listDataImpluse->end(); iterImpluse++)
		{
			SwathChannel* pImpluse = iterImpluse->second;
			if (!pImpluse)
				continue;
			delete pImpluse;
		}
		m_listDataImpluse->clear();

		//获取Cor文件内容信息，并且存储
		SwathCor swathCor;//先创建空的cor文件
		SaveImpluseCor(&swathCor, swathName, swathIndex, (char *)pathDst);

		//RD7有对应的Time文件，RD3没有对应的Time文件
		if (malaTime)
		{
			//获取Time文件内容，并且存储
			SwathTime swathTime;
			//将Mala的时间文件转化为Impluse格式
			TimeMala2Implus(&swathTime, malaTime);
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
			delete malaTime;
		}
		else
		{
			//产生空的Time文件
			SwathTime swathTime;
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
		}

		//释放数据
		delete m_listDataImpluse;
		delete malaSwath;

		swathIndex++;
		/*----------------------搜索下一个-------------------*/
		//搜索下一个
		if (_findnext(hFileMcor, &oFileInfoMcor) == 0)
			continue;

		printf("没有Mala的测线数据\n");
		_findclose(hFileMcor);
		break;
	}

	return 0;
}

/*
* Fun：将指定目录下的Mala数据转化为IPRH、IPRD，存储到指定目录下-----这里处理Mala 16的三维雷达
* Param：
*      pathMala    Mala数据目录
*      pathDst     IPRH、IPRD目标目录
* Return：成功返回TRUE，失败返回FALSE
*/
int TransformMala::transformMala16(const char * pathMala, const char * pathDst)
{
	//搜索雷达目录下，指定文件,Rd数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
	char szPathMcor[512] = { 0 };

	//首先寻找Mala16的标志文件
	sprintf(szPathMcor, "%s\\*.cor", pathMala);
	intptr_t hFileMcor;
	struct _finddata_t oFileInfoMcor;
	if ((hFileMcor = (intptr_t)_findfirst(szPathMcor, &oFileInfoMcor)) == -1L)
	{
		_findclose(hFileMcor);
		printf("没有发现Mala16的测线数据\n");
		return -1;
	}

	int swathIndex = 0;

	//1、循环加载Mala的所有测线
	while (true)
	{
		MalaSwath *malaSwath = new MalaSwath();
		MalaTime *malaTime = NULL;

		//获取测线名称
		char swathName[256] = { 0 };
		if (strlen(oFileInfoMcor.name) > 4)
			strncpy(swathName, oFileInfoMcor.name, strlen(oFileInfoMcor.name) - 4);

		/*----------------------这里处理16位Mala数据的情况-------------------*/
		//搜索雷达目录下，指定文件Mala数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRd3[512] = { 0 };
		sprintf(szPathRd3, "%s\\%s_A%03d.rd3", pathMala, swathName, swathIndex);
		intptr_t hFileRd3;
		struct _finddata_t oFileInfoRd3;

		//如果找到rd3，则说明是16位
		if ((hFileRd3 = (intptr_t)_findfirst(szPathRd3, &oFileInfoRd3)) != -1L)
		{
			//这里转化得到的rd7和Rad文件
			malaSwath->init((char *)pathMala, swathName, 16);
		}
		//关闭rd3文件搜索器
		_findclose(hFileRd3);


		/*----------------------这里将搜索到的数据另存为Impluse-------------------*/
		//Mala测线的通道数据
		std::map<long, MalaChannel*> *m_listDataMala = malaSwath->getData();

		//存放Impluse 的Channel数据
		std::map<long, SwathChannel*> *m_listDataImpluse = new std::map<long, SwathChannel*>();

		//将Mala的Channel转化为Implus
		ChannelMala2Implus(m_listDataMala, m_listDataImpluse);
		//将Impluse数据存储为IPRH\IPRB
		SaveIprb2File(m_listDataImpluse, swathName, swathIndex, (char *)pathDst);
		//释放空间
		for (std::map<long, MalaChannel*>::iterator iter = m_listDataMala->begin(); iter != m_listDataMala->end(); iter++)
		{
			MalaChannel* pMala = iter->second;
			if (!pMala)
				continue;
			delete pMala;
		}
		m_listDataMala->clear();
		for (std::map<long, SwathChannel*>::iterator iterImpluse = m_listDataImpluse->begin(); iterImpluse != m_listDataImpluse->end(); iterImpluse++)
		{
			SwathChannel* pImpluse = iterImpluse->second;
			if (!pImpluse)
				continue;
			delete pImpluse;
		}
		m_listDataImpluse->clear();

		//获取Cor文件内容信息，并且存储
		SwathCor swathCor;//先创建空的cor文件
		SaveImpluseCor(&swathCor, swathName, swathIndex, (char *)pathDst);

		//RD7有对应的Time文件，RD3没有对应的Time文件
		if (malaTime)
		{
			//获取Time文件内容，并且存储
			SwathTime swathTime;
			//将Mala的时间文件转化为Impluse格式
			TimeMala2Implus(&swathTime, malaTime);
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
			delete malaTime;
		}
		else
		{
			//产生空的Time文件
			SwathTime swathTime;
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
		}

		//释放数据
		delete m_listDataImpluse;
		delete malaSwath;

		swathIndex++;
		/*----------------------搜索下一个-------------------*/
		//搜索下一个
		if (_findnext(hFileMcor, &oFileInfoMcor) == 0)
			continue;

		printf("没有Mala的测线数据\n");
		_findclose(hFileMcor);
		break;
	}

	return 0;
}

/*
* Fun：将指定目录下的Mala数据转化为IPRH、IPRD，存储到指定目录下-----这里处理Mala 16的二维雷达
* Param：
*      pathMala    Mala数据目录
*      pathDst     IPRH、IPRD目标目录
* Return：成功返回TRUE，失败返回FALSE
*/
int TransformMala::transformMala16Ex(const char * pathMala, const char * pathDst)
{
	int swathIndex = 0;

	//搜索雷达目录下，指定文件Mala数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
	char szPathRd3[512] = { 0 };
	sprintf(szPathRd3, "%s\\*.rd3", pathMala);
	intptr_t hFileRd3;
	struct _finddata_t oFileInfoRd3;

	//如果找到rd3，则说明是16位
	if ((hFileRd3 = (intptr_t)_findfirst(szPathRd3, &oFileInfoRd3)) == -1L)
	{
		//关闭rd3文件搜索器
		_findclose(hFileRd3);
		return -1;
	}

	//1、循环加载Mala的所有测线
	while (true)
	{
		//获取测线名称
		char swathName[256] = { 0 };
		memset(swathName, 0, 256);

		/*----------------------这里处理16位Mala数据的情况-------------------*/


		strncpy(swathName, oFileInfoRd3.name, strlen(oFileInfoRd3.name) - 7);

		MalaSwath *malaSwath = new MalaSwath();
		MalaTime *malaTime = NULL;
		//这里转化得到的rd7和Rad文件
		malaSwath->init16Ex((char *)pathMala, swathName, 17);

		/*----------------------这里将搜索到的数据另存为Impluse-------------------*/
		//Mala测线的通道数据
		std::map<long, MalaChannel*> *m_listDataMala = malaSwath->getData();

		//存放Impluse 的Channel数据
		std::map<long, SwathChannel*> *m_listDataImpluse = new std::map<long, SwathChannel*>();

		//将Mala的Channel转化为Implus
		ChannelMala2Implus(m_listDataMala, m_listDataImpluse);
		//将Impluse数据存储为IPRH\IPRB
		SaveIprb2File(m_listDataImpluse, swathName, swathIndex, (char *)pathDst);
		//释放空间
		for (std::map<long, MalaChannel*>::iterator iter = m_listDataMala->begin(); iter != m_listDataMala->end(); iter++)
		{
			MalaChannel* pMala = iter->second;
			if (!pMala)
				continue;
			delete pMala;
		}
		m_listDataMala->clear();
		for (std::map<long, SwathChannel*>::iterator iterImpluse = m_listDataImpluse->begin(); iterImpluse != m_listDataImpluse->end(); iterImpluse++)
		{
			SwathChannel* pImpluse = iterImpluse->second;
			if (!pImpluse)
				continue;
			delete pImpluse;
		}
		m_listDataImpluse->clear();

		//获取Cor文件内容信息，并且存储
		SwathCor swathCor;//先创建空的cor文件
		SaveImpluseCor(&swathCor, swathName, swathIndex, (char *)pathDst);

		//RD7有对应的Time文件，RD3没有对应的Time文件
		if (malaTime)
		{
			//获取Time文件内容，并且存储
			SwathTime swathTime;
			//将Mala的时间文件转化为Impluse格式
			TimeMala2Implus(&swathTime, malaTime);
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
			delete malaTime;
		}
		else
		{
			//产生空的Time文件
			SwathTime swathTime;
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
		}

		//释放数据
		delete m_listDataImpluse;
		delete malaSwath;

		swathIndex++;
		/*----------------------搜索下一个-------------------*/
		//搜索下一个
		if (_findnext(hFileRd3, &oFileInfoRd3) != 0)
		{
			printf("没有Mala的测线数据\n");
			_findclose(hFileRd3);
			break;
		}
	}

	return 0;
}


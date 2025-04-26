/*
* Fun:��Segy����ת��Ϊiprb��iprh��ʽ
*/

//#include "pch.h"
//#include "framework.h"
#include <io.h>
#include <atlimage.h>

//#include "resource.h"		// ������

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

//��Mala��channel����ת��ΪImpluse��Channel
int TransformMala::ChannelMala2Implus(std::map<long, MalaChannel*> * lstDataMala, std::map<long, SwathChannel*> * lstDataImplus)
{
	if (!lstDataMala)
		return 1;
	if (!lstDataImplus)
		return 2;

	//ת�����з���Ŀռ�
	for (std::map<long, MalaChannel*>::iterator iter = lstDataMala->begin(); iter != lstDataMala->end(); iter++)
	{
		MalaChannel* pMala = iter->second;
		if (!pMala)
			continue;

		//��ȡ�����е�Malaͨ����Ϣ
		MalaChannelBlob *malaBlob = pMala->getBlob();
		MalaChannelHeader *malaHeader = pMala->getHeader();
		int id = pMala->getID();

		SwathChannel *pImpluse = new SwathChannel();
		pImpluse->setNo(id);
		SwathChannelBlob* impluseBlob = pImpluse->getChannelBlob();
		SwathChannelHeader* impluseHeader = pImpluse->getChannelHeader();

		//��GHzת��λMHz����
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
		//�õ�������ת���������Ƶ��
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
		impluseHeader->setSoilVel(100);    //����Ĭ���趨100

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

//��Mala��ʱ���ʽת��ΪImpluse��ʱ���ʽ
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
* Fun����ָ��Ŀ¼�µ�Mala����ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��
* Param��
*      pathMala    Mala����Ŀ¼
*      pathDst     IPRH��IPRDĿ��Ŀ¼
*      freq        Ƶ��
*      separation  ͨ�����
* Return���ɹ�����TRUE��ʧ�ܷ���FALSE
*/
int TransformMala::transformMala32(const char * pathMala, const char * pathDst)
{
	//�����״�Ŀ¼�£�ָ���ļ�,Rd����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
	char szPathMcor[512] = { 0 };

	//����Ѱ��Mala32�ı�־�ļ�
	sprintf(szPathMcor, "%s\\*.mcor", pathMala);
	intptr_t hFileMcor;
	struct _finddata_t oFileInfoMcor;
	if ((hFileMcor = (intptr_t)_findfirst(szPathMcor, &oFileInfoMcor)) == -1L)
	{
		_findclose(hFileMcor);
		printf("û�з���Mala32�Ĳ�������\n");
		return -1;
	}

	int swathIndex = 1;

	//1��ѭ������Mala�����в���
	while (true)
	{
		MalaSwath *malaSwath = new MalaSwath();
		MalaTime *malaTime = NULL;

		//��ȡ��������
		char swathName[256] = { 0 };
		if (strlen(oFileInfoMcor.name) > 5)
			strncpy(swathName, oFileInfoMcor.name, strlen(oFileInfoMcor.name) - 5);

		/*----------------------���ﴦ��32λMala���ݵ����-------------------*/
		//�����״�Ŀ¼�£�ָ���ļ�Mala����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRd7[512] = { 0 };
		sprintf(szPathRd7, "%s\\%s_001.rd7", pathMala, swathName);
		intptr_t hFileRd7;
		struct _finddata_t oFileInfoRd7;

		//����ҵ�rd7����˵����32λ
		if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) != -1L)
		{
			//����ת���õ���rd7��Rad�ļ�
			malaSwath->init((char *)pathMala, swathName, 32);

			malaTime = new MalaTime();
			//��ʼ��Mala times�ļ�����
			malaTime->init((char *)pathMala, swathName);
		}
		//�ر�rd7�ļ�������
		_findclose(hFileRd7);

		/*----------------------���ｫ���������������ΪImpluse-------------------*/
		//Mala���ߵ�ͨ������
		std::map<long, MalaChannel*> *m_listDataMala = malaSwath->getData();

		//���Impluse ��Channel����
		std::map<long, SwathChannel*> *m_listDataImpluse = new std::map<long, SwathChannel*>();

		//��Mala��Channelת��ΪImplus
		ChannelMala2Implus(m_listDataMala, m_listDataImpluse);
		//��Impluse���ݴ洢ΪIPRH\IPRB
		SaveIprb2File(m_listDataImpluse, swathName, swathIndex, (char *)pathDst);
		//�ͷſռ�
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

		//��ȡCor�ļ�������Ϣ�����Ҵ洢
		SwathCor swathCor;//�ȴ����յ�cor�ļ�
		SaveImpluseCor(&swathCor, swathName, swathIndex, (char *)pathDst);

		//RD7�ж�Ӧ��Time�ļ���RD3û�ж�Ӧ��Time�ļ�
		if (malaTime)
		{
			//��ȡTime�ļ����ݣ����Ҵ洢
			SwathTime swathTime;
			//��Mala��ʱ���ļ�ת��ΪImpluse��ʽ
			TimeMala2Implus(&swathTime, malaTime);
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
			delete malaTime;
		}
		else
		{
			//�����յ�Time�ļ�
			SwathTime swathTime;
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
		}

		//�ͷ�����
		delete m_listDataImpluse;
		delete malaSwath;

		swathIndex++;
		/*----------------------������һ��-------------------*/
		//������һ��
		if (_findnext(hFileMcor, &oFileInfoMcor) == 0)
			continue;

		printf("û��Mala�Ĳ�������\n");
		_findclose(hFileMcor);
		break;
	}

	return 0;
}

/*
* Fun����ָ��Ŀ¼�µ�Mala����ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��
* Param��
*      pathMala    һ�������32λMala����Ŀ¼
*      pathDst     IPRH��IPRDĿ��Ŀ¼
*      freq        Ƶ��
*      separation  ͨ�����
* Return���ɹ�����TRUE��ʧ�ܷ���FALSE
*/
int TransformMala::transformMala32Ex(const char * pathMala, const char * pathDst)
{
	//�����״�Ŀ¼�£�ָ���ļ�,Rd����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
	char szPathMcor[512] = { 0 };

	//����Ѱ��Mala32�ı�־�ļ�
	sprintf(szPathMcor, "%s\\*.cor", pathMala);
	intptr_t hFileMcor;
	struct _finddata_t oFileInfoMcor;
	if ((hFileMcor = (intptr_t)_findfirst(szPathMcor, &oFileInfoMcor)) == -1L)
	{
		_findclose(hFileMcor);
		printf("û�з���Mala32�Ĳ�������\n");
		return -1;
	}

	int swathIndex = 1;

	//1��ѭ������Mala�����в���
	while (true)
	{
		MalaSwath *malaSwath = new MalaSwath();
		MalaTime *malaTime = NULL;

		//��ȡ��������
		char swathName[256] = { 0 };
		if (strlen(oFileInfoMcor.name) > 4)
			strncpy(swathName, oFileInfoMcor.name, strlen(oFileInfoMcor.name) - 4);

		/*----------------------���ﴦ��32λMala���ݵ����-------------------*/
		//�����״�Ŀ¼�£�ָ���ļ�Mala����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRd7[512] = { 0 };
		sprintf(szPathRd7, "%s\\%s.rd7", pathMala, swathName);
		intptr_t hFileRd7;
		struct _finddata_t oFileInfoRd7;

		//����ҵ�rd7����˵����32λ
		if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) != -1L)
		{
			//����ת���õ���rd7��Rad�ļ�
			malaSwath->init((char *)pathMala, swathName, 33);

			malaTime = new MalaTime();
			//��ʼ��Mala times�ļ�����
			malaTime->init((char *)pathMala, swathName);
		}
		//�ر�rd7�ļ�������
		_findclose(hFileRd7);

		/*----------------------���ｫ���������������ΪImpluse-------------------*/
		//Mala���ߵ�ͨ������
		std::map<long, MalaChannel*> *m_listDataMala = malaSwath->getData();

		//���Impluse ��Channel����
		std::map<long, SwathChannel*> *m_listDataImpluse = new std::map<long, SwathChannel*>();

		//��Mala��Channelת��ΪImplus
		ChannelMala2Implus(m_listDataMala, m_listDataImpluse);
		//��Impluse���ݴ洢ΪIPRH\IPRB
		SaveIprb2File(m_listDataImpluse, swathName, swathIndex, (char *)pathDst);
		//�ͷſռ�
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

		//��ȡCor�ļ�������Ϣ�����Ҵ洢
		SwathCor swathCor;//�ȴ����յ�cor�ļ�
		SaveImpluseCor(&swathCor, swathName, swathIndex, (char *)pathDst);

		//RD7�ж�Ӧ��Time�ļ���RD3û�ж�Ӧ��Time�ļ�
		if (malaTime)
		{
			//��ȡTime�ļ����ݣ����Ҵ洢
			SwathTime swathTime;
			//��Mala��ʱ���ļ�ת��ΪImpluse��ʽ
			TimeMala2Implus(&swathTime, malaTime);
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
			delete malaTime;
		}
		else
		{
			//�����յ�Time�ļ�
			SwathTime swathTime;
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
		}

		//�ͷ�����
		delete m_listDataImpluse;
		delete malaSwath;

		swathIndex++;
		/*----------------------������һ��-------------------*/
		//������һ��
		if (_findnext(hFileMcor, &oFileInfoMcor) == 0)
			continue;

		printf("û��Mala�Ĳ�������\n");
		_findclose(hFileMcor);
		break;
	}

	return 0;
}

/*
* Fun����ָ��Ŀ¼�µ�Mala����ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��-----���ﴦ��Mala 16����ά�״�
* Param��
*      pathMala    Mala����Ŀ¼
*      pathDst     IPRH��IPRDĿ��Ŀ¼
* Return���ɹ�����TRUE��ʧ�ܷ���FALSE
*/
int TransformMala::transformMala16(const char * pathMala, const char * pathDst)
{
	//�����״�Ŀ¼�£�ָ���ļ�,Rd����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
	char szPathMcor[512] = { 0 };

	//����Ѱ��Mala16�ı�־�ļ�
	sprintf(szPathMcor, "%s\\*.cor", pathMala);
	intptr_t hFileMcor;
	struct _finddata_t oFileInfoMcor;
	if ((hFileMcor = (intptr_t)_findfirst(szPathMcor, &oFileInfoMcor)) == -1L)
	{
		_findclose(hFileMcor);
		printf("û�з���Mala16�Ĳ�������\n");
		return -1;
	}

	int swathIndex = 0;

	//1��ѭ������Mala�����в���
	while (true)
	{
		MalaSwath *malaSwath = new MalaSwath();
		MalaTime *malaTime = NULL;

		//��ȡ��������
		char swathName[256] = { 0 };
		if (strlen(oFileInfoMcor.name) > 4)
			strncpy(swathName, oFileInfoMcor.name, strlen(oFileInfoMcor.name) - 4);

		/*----------------------���ﴦ��16λMala���ݵ����-------------------*/
		//�����״�Ŀ¼�£�ָ���ļ�Mala����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRd3[512] = { 0 };
		sprintf(szPathRd3, "%s\\%s_A%03d.rd3", pathMala, swathName, swathIndex);
		intptr_t hFileRd3;
		struct _finddata_t oFileInfoRd3;

		//����ҵ�rd3����˵����16λ
		if ((hFileRd3 = (intptr_t)_findfirst(szPathRd3, &oFileInfoRd3)) != -1L)
		{
			//����ת���õ���rd7��Rad�ļ�
			malaSwath->init((char *)pathMala, swathName, 16);
		}
		//�ر�rd3�ļ�������
		_findclose(hFileRd3);


		/*----------------------���ｫ���������������ΪImpluse-------------------*/
		//Mala���ߵ�ͨ������
		std::map<long, MalaChannel*> *m_listDataMala = malaSwath->getData();

		//���Impluse ��Channel����
		std::map<long, SwathChannel*> *m_listDataImpluse = new std::map<long, SwathChannel*>();

		//��Mala��Channelת��ΪImplus
		ChannelMala2Implus(m_listDataMala, m_listDataImpluse);
		//��Impluse���ݴ洢ΪIPRH\IPRB
		SaveIprb2File(m_listDataImpluse, swathName, swathIndex, (char *)pathDst);
		//�ͷſռ�
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

		//��ȡCor�ļ�������Ϣ�����Ҵ洢
		SwathCor swathCor;//�ȴ����յ�cor�ļ�
		SaveImpluseCor(&swathCor, swathName, swathIndex, (char *)pathDst);

		//RD7�ж�Ӧ��Time�ļ���RD3û�ж�Ӧ��Time�ļ�
		if (malaTime)
		{
			//��ȡTime�ļ����ݣ����Ҵ洢
			SwathTime swathTime;
			//��Mala��ʱ���ļ�ת��ΪImpluse��ʽ
			TimeMala2Implus(&swathTime, malaTime);
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
			delete malaTime;
		}
		else
		{
			//�����յ�Time�ļ�
			SwathTime swathTime;
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
		}

		//�ͷ�����
		delete m_listDataImpluse;
		delete malaSwath;

		swathIndex++;
		/*----------------------������һ��-------------------*/
		//������һ��
		if (_findnext(hFileMcor, &oFileInfoMcor) == 0)
			continue;

		printf("û��Mala�Ĳ�������\n");
		_findclose(hFileMcor);
		break;
	}

	return 0;
}

/*
* Fun����ָ��Ŀ¼�µ�Mala����ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��-----���ﴦ��Mala 16�Ķ�ά�״�
* Param��
*      pathMala    Mala����Ŀ¼
*      pathDst     IPRH��IPRDĿ��Ŀ¼
* Return���ɹ�����TRUE��ʧ�ܷ���FALSE
*/
int TransformMala::transformMala16Ex(const char * pathMala, const char * pathDst)
{
	int swathIndex = 0;

	//�����״�Ŀ¼�£�ָ���ļ�Mala����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
	char szPathRd3[512] = { 0 };
	sprintf(szPathRd3, "%s\\*.rd3", pathMala);
	intptr_t hFileRd3;
	struct _finddata_t oFileInfoRd3;

	//����ҵ�rd3����˵����16λ
	if ((hFileRd3 = (intptr_t)_findfirst(szPathRd3, &oFileInfoRd3)) == -1L)
	{
		//�ر�rd3�ļ�������
		_findclose(hFileRd3);
		return -1;
	}

	//1��ѭ������Mala�����в���
	while (true)
	{
		//��ȡ��������
		char swathName[256] = { 0 };
		memset(swathName, 0, 256);

		/*----------------------���ﴦ��16λMala���ݵ����-------------------*/


		strncpy(swathName, oFileInfoRd3.name, strlen(oFileInfoRd3.name) - 7);

		MalaSwath *malaSwath = new MalaSwath();
		MalaTime *malaTime = NULL;
		//����ת���õ���rd7��Rad�ļ�
		malaSwath->init16Ex((char *)pathMala, swathName, 17);

		/*----------------------���ｫ���������������ΪImpluse-------------------*/
		//Mala���ߵ�ͨ������
		std::map<long, MalaChannel*> *m_listDataMala = malaSwath->getData();

		//���Impluse ��Channel����
		std::map<long, SwathChannel*> *m_listDataImpluse = new std::map<long, SwathChannel*>();

		//��Mala��Channelת��ΪImplus
		ChannelMala2Implus(m_listDataMala, m_listDataImpluse);
		//��Impluse���ݴ洢ΪIPRH\IPRB
		SaveIprb2File(m_listDataImpluse, swathName, swathIndex, (char *)pathDst);
		//�ͷſռ�
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

		//��ȡCor�ļ�������Ϣ�����Ҵ洢
		SwathCor swathCor;//�ȴ����յ�cor�ļ�
		SaveImpluseCor(&swathCor, swathName, swathIndex, (char *)pathDst);

		//RD7�ж�Ӧ��Time�ļ���RD3û�ж�Ӧ��Time�ļ�
		if (malaTime)
		{
			//��ȡTime�ļ����ݣ����Ҵ洢
			SwathTime swathTime;
			//��Mala��ʱ���ļ�ת��ΪImpluse��ʽ
			TimeMala2Implus(&swathTime, malaTime);
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
			delete malaTime;
		}
		else
		{
			//�����յ�Time�ļ�
			SwathTime swathTime;
			SaveImpluseTime(&swathTime, swathName, swathIndex, (char *)pathDst);
		}

		//�ͷ�����
		delete m_listDataImpluse;
		delete malaSwath;

		swathIndex++;
		/*----------------------������һ��-------------------*/
		//������һ��
		if (_findnext(hFileRd3, &oFileInfoRd3) != 0)
		{
			printf("û��Mala�Ĳ�������\n");
			_findclose(hFileRd3);
			break;
		}
	}

	return 0;
}


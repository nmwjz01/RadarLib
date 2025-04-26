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
#include "TransformSegy.h"


/*
* Fun����ָ��Ŀ¼�µ�Segy����ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��
* Param��
*      swathPathSegy    Segy����Ŀ¼(sgy��ʽ��������Ŀ¼)
*      pathDst          IPRH��IPRDĿ��Ŀ¼
* Return���ɹ�����0��ʧ�ܷ��ش�����
*/
int TransformSegy::transformSegy(const char * swathPathSegy, const char * pathDst)
{
	//�����״�Ŀ¼�£�ָ���ļ�,Rd����
	char szPathSegy[512] = { 0 };

	//����Ѱ��Mala16�ı�־�ļ�
	sprintf(szPathSegy, "%s\\*.sgy", swathPathSegy);
	intptr_t hFileTxt;
	struct _finddata_t oFileInfo;
	if ((hFileTxt = (intptr_t)_findfirst(szPathSegy, &oFileInfo)) == -1L)
	{
		_findclose(hFileTxt);
		printf("û�з���Segy�Ĳ�������\n");
		return ERROR_NOFILE_SEGY;
	}

	int swathIndex = 0;
	int iResult = ERROR_CODE_SUCCESS;

	//1��ѭ������Segy�����в���
	while (true)
	{
		//��ȡ��������
		char swathName[256] = { 0 };
		if (strlen(oFileInfo.name) > 4)
			strncpy(swathName, oFileInfo.name, strlen(oFileInfo.name) - 4);

		swathIndex++;

		//����õ������ļ����������ļ�ת��λImpluse
		//ת��һ�������ļ�
		iResult = transformSegySwath(swathPathSegy, swathName, pathDst, swathIndex );
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		//������һ����û���������ͷ���
		if (_findnext(hFileTxt, &oFileInfo) == -1L)
			break;
	}
	_findclose(hFileTxt);

	return iResult;
}

//��һ��segy�ļ�ת��Ϊһ��iprh\iprb����
int TransformSegy::transformSegySwath(const char *szPathSegy, const char *swathName, const char *pathDst, int swathIndex)
{
	char swathPathFileSegy[512] = { 0 };
	sprintf(swathPathFileSegy, "%s\\%s.sgy", szPathSegy, swathName);

	//��segy�ļ���
	FILE * srcFile = fopen(swathPathFileSegy, "rb");
	if (!srcFile)
	{
		//MessageBox("���ļ�ʧ��", "��ʾ", MB_OK);
		return ERROR_NOFILE_SEGY;
	}

	unsigned char szBuff[64] = { 0 };

	//����3200�ֽڵ�EBCDIC
	//fseek(srcFile, 3200, SEEK_SET);

	//���������ֽ��У�����3200�ֽڵ�EBCDIC��
	fseek(srcFile, 3220, SEEK_SET);
	//��ȡ��Ӧ�����ݣ�
	//1������������2��ÿ������������ݳ���
	fread(szBuff , 1, 8, srcFile);

	int unsigned byte0 = szBuff[0];
	int unsigned byte1 = szBuff[1];
	int unsigned byte2 = szBuff[2];
	int unsigned byte3 = szBuff[3];
	int unsigned byte4 = szBuff[4];
	int unsigned byte5 = szBuff[5];
	int unsigned byte6 = szBuff[6];
	int unsigned byte7 = szBuff[7];

	int unsigned byte20 = 0;
	int unsigned byte21 = 0;
	int unsigned byte22 = 0;
	int unsigned byte23 = 0;

	//һ��Trace�в����ĵ���
	int samples  = byte0 * 256 + byte1;
	int dataType = byte4 * 256 + byte5; (3 == dataType) ? dataType = 16 : dataType = 32; // else dataType = 32;

	//������ͨ��������
	int iChannel = 1;

	//�������ݿ�ʼ��������� 240�ֽ�����ͷ + ��������
	fseek(srcFile, 3600, SEEK_SET);

	int iResult = ERROR_CODE_SUCCESS;

	//ѭ����ȡ
	while (!feof(srcFile))
	{
		std::map<int, void*>*lstDataTmp = new std::map<int, void*>();
		int iTraceCount = 0;


		//����һ��iprb�ļ�
		iResult = CreateIprb(pathDst, swathName, swathIndex, iChannel);
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		iResult = ERROR_CODE_SUCCESS;

		//ѭ����ȡһ��ͨ����iprb����
		while (!feof(srcFile))
		{
			int iReadTimes = 5;    //���ֻ�ܶ�ȡ5��
			int iReadLen  = 32;    //Ҫ���ȡ32�ֽ�����
			while (iReadLen > 0)
			{
				iReadLen = iReadLen - (int)fread(szBuff + 32 - iReadLen, 1, iReadLen, srcFile);

				iReadTimes--;
				if (iReadTimes <= 0)
					break;
			}
			//��ȡ������˵�������Ѿ�û����
			if (0 != iReadLen)
			{
				iResult = ERROR_FILE_READ;
				break;
			}

			byte0 = szBuff[0];
			byte1 = szBuff[1];
			byte2 = szBuff[2];
			byte3 = szBuff[3];
			byte4 = szBuff[4];
			byte5 = szBuff[5];
			byte6 = szBuff[6];
			byte7 = szBuff[7];

			byte20 = szBuff[20];
			byte21 = szBuff[21];
			byte22 = szBuff[22];
			byte23 = szBuff[23];

			//����ͨ����
			int iChannelTmp = ((byte4 * 256  + byte5 ) * 256 + byte6 ) * 256 + byte7;
			//����Trace��
			//int iTraceNum   = ((byte20 * 256 + byte21) * 256 + byte22) * 256 + byte23;

			//����һ��Trace��������
			int iDataLen = samples * dataType / 8;

			//����ÿ��Traceͷ��240�ֽ�(֮���Լ�ȥ32�ֽڣ�������Ϊǰ���ȡ��32�ֽ�)
			fseek(srcFile, 240 - 32, SEEK_CUR);

			unsigned char *szData = (unsigned char *)malloc(iDataLen);
			iReadTimes = 5;           //���ֻ�ܶ�ȡ5��
			iReadLen   = iDataLen;    //Ҫ���ȡ�ֽ�����һ��Trace��������
			while (iReadLen > 0)
			{
				int iLengthTmp = (int)fread(szData + iDataLen - iReadLen, 1, iReadLen, srcFile);
				iReadLen = iReadLen - iLengthTmp;

				iReadTimes--;
				if (iReadTimes <= 0)
					break;
			}
			//��ȡ������˵�������Ѿ�û����
			if (0 != iReadLen)
			{
				iResult = ERROR_FILE_READ;
				break;
			}
			if (16 == dataType)
			{
				//����Buff��Сͷת�� --- 16λ����ת��
				for (int i = 0; i < samples; i++)
				{
					unsigned char iTmp = szData[2 * i];
					szData[2 * i] = szData[2 * i + 1];
					szData[2 * i + 1] = iTmp;
				}
			}
			else
			{
				//����Buff��Сͷת�� --- 16λ����ת��
				for (int i = 0; i < samples; i++)
				{
					float *f1 = (float*)(szData + 4 * i);

					unsigned char iTmp = szData[4 * i];
					szData[4 * i] = szData[4 * i + 3];
					szData[4 * i + 3] = iTmp;

					iTmp = szData[4 * i + 1];
					szData[4 * i + 1] = szData[4 * i + 2];
					szData[4 * i + 2] = iTmp;

					float *f2 = (float *)( szData + 4*i );
					printf( "%f", *f2 );
				}
			}
			//ͨ���Ų�һ����˵���Ѿ����һ��ͨ���Ķ�ȡ
			if (iChannelTmp != iChannel)
			{
				fseek(srcFile, - 240 - iDataLen, SEEK_CUR);
				break;
			}

			void *pTrace = NULL;
			if (16 == dataType)
			{
				pTrace = new Trace16();
				((Trace16 *)pTrace)->setTrace((short*)szData, samples);
			}
			else
			{
				pTrace = new Trace32();
				((Trace32 *)pTrace)->setTrace((long*)szData, samples);
			}
			lstDataTmp->insert(std::pair<int, void*>(iTraceCount, pTrace));

			iTraceCount++;
		}

		//���ԭ�е�����
		oIprb.clearTraceData();
		//����iprb�Ĳ���
		oIprb.setChannelParam(dataType, samples, 0);
		//���������
		oIprb.AppendTraceData(lstDataTmp, dataType);
		lstDataTmp->clear();
		//�洢iprb
		iResult = SaveIprb();
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		//����һ��iprh�ļ�
		iResult = CreateIprh(pathDst, swathName, swathIndex, iChannel);
		if (ERROR_CODE_SUCCESS != iResult)
			break;
		//����iprh�ļ�����
		oIprh.setDataVersion(dataType);
		oIprh.setSample(samples);
		oIprh.setFrequency(5120.0);
		oIprh.setSoilVel(100.0);
		oIprh.setTraceCount(iTraceCount);
		//�洢iprh�ļ�����
		iResult = SaveIprh();
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		//ͨ��������һ
		iChannel++;
	}

	fclose(srcFile);


	//����iprhͨ������--����cor�ļ�
	iResult = CreateCor(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;
	//����iprhͨ������--����gps�ļ�
	iResult = CreateGps(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;
	//����iprhͨ������--����mrk�ļ�
	iResult = CreateMrk(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;
	//����iprhͨ������--����ord�ļ�
	iResult = CreateOrd(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;
	//����iprhͨ������--����time�ļ�
	iResult = CreateTime(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;

	return iResult;
}

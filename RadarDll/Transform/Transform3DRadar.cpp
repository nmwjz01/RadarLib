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
#include "Transform3DRadar.h"



/*
* Fun����ָ��Ŀ¼�µ�3DRadar����ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��
* Param��
*      swathPath3DRadar    3DRadar����Ŀ¼(txt��ʽ��������Ŀ¼)
*      pathDst             IPRH��IPRDĿ��Ŀ¼
* Return���ɹ�����TRUE��ʧ�ܷ���FALSE
*/
int Transform3DRadar::transform3DRadar(const char * swathPath3DRadar, const char * pathDst)
{
	//�����״�Ŀ¼�£�ָ���ļ�,Rd����
	char szPathTxt[512] = { 0 };

	//����Ѱ��Mala16�ı�־�ļ�
	sprintf(szPathTxt, "%s\\*.txt", swathPath3DRadar);
	intptr_t hFileTxt;
	struct _finddata_t oFileInfoTxt;
	if ((hFileTxt = (intptr_t)_findfirst(szPathTxt, &oFileInfoTxt)) == -1L)
	{
		_findclose(hFileTxt);
		printf("û�з���3DRadar�Ĳ�������\n");
		return ERROR_NOFILE_3DRADAR;
	}

	int iResult = ERROR_CODE_SUCCESS;
	int swathIndex = 0;

	//1��ѭ������3DRadar�����в���
	while (true)
	{
		//��ȡ��������
		char swathName[256] = { 0 };
		if (strlen(oFileInfoTxt.name) > 4)
			strncpy(swathName, oFileInfoTxt.name, strlen(oFileInfoTxt.name) - 4);

		//����õ������ļ����������ļ�ת��λImpluse
		//ת��һ�������ļ�
		iResult = transform3DRadarSwath(swathPath3DRadar, pathDst, swathName);
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		swathIndex++;

		//������һ����û���������ͷ���
		if (_findnext(hFileTxt, &oFileInfoTxt) == -1L)
			break;
	}
	_findclose(hFileTxt);

	return iResult;
}

//��һ��3Dradar�ļ�ת��Ϊһ��iprh\iprb����
int Transform3DRadar::transform3DRadarSwath(const char * swathPath3DRadar, const char * pathDst, const char * swathName)
{
	char swathPathFile3DRadar[512] = { 0 };
	sprintf(swathPathFile3DRadar, "%s\\%s.txt", swathPath3DRadar, swathName);

	//��
	FILE * srcFile = fopen(swathPathFile3DRadar, "r");
	if (!srcFile)
	{
		//MessageBox("���ļ�ʧ��", "��ʾ", MB_OK);
		return ERROR_NOFILE_3DRADAR;
	}

	//������Trace������
	int lastTrace = 0;
	//������ͨ��������
	int inLines = 0;
	//һ��Trace�в����ĵ���
	int samples = 0;
	//��ȡÿ�����ߵ�Trace������ͨ���������Լ�ÿ��Trace�Ĳ�������
	{
		//���һ���ı�
		char buffLline[512] = { 0 };
		//��ȡ���ļ�����
		int countLine = 0;

		//ѭ����ȡ
		while (!feof(srcFile))
		{
			//��ʼ������
			memset(buffLline, 0, 512);

			char * lineIndex = buffLline;
			//��¼�ַ����Ŀ�ʼ
			char * lineStart = buffLline;
			//��¼��ȡ�ĳ���()
			int countChar = 0;
			//ѭ����ȡ
			while (!feof(srcFile))
			{
				if (countChar >= 510)
					break;

				fread(lineIndex, 1, 1, srcFile);

				//����õ�������
				if ('\n' == lineIndex[0])
					break;

				lineIndex++;
				countChar++;
			}

			//�õ�һ�����ݣ������ж��Ƿ����Ŀ���ַ���
			if (strstr(lineStart, "#Volume: X-lines="))
			{
				//��ȡһ�������е�Trace����
				lastTrace = getLastTraceByTxt(lineStart);
			}
			if (strstr(lineStart, "In-lines="))
			{
				//��ȡͨ������
				inLines = getInLinesByTxt(lineStart);
			}
			if (strstr(lineStart, "Samples="))
			{
				//��ȡÿ��Trace�Ĳ�������
				samples = getSamplesByTxt(lineStart);
			}

			//�ļ�ͷ�����һ��
			if (strstr(lineStart, "\n") && strstr(lineStart, "#RegionGainData:"))
				break;
			//���ļ���ͷ�Ѿ���ȡ10�У���û�з����ļ�ͷ�����һ��
			if (countLine >= 10)
				break;

			countLine++;
		}
	}

	//Impluse cor�ļ�
	char swathPathFileImpluseCor[512] = { 0 };
	sprintf(swathPathFileImpluseCor, "%s\\%s.cor", pathDst, swathName);
	//Impluse time�ļ�
	char swathPathFileImpluseTime[512] = { 0 };
	sprintf(swathPathFileImpluseTime, "%s\\%s.time", pathDst, swathName);

	FILE * dstFileCor = fopen(swathPathFileImpluseCor, "wb+");
	FILE * dstFileTime = fopen(swathPathFileImpluseTime, "wb+");
	fclose(dstFileCor);
	fclose(dstFileTime);

	//ѭ��ÿ��ͨ������ȡ���ݣ�д��Ŀ��
	for (int i = 0; i < inLines; i++)
	{
		char swathPathFileImpluseIprb[512] = { 0 };
		char swathPathFileImpluseIprh[512] = { 0 };
		sprintf(swathPathFileImpluseIprb, "%s\\%s_A%02d.iprb", pathDst, swathName, i + 1);
		sprintf(swathPathFileImpluseIprh, "%s\\%s_A%02d.iprh", pathDst, swathName, i + 1);   //like "C:\\3D_TestData\\RadarData\\20231011-TEST_001_A01.iprb";

		FILE * dstFileIprb = fopen(swathPathFileImpluseIprb, "wb+");
		FILE * dstFileIprh = fopen(swathPathFileImpluseIprh, "wb+");

		//��ȡһ��ͨ��������Trace��д��iprb�ļ�
		for (int j = 0; j < lastTrace; j++)
		{
			char szLatitude[64] = { 0 };
			char szLongitude[64] = { 0 };
			fscanf(srcFile, "%s	%s", szLatitude, szLongitude);

			//��ȡһ��Trace������sample����
			unsigned char szBuffDst[1324] = { 0 };
			for (int k = 0; k < samples; k++)
			{
				unsigned int iTmp = 0;
				fscanf(srcFile, "%d", &iTmp);
				//iTmp = htonl(iTmp);

				unsigned char *pTmp = szBuffDst + 2 * k;
				memcpy(pTmp, &iTmp, 2);
			}
			//��һ��Trace�е�����sampleд��Ŀ��
			int lenDst = (int)fwrite(szBuffDst, 1, samples * 2, dstFileIprb);
			if (samples * 2 != lenDst)
			{
				printf("Error");
			}
		}
		fclose(dstFileIprb);

		//����Ҫ����iprh�ļ�
		{
			//=======д�ļ�========//
			char szBuff[64] = { 0 };
			sprintf(szBuff, "HEADER VERSION: 10\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "DATA VERSION: 16\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "DATE: 2022-11-10\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "START TIME: 12:08:46\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "STOP TIME: 12:09:48\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA: %d MHz\n", 500);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA SEPARATION: %f\n", 1.1);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "SAMPLES: %d\n", samples);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "SIGNAL POSITION: 12\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "CLIPPED SAMPLES: 0\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "RUNS: 32\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "MAX STACKS: 512\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "AUTOSTACKS: 1\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "FREQUENCY: %d\n", 500);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "TIMEWINDOW: %d\n", 80);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "LAST TRACE: %d\n", lastTrace);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "TRIG SOURCE: wheel\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "TIME INTERVAL: 0.050000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "DISTANCE INTERVAL: %f\n", 1.1);
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "USER DISTANCE INTERVAL: 0.050000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "STOP POSITION: 368.635412\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "WHEEL NAME: New_Wheel123\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "WHEEL CALIBRATION:477.8000000000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ZERO LEVEL: 40\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "SOIL VELOCITY: 100.000000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "PREPROCESSING: 0\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "OPERATOR COMMENT: _\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA F/W: 48001262\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA H/W: 0\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA FPGA: DA74\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "ANTENNA SERIAL: 1579\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "SOFTWARE VERSION: T 1.2.20\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "POSITIONING: 2\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);


			sprintf(szBuff, "CHANNELS: 18\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "CHANNEL CONFIGURATION: T2 - R2\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);


			sprintf(szBuff, "CH_X_OFFSET: 0.000000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			sprintf(szBuff, "CH_Y_OFFSET: 0.000000\n");
			fwrite(szBuff, 1, strlen(szBuff), dstFileIprh);

			fclose(dstFileIprh);
		}
	}

	return ERROR_CODE_SUCCESS;
}

int Transform3DRadar::getLastTraceByTxt(char * lineStart)
{
	char * tmp = strstr(lineStart, "X-lines=");
	if (NULL == tmp)
		return 0;
	else
	{
		tmp = tmp + 8;
		char * tmp1 = strstr(tmp, ",");

		char strNum[20] = { 0 };
		strncpy(strNum, tmp, tmp1 - tmp);

		return atoi(strNum);
	}
}
int Transform3DRadar::getInLinesByTxt(char * lineStart)
{
	char * tmp = strstr(lineStart, "In-lines=");
	if (NULL == tmp)
		return 0;
	else
	{
		tmp = tmp + 9;
		char * tmp1 = strstr(tmp, ",");

		char strNum[20] = { 0 };
		strncpy(strNum, tmp, tmp1 - tmp);

		return atoi(strNum);
	}
}
int Transform3DRadar::getSamplesByTxt(char * lineStart)
{
	char * tmp = strstr(lineStart, "Samples=");
	if (NULL == tmp)
		return 0;
	else
	{
		tmp = tmp + 8;
		char * tmp1 = strstr(tmp, "\n");

		char strNum[20] = { 0 };
		strncpy(strNum, tmp, tmp1 - tmp);

		return atoi(strNum);
	}
}

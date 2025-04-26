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
#include "TransformDT.h"


/*
Fun:��DT�ļ��D����iprb�ļ�
Param:
	strDT �����DT�ļ�
	strIPRB �D��Ŀ���iprb�ļ�
	height  ÿ�βɘӵ��c��
	width   Trace����
*/
int TransformDT::dt2iprb(char *szDT, char *szIPRB, int height, int width)
{
	char szLength[4] = { 0 };
	//���M�ƴ��ļ�
	FILE * pFileDT = fopen(szDT, "rb");
	if (!pFileDT)
		return false;

	//��ȡ����
	fread(szLength, 1, 4, pFileDT);
	memset(szLength, 0, 4);
	fread(szLength, 1, 2, pFileDT);

	//�������D�QΪ����
	int length = szLength[0] * 256 + szLength[1];
	//��λ��ȡλ��
	fseek(pFileDT, length, SEEK_SET);

	//ѭ�h��ȡ�ļ��^��Ϣ
	while (!feof(pFileDT))
	{
		char szTitle[8] = { 0 };
		//��ȡ����
		fread(szTitle, 1, 4, pFileDT);
		//ȥ���Ƿ��ַ�
		if ((0x15 == szTitle[0]) || (0x4 == szTitle[0]) || (0x7 == szTitle[0])) szTitle[0] = 0;
		if ((0x15 == szTitle[1]) || (0x4 == szTitle[1]) || (0x7 == szTitle[1])) szTitle[1] = 0;
		if ((0x15 == szTitle[2]) || (0x4 == szTitle[2]) || (0x7 == szTitle[2])) szTitle[2] = 0;
		if ((0x15 == szTitle[3]) || (0x4 == szTitle[3]) || (0x7 == szTitle[3])) szTitle[3] = 0;

		//���������ж�
		if (0 == strcmp(szTitle, "FI"))
		{
		}
		else if (0 == strcmp(szTitle, "I"))
		{
		}
		else if (0 == strcmp(szTitle, "C"))
		{
		}
		else if (0 == strcmp(szTitle, "AH"))
		{
		}
		else if (0 == strcmp(szTitle, "FZ"))
		{
		}
		else if (0 == strcmp(szTitle, "FX"))
		{
		}
		else if (0 == strcmp(szTitle, "FQ"))
		{
		}
		else if (0 == strcmp(szTitle, "AC1"))
		{
		}
		else if (0 == strcmp(szTitle, "AM"))   //AM0��
		{
		}
		else if (0 == strcmp(szTitle, "ATR"))
		{
		}
		else if (0 == strcmp(szTitle, "ATX"))
		{
		}
		else if (0 == strcmp(szTitle, "AA"))
		{
		}
		else if (0 == strcmp(szTitle, "S"))
		{
		}
		else if (0 == strcmp(szTitle, "FW"))
		{
		}
		else if (0 == strcmp(szTitle, "H"))
		{
		}
		else if (0 == strcmp(szTitle, "FC"))
		{
		}
		else if (0 == strcmp(szTitle, "FS"))
		{
		}
		else if (0 == strcmp(szTitle, "FT"))
		{
		}
		else if (0 == strcmp(szTitle, "FO"))
		{
		}
		else if (0 == strcmp(szTitle, "FN"))
		{
		}
		else if (0 == strcmp(szTitle, "ARX"))
		{
		}
		else if (0 == strcmp(szTitle, "GI"))
		{
		}
		else if (0 == strcmp(szTitle, "R"))
		{
			//��ȡ���
			break;
		}

		//��λ��һ����ȡλ��
		fseek(pFileDT, length - 4, SEEK_CUR);
	}

	//�������ݴ惦�ռ�
	unsigned char *dtData = (unsigned char *)malloc(height * width * 2);
	unsigned char *tmpWrite = dtData;
	int i = 0;
	//ѭ�h��ȡ�ļ��n��Ϣ
	while (true)
	{
		unsigned char tmp[4] = { 0 };
		//��ȡ����
		fread(tmp, 2, 2, pFileDT);

		int size = 2 * height;
		//��ȡ����(֮�����@�Ӷ�ȡ������ÿ�ζ�ȡ���ص������������_��)
		while (true)
		{
			int tmp = (int)fread(tmpWrite, 1, size, pFileDT);

			tmpWrite = tmpWrite + tmp;
			size = size - tmp;

			if (feof(pFileDT))
				break;
			if (!size)
				break;
		}

		//�惦λ��ָ����һ��߀δ��д���ݵą^��
		//tmpWrite = tmpWrite + 2 * height;

		if (feof(pFileDT))
			break;

		//���ݿ��ܻᳬ�^
		i++;
		if (i >= width)
			break;
	}
	//�P�]ԭʼ�ļ�
	fclose(pFileDT);

	//=========����дiprb�ļ�========//
	//���M�ƴ��ļ�
	FILE * pFileIPRB = fopen(szIPRB, "wb");
	if (!pFileIPRB)
	{
		free(dtData);
		return false;
	}
	fwrite(dtData, 2, height * width, pFileIPRB);
	//�P�]Ŀ���ļ�
	fclose(pFileIPRB);

	free(dtData);
	return true;
}

/*
Fun:����IPRH�ļ�
param  szDTH       dt��Ӧ���^�ļ�������LA010001.HDR_00
	   freq        �����l��
	   ns          ʱ�䴰
	   distance    �������
	   separation  ����֮��ľ��x

	   tracecount  ���ز������^�ļ��м�¼��trace����
*/
int TransformDT::dt2iprh(char *szDTH, int freq, int ns, float distance, float separation, char *szIPRH, int &tracecount)
{
	int lastTrace = 0;
	int samples = 0;

	char szContent[512] = { 0 };
	//��ȡlast trace��sample
	FILE * pFileDTH = fopen(szDTH, "r");
	if (!pFileDTH)
		return false;
	int size = 512;
	while (true)
	{
		size = size - (int)fread(szContent, 1, size, pFileDTH);
		if (size <= 0)
			break;
		if (feof(pFileDTH))
			break;
	}
	//�P�]ԭʼ�ļ�
	fclose(pFileDTH);

	//����Ŀ������
	char *tmp = strstr(szContent, "<CAMP>"); if (!tmp)		return false;
	tmp = strstr(tmp, "  ");                 if (!tmp)		return false;
	sscanf(tmp, " %d         %d", &lastTrace, &samples);

	//���ļ�
	FILE * pFileIPRH = fopen(szIPRH, "w+");
	if (!pFileIPRH)
		return false;

	//=======д�ļ�========//
	char szBuff[64] = { 0 };
	sprintf(szBuff, "HEADER VERSION: 10\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "DATA VERSION: 16\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "DATE: 2022-11-10\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "START TIME: 12:08:46\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "STOP TIME: 12:09:48\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "ANTENNA: %d MHz\n", freq);
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "ANTENNA SEPARATION: %f\n", separation);
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "SAMPLES: %d\n", samples);
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "SIGNAL POSITION: 12\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "CLIPPED SAMPLES: 0\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "RUNS: 32\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "MAX STACKS: 512\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "AUTOSTACKS: 1\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "FREQUENCY: %d\n", freq);
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "TIMEWINDOW: %d\n", ns);
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "LAST TRACE: %d\n", lastTrace);
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "TRIG SOURCE: wheel\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "TIME INTERVAL: 0.050000\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "DISTANCE INTERVAL: %f\n", distance);
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "USER DISTANCE INTERVAL: 0.050000\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "STOP POSITION: 368.635412\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "WHEEL NAME: New_Wheel123\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "WHEEL CALIBRATION:477.8000000000\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "ZERO LEVEL: 40\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "SOIL VELOCITY: 100.000000\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "PREPROCESSING: 0\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "OPERATOR COMMENT: _\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "ANTENNA F/W: 48001262\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "ANTENNA H/W: 0\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "ANTENNA FPGA: DA74\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "ANTENNA SERIAL: 1579\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "SOFTWARE VERSION: T 1.2.20\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "POSITIONING: 2\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);


	sprintf(szBuff, "CHANNELS: 18\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "CHANNEL CONFIGURATION: T2 - R2\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);


	sprintf(szBuff, "CH_X_OFFSET: 0.000000\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	sprintf(szBuff, "CH_Y_OFFSET: 0.000000\n");
	fwrite(szBuff, 1, strlen(szBuff), pFileIPRH);

	fclose(pFileIPRH);

	//��¼trace����
	tracecount = lastTrace;
	return true;
}




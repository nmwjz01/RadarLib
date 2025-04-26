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
#include "TransformIDS.h"



TransformIDS::TransformIDS()
{
	//����һ���ṩͨ�����Զ���
	headVersion = 10;        // header version: 10
	dataVersion = 16;        // data version: 16-16b; 32-32b 
	strcpy(date, "2023-10-22");       //���߲�������
	strcpy(startTime, "00:00:00");    //���߿�ʼʱ��
	strcpy(stopTime, "23:59:59");     //���߽���ʱ��
	strcpy(ANTENNA, "200 MHz");       //Ƶ�ʣ�
	separation   = 0.01;              //Ĭ�Ϲ�դ����0.01
	samples      = 512;               //һ��trace�У���������
	runs         = 0;
	maxStacks    = 0;
	frequency    = 200;        // Sampling Frequency (MHz)����һ��Ƶ�ʣ�
	timeWindow   = 60;         //ʱ�䴰�����İl�����ص�ʱ��
	lastTrace    = 0;
	strcpy(trigSource, "wheel");
	intervalTime = 0.05;       // time interval (seconds)
	intervalDist = 0.2;        // real distance interval (meters)
	intervalUser = 0.05;       // user distance interval (meters)
	zeroLevel    = 0;
	soilVel      = 100;        // soil velocity (m/us)
	positioning  = 0;          // it is used for combined files only
	channels     = 0;
	memset(chanName, 0, 16);      // channel name (like "T1-R1")
}
TransformIDS::~TransformIDS()
{
	printf("ת����һ�������ļ�����\n");
}

/*
 * Fun:����cor�ļ�
 * Param: pathDst�ļ����Ŀ¼
 *        swathName ������
 *        swathID ����ID
 * ����:�ɹ�����true��ʧ�ܷ���false
 */
bool TransformIDS::makeFileCor(const char * pathDst, const char *swathName, int swathID)
{
	char szSwathFile[512] = { 0 };
	sprintf(szSwathFile, "%s\\Swath%03d.cor", pathDst, swathID);

	FILE *fp = fopen(szSwathFile, "w+");

	char szContext[1024] = { 0 };
	sprintf(szContext, "%s", "1	2022-09-28	07:20:45:601	22.57101145217	N	113.93479769317	E	31.531	M	1\n");

	fwrite(szContext, 1, strlen(szContext), fp);

	fclose(fp);

	return true;
}

/*
 * Fun:����time�ļ�
 * Param: pathDst�ļ����Ŀ¼
 *        swathName ������
 *        swathID ����ID
 * ����:�ɹ�����true��ʧ�ܷ���false
 */
bool TransformIDS::makeFileTime(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
{
	//===========����Ϊת��IDS��Index�ļ���Impluse��Time�ļ�==========//
	//ȡ����ʼʱ��
	char fileSurvey[1024] = {}; sprintf(fileSurvey, "%s\\Survey", pathSrc);
	//���岢�Ҵ�IDS��Survey�ļ�
	FILE *fpSurvey = fopen(fileSurvey, "r");
	if (!fpSurvey)
		return false;
	char szSurvey[256] = { 0 };
	fread(szSurvey, 1, 250, fpSurvey);
	fclose(fpSurvey);
	char *tmp = strstr(szSurvey, "Begin=\"");   //ʱ�侫ȷ����
	tmp = tmp + 7;

	long long timeStart = 0;
	//���ַ���ת��Ϊ����
	Utils::char2int(tmp, timeStart);

	//д��time�ļ�
	char fileTimeIDS[1024] = { 0 }; sprintf(fileTimeIDS, "%s\\%s_Array01.index", pathSrc, swathName);
	char fileTimeImpluse[1024] = { 0 }; sprintf(fileTimeImpluse, "%s\\Swath%03d.time", pathDst, swathID);
	//���岢�Ҵ�IDS��index�ļ�
	FILE *fpTimeIDS = fopen(fileTimeIDS, "rb");
	if (!fpTimeIDS)
		return false;
	//���岢�Ҵ�impluse��time�ļ�
	FILE *fpTimeImpluse = fopen(fileTimeImpluse, "wb");
	if (!fpTimeImpluse)
	{
		fclose(fpTimeIDS);
		return false;
	}

	//���ڴ洢ÿ��ʱ��Ķ�ȡ����
	unsigned char buffTime[20] = { 0 };
	static long long offset = 0;
	/*
	//����ǵ�һ�����ߣ����¼������ʼʱ��ƫ��
	if (1 == swathID)
	{
		//����IDS index���ļ�ͷ(120�ֽڣ�����trace��Ϊ0������)
		fseek(fpTimeIDS, 100, SEEK_SET);
		fread(buffTime, 1, 20, fpTimeIDS);

		offset = buffTime[7] * 256;   //ȡ��Traceʱ�䣨��Կ�ʼ��ʱ��--���룩
		offset = (offset + buffTime[6]) * 256;
		offset = (offset + buffTime[5]) * 256;
		offset = (offset + buffTime[4]);
	}
	else
		//����IDS index���ļ�ͷ(120�ֽڣ�����trace��Ϊ0������)
		fseek(fpTimeIDS, 120, SEEK_SET);
	*/

	//*
	int channelCount = 0;
	unsigned char buffChannel[20] = { 0 };
	//����ͨ��������λ��
	fseek(fpTimeIDS, 44, SEEK_SET);
	//��ȡͨ�����������ݳ�����4�ֽڣ�����ֻ��Ҫ1���ֽڼ���
	fread(buffChannel, 1, 4, fpTimeIDS);
	channelCount = buffChannel[0];
	//�����ļ�ͷ�ĳ��ȣ�
	//             4�ֽ��ļ���ʶ   + 32�ֽ�MD5    + 4�ֽڱ�־���ߵĵڼ����ļ� + 4�ֽ�����Ƶ�� + 
	//             4�ֽ�ͨ����     + 4�ֽ�Sample��+ 4�ֽ�δ֪                 + 4�ֽ�δ֪     + ͨ�����б�ͨ������*4��
	int lenHeader = 4 + 32 + 4 + 4 +
	                4 +  4 + 4 + 4 + channelCount * 4;
	//�����ļ�ͷ
	fseek(fpTimeIDS, lenHeader, SEEK_SET);
	//*/

	//����ѭ���������trace,һֱ���ļ���β
	while (!feof(fpTimeIDS))
	{
		fread(buffTime, 1, 20, fpTimeIDS);
		int traceNum = *((int *)buffTime);        //ȡ��Trace��
		long long traceTimeMsecond = buffTime[7] * 256;   //ȡ��Traceʱ�䣨��Կ�ʼ��ʱ��--���룩
		traceTimeMsecond = (traceTimeMsecond + buffTime[6]) * 256;
		traceTimeMsecond = (traceTimeMsecond + buffTime[5]) * 256;
		traceTimeMsecond = (traceTimeMsecond + buffTime[4]) - offset;
		long long  traceTime = traceTimeMsecond / 1000 + timeStart * 1000;    //Traceʱ�䣬����ʱ��--����
		char szTmp[128] = { 0 };
		char szLine[128] = { 0 };

		Utils::long2date(traceTime, szTmp);               //��ʱ��ת��Ϊ�ַ�����ʽ
		sprintf(szLine, "%d	%s\r\n", traceNum, szTmp);
		fwrite(szLine, 1, strlen(szLine), fpTimeImpluse);
	}
	fclose(fpTimeIDS);
	fclose(fpTimeImpluse);

	return true;
}

/*
 * Fun:�洢iprh�ļ�ͷ��Ϣ
 * Param:
 * Return:�ɹ�����true��ʧ�ܷ���false
 */
int TransformIDS::saveHeader(const char * pathDst, const char *swathName, int channelNum)
{
	frequency = channelFreq[channelNum];
	intervalDist = traceStep[channelNum];

	//�����Ϸ���У��
	if ((nullptr == pathDst) || (nullptr == swathName))
		return -1;
	if ((0 >= strlen(pathDst)) || (0 >= strlen(swathName)))
		return -1;

	char pathFile[1024] = { 0 };
	sprintf(pathFile, "%s\\%s_A%02d.iprh", pathDst, swathName, channelNum);

	FILE * m_pFile = fopen(pathFile, "wt+");
	if (nullptr == m_pFile)
		return -2;

	//ѭ����ȡ�������ò���
	char s[300];

	sprintf(s, "HEADER VERSION: %d\n", headVersion);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "DATA VERSION: %d\n", dataVersion);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "DATE: %s\n", date);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "START TIME: %s\n", startTime);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "STOP TIME: %s\n", stopTime);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "ANTENNA: %d MHz\n", (int)frequency);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "ANTENNA SEPARATION: %f\n", separation);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "SAMPLES: %d\n", samples);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "SIGNAL POSITION: 12\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "CLIPPED SAMPLES: 0\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "RUNS: %d\n", runs);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "MAX STACKS: %d\n", maxStacks);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "AUTOSTACKS: 1\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "FREQUENCY: %d\n", (int)frequency*20);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "TIMEWINDOW: %d\n", (int)timeWindow);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "LAST TRACE: %d\n", lastTrace);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "TRIG SOURCE: %s\n", trigSource);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "TIME INTERVAL: %f\n", intervalTime);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "DISTANCE INTERVAL: %f\n", intervalDist);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "USER DISTANCE INTERVAL: %f\n", intervalDist);  fwrite(s, 1, strlen(s), m_pFile);

	sprintf(s, "STOP POSITION: %f\n", 1.0);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "WHEEL NAME: WENDE\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "WHEEL CALIBRATION: 1.0\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "ZERO LEVEL: %d\n", zeroLevel);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "SOIL VELOCITY: %f\n", soilVel);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "PREPROCESSING: 0\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "OPERATOR COMMENT: _\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "ANTENNA F/W: 48001262\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "ANTENNA H/W: 0\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "ANTENNA FPGA: WENDE\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "ANTENNA SERIAL: 001\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "SOFTWARE VERSION: 1.0.0\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "POSITIONING: %d\n", positioning);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "CHANNELS: %d\n", channels);  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "CHANNEL CONFIGURATION: T2 - R2\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "CH_X_OFFSET: 0.000000\n");  fwrite(s, 1, strlen(s), m_pFile);
	sprintf(s, "CH_Y_OFFSET: 0.000000\n");  fwrite(s, 1, strlen(s), m_pFile);
	fclose(m_pFile);

	return 0;
}

//======================================���溯�����������ã�����Stream_DP�״�========================================//
/*
 * Fun:��Data��ʽ��IDS(8λ)�ļ�ת��Ϊiprh��iprb-------------�������ã�����Stream_DP�״�
 * Param: pathSrc�ļ����Ŀ¼
 *        pathDst iprb�ļ����Ŀ¼
 *        swathFile�ļ����� ... ... ����:Swath001_Array01.data
 *        swathName������   ... ... ����:Swath001
 *        sampleÿ��trace����������
 * ����:�ɹ�����true��ʧ�ܷ���false
 * Example:Swath001_Array01.data -->Swath001_Array01_A01.iprb��Swath001_A02.iprb ... ... Swath001_A10.iprb
 */
bool TransformIDS::transfer08First(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
{
	//�����Ϸ����ж�
	if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
		return false;
	if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
		return false;

	//200M hz��ʱ�䴰��60ns
	timeWindow = 60;

	//trace����������
	lastTrace = 0;
	//�����ids�ļ�
	char pathIDSFile[1024] = { 0 };
	sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
	FILE *fpSrc = fopen(pathIDSFile, "rb");
	if (!fpSrc)
	{
		printf("ԭ�ļ�(%s)������", pathIDSFile);
		return false;
	}

	char pathDstFile[1024] = { 0 };
	FILE *fpDst[20] = { NULL };
	for (int index = 1; index <= 19; index++)
	{
		//��������ļ�
		sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
		fpDst[index] = fopen(pathDstFile, "wb");
	}

	samples = 320;
	int traceSize = samples * 2;
	char *szBuff = (char *)malloc(traceSize + 256);

	//����36�ֽ�ͷ��Ϣ
	fread(szBuff, 1, 36, fpSrc);

	while (!feof(fpSrc))
	{
		int buffSize = 0;         //��¼��ȡ/д��ÿ��Traceʱ�Լ���ȡ�����ֽ�����
		char *tmpStart = szBuff;  //��¼��ȡ/д��ÿ��Traceʱ�Ŀ�ͷλ��

		//��һ��ͨ��
		{
			buffSize = 0;
			//��ȡ��һ��ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д���һ��ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[1]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//�ڶ���ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ�ڶ���ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//д��ڶ���ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[2]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//������ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ԭʼ�ļ���ȡһ��Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д�������ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[3]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//���ĸ�ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ���ĸ�ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//д����ĸ�ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[4]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

		}

		//�����ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ�����ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д������ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[5]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//������ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ������ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//д�������ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[6]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//���߸�ͨ��
		{
			tmpStart = szBuff;
			//��ȡ���߸�ͨ��������
			buffSize = 0;
			//��ԭʼ�ļ���ȡһ��Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д����߸�ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[7]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//�ڰ˸�ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ�ڰ˸�ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}

			//д��ڰ˸�ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[8]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//�ھŸ�ͨ��
		{
			tmpStart = szBuff;
			//��ȡ�ھŸ�ͨ��������
			buffSize = 0;
			//��ԭʼ�ļ���ȡһ��Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д��ھŸ�ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[9]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//��ʮ��ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ��ʮ��ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//д���ʮ��ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[10]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//��ʮһ��ͨ��
		{
			tmpStart = szBuff;
			//��ȡ��ʮһ��ͨ��������
			buffSize = 0;
			//��ԭʼ�ļ���ȡһ��Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д���ʮһ��ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[11]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//ʮ����ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ��ʮ����ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//д���ʮ����ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[12]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//ʮ����ͨ��
		{
			tmpStart = szBuff;
			//��ȡ��ʮ����ͨ��������
			buffSize = 0;
			//��ԭʼ�ļ���ȡһ��Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д���ʮ����ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[13]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//ʮ�ĸ�ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ��ʮ�ĸ�ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//д���ʮ�ĸ�ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[14]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

		}

		//ʮ���ͨ��
		{
			tmpStart = szBuff;
			//��ȡ��ʮ���ͨ��������
			buffSize = 0;
			//��ԭʼ�ļ���ȡһ��Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д���ʮ���ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[15]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//ʮ����ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ��ʮ����ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//д���ʮ����ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[16]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//ʮ�߸�ͨ��
		{
			tmpStart = szBuff;
			//��ȡ��ʮ�߸�ͨ��������
			buffSize = 0;
			//��ԭʼ�ļ���ȡһ��Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д���ʮ�߸�ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[17]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//ʮ�˸�ͨ��
		{
			tmpStart = szBuff;
			buffSize = 0;
			//��ȡ��ʮ�˸�ͨ��������
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}

			//д���ʮ�˸�ͨ��������
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[18]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//ʮ�Ÿ�ͨ��
		{
			tmpStart = szBuff;
			//��ȡ��ʮ�Ÿ�ͨ��������
			buffSize = 0;
			//��ԭʼ�ļ���ȡһ��Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//д���ʮ�Ÿ�ͨ��������
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[19]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//������������
		lastTrace++;
	}

	free(szBuff);

	//�ر����������ļ�����������ļ�(iprb)
	fclose(fpSrc);
	for (int index = 1; index <= 19; index++)
	{
		fclose(fpDst[index]);
	}

	//��¼��Trace����
	lastTrace--;

	//д��iprhͷ�ļ�
	for (int index = 1; index <= 19; index++)
	{
		//д��iprh
		saveHeader(pathDst, swathName, index);
	}

	return true;
}

/*
 * Fun:��Data��ʽ��IDS(8λ)�ļ�ת��Ϊiprh��iprb-------------�������ã�����Stream_DP�״�
 * Param: pathSrc�ļ����Ŀ¼
 *        pathDst iprb�ļ����Ŀ¼
 *        swathFile�ļ����� ... ... ����:Swath001_Array02.data
 *        swathName������   ... ... ����:Swath002
 *        sampleÿ��trace����������
 * ����:�ɹ�����true��ʧ�ܷ���false
 * Example:Swath001_Array02.data -->Swath001_Array02_A01.iprb��Swath001_A11.iprb ... ... Swath001_A29.iprb
 */
bool TransformIDS::transfer08Second(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
{
	//�����Ϸ����ж�
	if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
		return false;
	if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
		return false;

	//600M hz��ʱ�䴰��80ns
	timeWindow = 80;

	//trace����������
	lastTrace = 0;
	//�����ids�ļ�
	char pathIDSFile[1024] = { 0 };
	sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
	FILE *fpSrc = fopen(pathIDSFile, "rb");
	if (!fpSrc)
	{
		printf("ԭ�ļ�(%s)������", pathIDSFile);
		return false;
	}

	char pathDstFile[1024] = { 0 };
	FILE *fpDst[31] = { NULL };
	for (int index = 20; index <= 30; index++)
	{
		//��������ļ�
		sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
		fpDst[index] = fopen(pathDstFile, "wb");
	}

	samples = 320;

	int traceSize = samples * 2;
	char *szBuff = (char *)malloc(traceSize);

	//����36�ֽ�ͷ��Ϣ
	fread(szBuff, 1, 36, fpSrc);

	while (!feof(fpSrc))
	{
		int buffSize = 0;         //��¼��ȡ/д��ÿ��Traceʱ�Լ���ȡ�����ֽ�����
		char *tmpStart = szBuff;  //��¼��ȡ/д��ÿ��Traceʱ�Ŀ�ͷλ��

		buffSize = 0;
		//��ȡ��11��ͨ��������
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���11��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[20]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��12��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���12��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[21]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��13��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���13��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[22]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��14��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���14��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[23]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��15��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���15��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[24]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��16��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���16��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[25]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��17��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���17��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[26]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��18��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���18��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[27]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��19��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���19��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[28]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��20��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���20��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[29]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��21��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���21��ͨ��������
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[30]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//������������
		lastTrace++;
	}

	free(szBuff);

	//�ر�
	fclose(fpSrc);
	for (int index = 20; index <= 30; index++)
	{
		fclose(fpDst[index]);
	}

	lastTrace--;

	//д��iprhͷ�ļ�
	for (int index = 20; index <= 30; index++)
	{
		//д��iprh
		saveHeader(pathDst, swathName, index);
	}

	return true;
}

/*
 * Fun:ת��data��չ��������Ϊiprh/iprb---Ӧ����IDS8λ�״�-------------�������ã�����Stream_DP�״�
 * Param: pathSrc�ļ����Ŀ¼
 *        pathDst iprb�ļ����Ŀ¼
 *        swathName ������
 *        swathID   ����ID
 * ����:�ɹ�����true��ʧ�ܷ���false
 */
bool TransformIDS::transferData08(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
{
	char swathFile[128] = { 0 };
	//�����״�Ŀ¼�£�ָ���ļ�(��һ��IDS�����ļ�)
	sprintf(swathFile, "%s_Array01.data", swathName);
	//�����һ��data�ļ�ת��Ϊiprb�ļ�
	if (false == transfer08First(pathSrc, pathDst, swathFile, swathName))
	{
		printf("ת����һ�������ļ�����\n");
		return false;
	}

	/* --------------------Array01��Array02������--------------------*/
	//�����״�Ŀ¼�£�ָ���ļ�(�ڶ���IDS�����ļ�)
	sprintf(swathFile, "%s_Array02.data", swathName);
	//�����һ��data�ļ�ת��Ϊiprb�ļ�
	if (false == transfer08Second(pathSrc, pathDst, swathFile, swathName))
	{
		printf("ת���ڶ��������ļ�����\n");
		return false;
	}

	/* --------------------����cor�ļ�--------------------*/
	//����cor�ļ�
	if (false == makeFileCor(pathDst, swathName, swathID))
	{
		printf("����cor�ļ�����\n");
		return false;
	}

	/* --------------------����time�ļ�-------------------*/
	if (false == makeFileTime(pathSrc, pathDst, swathName, swathID))
	{
		printf("����time�ļ�����\n");
		return false;
	}

	return true;
}

/*
* Fun����ָ��Ŀ¼�µ�ids����ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��---Ӧ����IDS8λ�״�-------------�������ã�����Stream_DP�״�
* Param��
*      pathIDS     IDS�����ļ���Swath001_Array01.data��
*      pathDst     IPRH��IPRDĿ��Ŀ¼
* Return���ɹ�����TRUE��ʧ�ܷ���FALSE
*/
int TransformIDS::transformIDS08(const char * pathIDS, const char * pathDst)
{
	int swathID = 1;
	char swathName[128] = { 0 };
	//�����״�Ŀ¼�£�ָ���ļ�(��һ��IDS�����ļ�)
	char szPathTmp[512] = { 0 };
	sprintf(szPathTmp, "%s\\*.swath", pathIDS);
	intptr_t hFile = 0;
	struct _finddata_t oFileInfo;

	//��ѯ�Ƿ���ڵ�һ������
	if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
	{
		printf("û�з���IDS�Ĳ������ݣ�*.swath��\n");
		return -1;
	}


	do
	{
		//��ȡ��һ���ļ����ļ���
		sprintf(szPathTmp, "%s\\%s", pathIDS, oFileInfo.name);

		//�õ�������
		strncpy(swathName, oFileInfo.name, strlen(oFileInfo.name) - 6);
		//MessageBox(NULL, swathName, swathName, 0);
		//��IDS data�ļ�ת��Ϊiprh��iprb
		bool result = transferData08(pathIDS, pathDst, swathName, swathID);
		if (true != result)
		{
			//�ر��ļ�����
			_findclose(hFile);
			return -2;
		}

		swathID++;
		//} while ((hFile = (intptr_t)_findnext(hFile, &oFileInfo)) != -1L);
	} while (_findnext(hFile, &oFileInfo) == 0);

	//�ر��ļ�����
	if (0 != hFile)
		_findclose(hFile);

	return 0;
}
//======================================���溯�����������ã�����Stream_DP�״�========================================//

/*
 * Fun:��Data��ʽ��IDS(16λ)�ļ�ת��Ϊiprh��iprb
 * Param: pathSrc�ļ����Ŀ¼
 *        pathDst iprb�ļ����Ŀ¼
 *        swathFile�ļ����� ... ... ����:Swath001_Array01.data
 *        swathName������   ... ... ����:Swath001
 *        sampleÿ��trace����������
 * ����:�ɹ�����true��ʧ�ܷ���false
 * Example:Swath001_Array01.data -->Swath001_Array01_A01.iprb��Swath001_A02.iprb ... ... Swath001_A10.iprb
 */
bool TransformIDS::transferFirst(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
{
	//�����Ϸ����ж�
	if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
		return false;
	if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
		return false;

	//200M hz��ʱ�䴰��60ns
	timeWindow = 60;

	//trace����������
	lastTrace = 0;
	//�����ids�ļ�
	char pathIDSFile[1024] = { 0 };
	sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
	FILE *fpSrc = fopen(pathIDSFile, "rb");
	if (!fpSrc)
	{
		printf("ԭ�ļ�(%s)������", pathIDSFile);
		return false;
	}

	char pathDstFile[1024] = { 0 };
	FILE *fpDst[11] = { NULL };
	for (int index = 1; index <= 10; index++)
	{
		//��������ļ�
		sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
		fpDst[index] = fopen(pathDstFile, "wb");
	}

	int traceSize = samples * 2;
	char *szBuff = (char *)malloc(traceSize);

	//����36�ֽ�ͷ��Ϣ
	fread(szBuff, 1, 36, fpSrc);

	while (!feof(fpSrc))
	{
		int buffSize = 0;         //��¼��ȡ/д��ÿ��Traceʱ�Լ���ȡ�����ֽ�����
		char *tmpStart = szBuff;  //��¼��ȡ/д��ÿ��Traceʱ�Ŀ�ͷλ��

		buffSize = 0;
		//��ȡ��һ��ͨ��������
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д���һ��ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[1]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ�ڶ���ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д��ڶ���ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[2]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ������ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д�������ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[3]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ���ĸ�ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д����ĸ�ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[4]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ�����ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д������ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[5]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ������ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д�������ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[6]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ���߸�ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д����߸�ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[7]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ�ڰ˸�ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д��ڰ˸�ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[8]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ�ھŸ�ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д��ھŸ�ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[9]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��ʮ��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//д���ʮ��ͨ��������
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[10]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//������������
		lastTrace++;
	}

	free(szBuff);

	//�ر����������ļ�����������ļ�(iprb)
	fclose(fpSrc);
	for (int index = 1; index <= 10; index++)
	{
		fclose(fpDst[index]);
	}

	//��¼��Trace����
	lastTrace--;

	//д��iprhͷ�ļ�
	for (int index = 1; index <= 10; index++)
	{
		//д��iprh
		saveHeader(pathDst, swathName, index);
	}

	return true;
}

/*
 * Fun:��Data��ʽ��IDS(16λ)�ļ�ת��Ϊiprh��iprb
 * Param: pathSrc�ļ����Ŀ¼
 *        pathDst iprb�ļ����Ŀ¼
 *        swathFile�ļ����� ... ... ����:Swath001_Array02.data
 *        swathName������   ... ... ����:Swath002
 *        sampleÿ��trace����������
 * ����:�ɹ�����true��ʧ�ܷ���false
 * Example:Swath001_Array02.data -->Swath001_Array02_A01.iprb��Swath001_A11.iprb ... ... Swath001_A29.iprb
 */
bool TransformIDS::transferSecond(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
{
	//�����Ϸ����ж�
	if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
		return false;
	if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
		return false;

	//600M hz��ʱ�䴰��80ns
	timeWindow = 80;

	//trace����������
	lastTrace = 0;
	//�����ids�ļ�
	char pathIDSFile[1024] = { 0 };
	sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
	FILE *fpSrc = fopen(pathIDSFile, "rb");
	if (!fpSrc)
	{
		printf("ԭ�ļ�(%s)������", pathIDSFile);
		return false;
	}

	char pathDstFile[1024] = { 0 };
	FILE *fpDst[30] = { NULL };
	for (int index = 11; index <= 29; index++)
	{
		//��������ļ�
		sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
		fpDst[index] = fopen(pathDstFile, "wb");
	}

	int traceSize = samples * 2;
	char *szBuff = (char *)malloc(traceSize);

	//����36�ֽ�ͷ��Ϣ
	fread(szBuff, 1, 36, fpSrc);

	while (!feof(fpSrc))
	{
		int buffSizeOld = 0;
		int buffSize = 0;         //��¼��ȡ/д��ÿ��Traceʱ�Լ���ȡ�����ֽ�����
		char *tmpStart = szBuff;  //��¼��ȡ/д��ÿ��Traceʱ�Ŀ�ͷλ��

		buffSize = 0;
		//��ȡ��11��ͨ��������
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���11��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[11]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[11]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��12��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���12��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[12]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[12]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��13��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���13��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[13]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[13]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��14��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���14��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[14]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[14]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��15��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���15��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[15]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[15]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��16��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���16��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[16]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[16]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��17��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���17��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[17]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[17]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��18��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���18��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[18]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[18]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��19��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���19��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[19]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[19]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//��ȡ��20��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���20��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[20]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[20]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��21��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���21��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[21]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[21]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��22��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���22��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[22]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[22]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��23��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���23��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[23]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[23]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��24��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���24��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[24]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[24]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��25��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���25��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[25]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[25]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��26��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���26��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[26]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[26]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��27��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���27��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[27]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[27]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��28��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���28��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[28]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[28]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//��ȡ��29��ͨ��������
		buffSize = 0;
		//��ԭʼ�ļ���ȡһ��Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//д���29��ͨ��������
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[29]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//�ٴ�д��һ�Σ���Ϊ11-29ͨ����������1-10��һ��
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[29]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//������������
		lastTrace++;
	}

	free(szBuff);

	//�ر�
	fclose(fpSrc);
	for (int index = 11; index <= 29; index++)
	{
		fclose(fpDst[index]);
	}

	//��¼��Trace����
	lastTrace--;
	lastTrace = lastTrace * 2;

	//д��iprhͷ�ļ�
	for (int index = 11; index <= 29; index++)
	{
		//д��iprh
		saveHeader(pathDst, swathName, index);
	}

	return true;
}


/*
 * Fun:ת��data��չ����Swath001_Array01.data��Swath001_Array01.index��Swath001.swath��Survey��������Ϊiprh/iprb��ת��һ��IDS����Ϊiprh/iprb
 * Param: pathSrc�ļ����Ŀ¼
 *        pathDst iprb�ļ����Ŀ¼
 *        swathName ������
 *        swathID   ����ID
 * ����:�ɹ�����true��ʧ�ܷ���false
 */
bool TransformIDS::transferData16(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
{
	char swathFile[128] = { 0 };
	//Survey�ļ�������·��
	sprintf(swathFile, "%s/Survey", pathSrc);
	//���״�Ŀ¼�£���ȡָ���ļ�(Survey)�����ҽ���Ϣд���Ա����ͷ
	getHeaderInfo(swathFile);

	/* ---------------------Survey��Array01������--------------------*/
	//�����״�Ŀ¼�£�ָ���ļ�(��һ��IDS�����ļ�)
	sprintf(swathFile, "%s_Array01.data", swathName);
	//�����һ��data�ļ�ת��Ϊiprb�ļ�
	if (false == transferFirst(pathSrc, pathDst, swathFile, swathName))
	{
		printf("ת����һ�������ļ�����\n");
		return false;
	}

	/* --------------------Array01��Array02������--------------------*/
	//�����״�Ŀ¼�£�ָ���ļ�(�ڶ���IDS�����ļ�)
	sprintf(swathFile, "%s_Array02.data", swathName);
	//�����һ��data�ļ�ת��Ϊiprb�ļ�
	if (false == transferSecond(pathSrc, pathDst, swathFile, swathName))
	{
		printf("ת���ڶ��������ļ�����\n");
		return false;
	}

	/* --------------------����cor�ļ�--------------------*/
	//����cor�ļ�
	if (false == makeFileCor(pathDst, swathName, swathID))
	{
		printf("����cor�ļ�����\n");
		return false;
	}

	/* --------------------����time�ļ�-------------------*/
	if (false == makeFileTime(pathSrc, pathDst, swathName, swathID))
	{
		printf("����time�ļ�����\n");
		return false;
	}

	return true;
}

/*
 * Fun����ָ��Ŀ¼�µ�ids���ݣ�Swath001_Array01.data��Swath001_Array01.index��Swath001.swath��Survey��ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��
 * Param��
 *      pathIDS     IDS�����ļ���Swath001_Array01.data��
 *      pathDst     IPRH��IPRDĿ��Ŀ¼
 * Return���ɹ�����TRUE��ʧ�ܷ���FALSE
 */
int TransformIDS::transformIDS16(const char * pathIDS, const char * pathDst)
{
	int swathID = 1;
	char swathName[128] = { 0 };
	//�����״�Ŀ¼�£�ָ���ļ�(��һ��IDS�����ļ�)
	char szPathTmp[512] = { 0 };
	sprintf(szPathTmp, "%s\\*.swath", pathIDS);
	intptr_t hFile = 0;
	struct _finddata_t oFileInfo;

	//��ѯ�Ƿ���ڵ�һ������
	if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
	{
		printf("û�з���IDS�Ĳ������ݣ�*.swath��\n");
		return -1;
	}


	do
	{
		//��ȡ��һ���ļ����ļ���
		sprintf(szPathTmp, "%s\\%s", pathIDS, oFileInfo.name);

		//�õ�������
		strncpy(swathName, oFileInfo.name, strlen(oFileInfo.name) - 6);

		//��IDS data�ļ�ת��Ϊiprh��iprb
		bool result = transferData16(pathIDS, pathDst, swathName, swathID);
		if (true != result)
		{
			//�ر��ļ�����
			_findclose(hFile);
			return -2;
		}

		swathID++;
	} while (_findnext(hFile, &oFileInfo) == 0);

	//�ر��ļ�����
	if (0 != hFile)
		_findclose(hFile);

	return 0;
}





/*
* Fun����ָ��Ŀ¼�µ�ids���ݣ�*.scan��ת��ΪIPRH��IPRD���洢��ָ��Ŀ¼��
* Param��
*      pathIDS     IDS����Ŀ¼
*      pathDst     IPRH��IPRDĿ��Ŀ¼
*      separation  ͨ�����
* Return���ɹ�����TRUE��ʧ�ܷ���FALSE
*/
int TransformIDS::transformIDS(const char * pathIDS, const char * pathDst, float separation)
{
	//����ID
	int swathID = 1;
	IDSSwath idsSwath;
	idsSwath.setID(swathID);    //��һ������ID�̶�Ϊ1


	//�����״�Ŀ¼�£�ָ��xml�ļ�,��ȡͷ��Ϣ
	char szPathXML[512] = { 0 };
	sprintf(szPathXML, "%s\\*.xml", pathIDS );
	intptr_t hFileXML;
	struct _finddata_t oFileInfoXML;
	if ((hFileXML = (intptr_t)_findfirst(szPathXML, &oFileInfoXML)) != -1L)
	{
		char szPathTmp[512] = { 0 };
		//��ȡ��һ���ļ����ļ���
		sprintf(szPathTmp, "%s\\%s", pathIDS, oFileInfoXML.name);

		//�ҵ�XML������£���ȡͷ��Ϣ
		getHeaderInfo(szPathTmp);

		_findclose(hFileXML);
	}
	else
	{
		return -1;
	}

	int i = 1;
	while (true)
	{
		//�����״�Ŀ¼�£�ָ���ļ�,IDS����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathTmp[512] = { 0 };
		sprintf(szPathTmp, "%s\\RAW_%06d.scan", pathIDS, i);
		intptr_t hFile;
		struct _finddata_t oFileInfo;
		if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
		{
			//ȡ�ò��ߵ���������
			std::map<long, IDSChannel*> * lstData = idsSwath.getData();

			//����cor�ļ�
			CreateCor(pathDst, "Swath", swathID);

			//����time�ļ�
			CreateTime(pathDst, "Swath", swathID);

			//��idsSwath�е�IDS����ת��ΪIPRH��IPRB����
			IDSTransferData(lstData, swathID, (char *)pathDst, separation);

			//���ߺ�����һ
			swathID++;
			//����Ѿ���ɵ�����
			idsSwath.swathDataClear();
			idsSwath.setID(swathID);

			_findclose(hFile);
			printf("û�з���IDS�Ĳ�������\n");
			break;
		}

		//ʹ��ids�ļ���ʼ��ids����Ƭ��
		IDSSwathFragment idsSwathFragment;
		int iResult = idsSwathFragment.init(szPathTmp);
		if (0 != iResult)
		{
			_findclose(hFile);
			printf("����IDS�Ĳ������ݳ���\n");
			break;
		}

		//�ļ�û������
		if (idsSwathFragment.getID() == 0)
		{
			_findclose(hFile);
			printf("����ID����\n");
			break;
		}

		//�������Ƭ���е�trace����Ϊ0������Ҫ����
		if (0 == idsSwathFragment.getTraceCount())
		{
			_findclose(hFile);
			printf("�ļ��е�trace����Ϊ0\n");
			i++;
			continue;
		}

		//������µĲ��ߺ��ϴεĲ���ID��һ����˵���ǲ�ͬ�Ĳ��ߣ���Ҫ���ϴ������Ĳ�������ת��Ϊiprh��iprd��ʽ
		if (idsSwathFragment.getID() != swathID)
		{
			//ȡ�ò��ߵ���������
			std::map<long, IDSChannel*> * lstData = idsSwath.getData();

			//����cor�ļ�
			CreateCor(pathDst, "Swath", swathID);

			//����time�ļ�
			CreateTime(pathDst, "Swath", swathID);

			//��idsSwath�е�IDS����ת��ΪIPRH��IPRB����
			IDSTransferData(lstData, swathID, (char *)pathDst, separation);

			//���ߺ�����һ
			swathID++;
			//����Ѿ���ɵ�����
			idsSwath.swathDataClear();
			idsSwath.setID(swathID);
		}
		//ȡ��������������
		std::map<long, IDSChannel*> * swathData = idsSwathFragment.getDataList();

		//������ߺź��ϴ�һ�£���ôͳһ��ӵ�������
		idsSwath.swathDataAdd(swathData);

		//�ر�������
		_findclose(hFile);
		i++;
	}

	return 0;
}

//��һ��IDS��ԭʼ����(��������)ת��ΪIPRH��IPRB
int TransformIDS::IDSTransferData(std::map<long, IDSChannel*> * lstData, int swathID, char *swathPathDst, float separation )
{
	if (NULL == lstData)
	{
		//��������
		return -1;
	}
	if (0 == lstData->size())
	{
		//�����б��У�û������
		return 0;
	}

	//��������ͨ������
	for (std::map<long, IDSChannel*>::iterator iter = lstData->begin(); iter != lstData->end(); iter++)
	{
		IDSChannel* p = iter->second;
		if (!p)
			continue;

		//��ȡƵ�ʺ͵����
		frequency    = channelFreq[iter->first];
		intervalDist = traceStep[iter->first];
		samples      = channelSample[iter->first];

		//ͨ������
		IDSChannelHeader *header = p->getHeader();
		IDSChannelBlob   *blob = p->getBlob();

		//�����ݽ��зǿ��жϣ���һЩ������
		if ((!header) || (!blob))
			continue;

		//ȡ��trace����
		int lastTrace = header->GetTraceCount();

		//�洢��Ŀ���ļ�
		char szSwathFileHeader[512] = { 0 };
		char szSwathFileBlob[512] = { 0 };
		sprintf(szSwathFileHeader, "%s\\Swath_%04d_A%02d.iprh", swathPathDst, swathID, p->getID());
		sprintf(szSwathFileBlob, "%s\\Swath_%04d_A%02d.iprb", swathPathDst, swathID, p->getID());

		//��Ŀ���ļ�
		FILE *fpHeader = fopen(szSwathFileHeader, "w+");
		FILE *fpBlob = fopen(szSwathFileBlob, "wb+");

		//ת�浽Ŀ���ļ�---Header�ļ�
		char szBuff[64] = { 0 };
		sprintf(szBuff, "HEADER VERSION: 10\n");            	fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "DATA VERSION: 16\n");                 	fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "DATE: 2022-11-10\n");                 	fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "START TIME: 12:08:46\n");              fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "STOP TIME: 12:09:48\n");               fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "ANTENNA: %d MHz\n", (int)frequency);   fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "ANTENNA SEPARATION: %f\n", separation); fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "SAMPLES: %d\n", samples);              fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "SIGNAL POSITION: 12\n");               fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "CLIPPED SAMPLES: 0\n");                fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "RUNS: 32\n");                          fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "MAX STACKS: 512\n");                   fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "AUTOSTACKS: 1\n");                     fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "FREQUENCY: %d\n", (int)frequency*20);  fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "TIMEWINDOW: %d\n", 80);                fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "LAST TRACE: %d\n", lastTrace);         fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "TRIG SOURCE: wheel\n");                fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "TIME INTERVAL: 0.050000\n");           fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "DISTANCE INTERVAL: %f\n", intervalDist);       fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "USER DISTANCE INTERVAL: %f\n", intervalDist);  fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "STOP POSITION: 368.635412\n");         fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "WHEEL NAME: New_Wheel123\n");          fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "WHEEL CALIBRATION:477.8000000000\n");  fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "ZERO LEVEL: 40\n");                    fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "SOIL VELOCITY: 100.000000\n");         fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "PREPROCESSING: 0\n");                  fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "OPERATOR COMMENT: _\n");               fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "ANTENNA F/W: 48001262\n");             fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "ANTENNA H/W: 0\n");                    fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "ANTENNA FPGA: DA74\n");                fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "ANTENNA SERIAL: 1579\n");              fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "SOFTWARE VERSION: T 1.2.20\n");        fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "POSITIONING: 2\n");                    fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "CHANNELS: 15\n");                      fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "CHANNEL CONFIGURATION: T2 - R2\n");    fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "CH_X_OFFSET: 0.000000\n");             fwrite(szBuff, 1, strlen(szBuff), fpHeader);
		sprintf(szBuff, "CH_Y_OFFSET: 0.000000\n");             fwrite(szBuff, 1, strlen(szBuff), fpHeader);

		//ת�浽Ŀ���ļ�---Blob�ļ�����ȡTraceList
		std::map<long, IDSTrace16*> *traceData = blob->getTraceList();
		for (std::map<long, IDSTrace16*>::iterator iter_Trace = traceData->begin(); iter_Trace != traceData->end(); iter_Trace++)
		{
			IDSTrace16* traceBlob = iter_Trace->second;

			fwrite(traceBlob->getTrace16(), 1, 1024, fpBlob);
		}

		fclose(fpBlob);
		fclose(fpHeader);
	}

	return 0;
}

//��Survey�л�ȡͷ��Ϣ
bool TransformIDS::getHeaderInfo(const char * fileSurvey)
{
	if (!fileSurvey)
		return false;

	FILE *fp = fopen(fileSurvey, "r");

	char *buff = (char *)malloc( 1024*30 );
	char *tmp = buff;
	//��ȡ�ļ�������Ϣ
	while ( true )
	{
		int iLen = (int)fread(tmp, 1, 20, fp);
		if (iLen <= 0)
			break;

		tmp = tmp + iLen;
	}

	int channID = 0;
	//��ͷ��ʼ����Channel��EncoderStack
	tmp = buff;
	while ( tmp )
	{
		//1����ȡChannelID
		char *tmp1 = strstr(tmp, "<Channel Id=\"");
		if (!tmp1)
			break;
		tmp1 = tmp1 + 13;
		tmp = tmp1;
		char *tmp2 = strstr(tmp1, "\"");

		char szChannelID[8] = { 0 };
		strncpy(szChannelID, tmp1, tmp2 - tmp1);
		channID = atoi(szChannelID);

		//2����ȡDadID
		tmp1 = strstr(tmp, "DadId=\"");
		if (!tmp1)
			break;
		tmp2 = strstr(tmp1 + 8, "\"");
		char szDadID[32] = { 0 };
		strncpy(szDadID, tmp1, tmp2 - tmp1 + 1);

		//3��ͨ��DadID����ȡ��������
		char *antennaType = strstr(buff, szDadID);
		while ('=' != *antennaType)
		{
			antennaType--;
		}
		char szAntennaType[32] = { 0 };
		strncpy(szAntennaType, antennaType+2, 7);

		//�����������ͣ���������Ƶ��
		tmp1 = strstr(buff, "<AntennaTypes>");
		tmp1 = strstr(tmp1, szAntennaType);
		tmp1 = strstr(tmp1, "Frequency");
		tmp1 = tmp1 + 11;
		tmp2 = strstr(tmp1, "\"");
		while ('"' != *tmp1)
		{
			channelFreq[channID] = channelFreq[channID] * 10 + (*tmp1 - 0x30);
			tmp1++;
		}

		//========��ȡEncoderStack Begin=======//
		char szEncoderStack[8] = { 0 };
		tmp1 = strstr(buff, "<EncoderStack value=");
		//��������ڿͻ�ָ����EncoderStack����ô��ȡͨ���Լ���EncoderStack
		if (!tmp1)
		{
			//��ȡEncoderStack
			tmp1 = strstr(tmp, "EncoderStack=\"");
			if (!tmp1)
				break;
			tmp1 = tmp1 + 14;
		}
		else
		{
			tmp1 = tmp1 + 21;
		}
		tmp2 = strstr(tmp1, "\"");
		strncpy(szEncoderStack, tmp1, tmp2 - tmp1);
		traceStep[channID] = (float)atoi(szEncoderStack);
		//========��ȡEncoderStack  End=======//

		//��ȡSample
		char szSample[8] = { 0 }; memset(szSample, 0, 8);
		tmp = strstr(tmp, "Samples=\"");
		tmp2 = strstr(tmp + 9, "\"");
		int length = (int)(tmp2 - tmp - 9);
		strncpy(szSample, tmp + 9, length);
		channelSample[channID] = atoi(szSample);
	}

	tmp = buff;
	char *tmp1 = strstr(tmp, "XStep=\"");	tmp1 = tmp1 + 7;
	char *tmp2 = strstr(tmp1, "\"");
	*tmp2 = 0;
	//��դ����    //��դ�����¼����Ա���������ں���ת�����iprh�ļ�
	separation = (float)atof(tmp1);

	//����Custom�����XStep��������ڣ���ʹ��Custom����
	tmp2++;
	tmp1 = strstr(tmp2, "<XStep value=\"");
	if (tmp1)
	{
		tmp1 = tmp1 + 14;
		tmp2 = strstr(tmp1, "\"");
		*tmp2 = 0;
		//��դ����    //��դ�����¼����Ա���������ں���ת�����iprh�ļ�
		separation = (float)atof(tmp1);
	}
	for (; channID >= 1; channID--)
	{
		traceStep[channID] = (float)(traceStep[channID] * separation);
	}

	return true;
}

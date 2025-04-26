/*
 * Fun:�������ҵ��㷨���ͼ�����ݽ��д���
 */

#include "framework.h"
#include <io.h>
//#include <atlimage.h>

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

#include "Transform\\TransformSegy.h"
#include "Transform\\Transform3DRadar.h"
#include "Transform\\TransformMala.h"
#include "Transform\\TransformIDS.h"
#include "Transform\\TransformDT.h"

#include "Project.h"
#include "RadarDll.h"


//ȫ�ֶ���
extern CRadarDllApp theApp;

char szProjectPathC[PATH_MAX_LENGTH] = { 0 };    //����Ŀ¼
Project gProjectC;      //ȫ�ֵĹ��̶���

extern "C" __declspec(dllexport) int getVersion(const char *strPwd, char *strVersion)
{
	//�����Ϸ���У��
	if (!strPwd)
		return ERROR_PARAM;
	if (!strVersion)
		return ERROR_PARAM;
	//�����������
	int length = (int)wcslen((wchar_t*)strPwd);
	if (length <= 0)
		return ERROR_PARAM;
	if (length > 64)
		length = 64;

	char szPwd[65] = { 0 };
	char szVersion[65] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)strPwd, length, szPwd, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	if (strcmp(szPwd, "My name is alpha") == 0)
		strcpy(szVersion, "Make by:Wende on 2024-04-20");
	else
		strcpy(szVersion, "Make by:zgz&wjz&cf");

	wchar_t* buffer = (wchar_t*)malloc(128);
	memset((char *)buffer, 0, 128);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szVersion, (int)strlen(szVersion) * 2 + 2, buffer, (int)strlen(szVersion) * 2 + 2);
	wcscpy((wchar_t *)strVersion, buffer);
	free(buffer);

	return ERROR_CODE_SUCCESS;
}

//�����״����ݵĴ洢Ŀ¼��ͬʱ��ʼ���״﹤�̣����������ݵĲ��������ڸù���
extern "C" __declspec(dllexport) int setProjectPath(const char *strpath)
{
	int length = (int)wcslen((wchar_t*)strpath);
	if (length <= 0)
		return ERROR_PARAM;

	memset(szProjectPathC, 0, PATH_MAX_LENGTH);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)strpath, length, szProjectPathC, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	return ERROR_CODE_SUCCESS;
}

//��ʼ������
extern "C" __declspec(dllexport) int initProject()
{
	if (0 >= strlen(szProjectPathC))
		return ERROR_CODE_PATH;

	//��ʼ�����̣�����cor�ļ���time�ļ������ݼ���
	int result = gProjectC.init(szProjectPathC);
	//�����һ�ַ�ʽ��ʼ��ʧ�ܣ�����õڶ��ַ�ʽ��ʼ��
	if (result)
	{
		//������mrk�ļ���ord�ļ������ݼ���
		result = gProjectC.initEx(szProjectPathC);
	}

	return result;
}

//���ʼ������
extern "C" __declspec(dllexport) int unInitProject()
{
	gProjectC.unInit();
	return ERROR_CODE_SUCCESS;
}

//ͨ��ʱ���ȡ��Ӧ��Trace��
extern "C" __declspec(dllexport) int getTraceNumByTime(const char *  swathName, char *traceTime)
{
	int length = (int)wcslen((wchar_t*)traceTime);
	if (length <= 0)
		return ERROR_CODE_OTHER;

	//TraceTime
	char szTraceTime[64] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)traceTime, length, szTraceTime, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_CODE_OTHER;

	if (strlen(szTraceTime) <= 0)
		return ERROR_CODE_OTHER;

	length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_CODE_OTHER;
	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_CODE_OTHER;

	//��ȡĿ�����
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_OTHER;

	SwathTime * swathTime = swath->getSwathTime();
	return swathTime->getTraceNumByTime(szTraceTime);
}

//ͨ��Trace�Ż�ȡ��Ӧ��ʱ��
extern "C" __declspec(dllexport) int getTraceTimeByNum(const char *  swathName, int traceNum, char *traceTime)
{
	if (!traceTime)
		return ERROR_PARAM;

	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_CODE_OTHER;
	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_CODE_OTHER;

	//��ȡĿ�����
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_OTHER;

	char szDateTime[64] = { 0 };
	SwathTime * swathTime = swath->getSwathTime();
	swathTime->getTraceTimeByNum(traceNum, szDateTime);

	wchar_t* buffer = (wchar_t*)malloc(128);
	memset((char *)buffer, 0, 128);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szDateTime, (int)strlen(szDateTime) * 2 + 2, buffer, (int)strlen(szDateTime) * 2 + 2);
	wcscpy((wchar_t *)traceTime, buffer);
	free(buffer);

	return 0;
}


//���ü����״����ݵĲ���--�Աȶȣ�����ͼ����ǰ��Ҫ������Ҫ�趨�ò���������ʹ��Ĭ�����ݻ��Ѿ��趨�Ĳ���
extern "C" __declspec(dllexport) int setParamContrast(BOOL autoEnable, int contrast)
{
	if (!autoEnable)
		//�ֶ�����һ���Աȶ�
		theApp.SetContrast(contrast);
	else
		theApp.SetContrast(20);
	return ERROR_CODE_SUCCESS;
}

//���ü����״����ݵĲ���--���棬����ͼ����ǰ��Ҫ������Ҫ�趨�ò���������ʹ��Ĭ�����ݻ��Ѿ��趨�Ĳ���
extern "C" __declspec(dllexport) int setParamGain(BOOL autoEnable, int gain)
{
	if (!autoEnable)
		//�ֶ���������
		theApp.SetGain(gain);
	else
		theApp.SetGain(10);

	return ERROR_CODE_SUCCESS;
}

//����ͼ�������ɫ
extern "C" __declspec(dllexport) int setColor(int iColor, int iMask)
{
	theApp.SetColor(iColor, iMask);
	return ERROR_CODE_SUCCESS;
}

//���ü����״����ݵĲ���--��ȣ�����ͼ����ǰ��Ҫ������Ҫ�趨�ò���������ʹ��Ĭ�����ݻ��Ѿ��趨�Ĳ���
extern "C" __declspec(dllexport) int setParamDeep(BOOL autoEnable, int deep[])
{
	if (!autoEnable)
	{
		int iDeep[32] = { 10 };   //����
		//�ֶ�����z������ƽ����
		for (int i = 0; i < 32; ++i)
		{
			iDeep[i] = deep[i];
		}

		theApp.SetDeep(iDeep);
	}
	else
	{
		int iDeep[32] = { 10 };   //����
		theApp.SetDeep(iDeep);
	}
	return ERROR_CODE_SUCCESS;
}

//��ȡ��������
extern "C" __declspec(dllexport) int getSwathCount()
{
	std::vector<char*> lstSwath;
	gProjectC.getAllSwathName(lstSwath);

	//ȡ�ò�������
	int iSize = (int)lstSwath.size();

	return iSize;
}

//ͨ�����ߵ���������ȡ�������ơ�����ܲ�������Ϊ3�����������Ϊ0��1��2���Դ�����
extern "C" __declspec(dllexport) int getSwathName(int index, char *swathName)
{
	std::vector<char*> lstSwath;
	gProjectC.getAllSwathName(lstSwath);

	//ȡ�ò�������
	int iSize = (int)lstSwath.size();
	if (0 >= iSize)
		return ERROR_CODE_NOSWATH;

	//�������õ�Ŀ����ߵ�����
	int i = 0;
	char* pSwath = NULL;
	for (std::vector<char*>::iterator it_Swath = lstSwath.begin(); it_Swath != lstSwath.end(); it_Swath++)
	{
		if (NULL == *it_Swath)
			continue;

		//�ҵ���Ӧ�Ĳ�����
		if (index == i)
		{
			pSwath = *it_Swath;
			//strcpy(swathName, *it_Swath);
			break;
		}

		i++;
	}
	if (!pSwath)
		return ERROR_CODE_SUCCESS;
	int slen = (int)strlen(pSwath);
	if (slen == 0)
		return ERROR_CODE_SUCCESS;

	int length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pSwath, slen, NULL, 0);
	wchar_t* buffer = (wchar_t*)malloc(length * 2 + 2);
	memset((char *)buffer, 0, length * 2 + 2);

	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pSwath, slen, buffer, length);

	wcscpy((wchar_t *)swathName, buffer);
	free(buffer);
	return ERROR_CODE_SUCCESS;
}

//ͨ��ָ���Ĳ������ƣ���ȡָ�������е�ͨ������
extern "C" __declspec(dllexport) int getChannelCount(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//��ȡĿ�����
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return 0;

	//�Ӳ����л�ȡͨ������
	int iCount = 0;
	swath->getChannelCount(iCount);

	return iCount;
}

//ͨ��ָ���������ƺ�ͨ����������ȡָ��ͨ���е�Trace������ͨ��ID��Ϊ1��2��3 ... ...������Ǻ�Ŀ¼��ͨ���ļ���Ӧ�ġ�
extern "C" __declspec(dllexport) int getTraceCount(const char *  swathName, int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//��ȡĿ�����
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return 0;

	//��ȡ��Ӧ��ͨ������
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return 0;

	//ȡ��ͨ�������ͷ��Ϣ
	SwathChannelHeader * channelHeader = channel->getChannelHeader();
	if (!channelHeader)
		return 0;

	//ֱ�ӷ���Trace����
	return channelHeader->getTraceCount();
}

//��ȡһ�������У�Trace֮��ļ��
extern "C" __declspec(dllexport) double getDistanceInterval(const char *  swathName, int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return 0;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return 0;

	//��ȡĿ�����
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return 0;

	//��ȡ��Ӧ��ͨ������
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return 0;

	//ȡ��ͨ�������ͷ��Ϣ
	int iCount = 0;
	SwathChannelHeader * channelHeader = channel->getChannelHeader();
	if (!channelHeader)
		return 0;

	//�õ�Trace���
	double dist = channelHeader->getIntervalDist();
	return dist;
}

/*
Fun:���ݲ�������ͨ���ţ�Trace��ʼ��Trace������������ֱɨ��ͼ
PAram:
	picFile    Ŀ��ͼ��·��(·�����ļ������磺C:\MyPath\���ϴ��2022-10-30\Pic\123.jpg)
	swathName  ������
	withLine   �Ƿ���б���
	channel    ͨ����
	traceBegin ��ʼTrace
	traceEnd   ����Trace
	deep       ��Ҫ�u�D����ȹ���
Return: �ɹ�����0��ʧ�ܶ�Ӧ�Ĵ�����
*/
extern "C" __declspec(dllexport) int exportRadarPicV(const char * picFile, const char * swathName, BOOL withLine, int channelID, int traceBegin, int traceCount, int deep)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	length = (int)wcslen((wchar_t*)picFile);
	if (length <= 0)
		return ERROR_PARAM;
	char szPicFile[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)picFile, length, szPicFile, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//4���Դ洢·�����ļ������򵥵�У��
	if (strlen(szPicFile) <= 4)
		return ERROR_PARAM;

	//���������ȣ�cm��
	//int iAllDeep = ( int )( channel->getChannelHeader()->getTimeWindow() * channel->getChannelHeader()->getSoilVel() / 20 );
	//�@���Ǵ�ֱ�G�ӱ���
	//int sampleCount = channel->getChannelHeader()->getSample();
	//ͨ�^��ȹ���Ӌ�㴹ֱ�ӱ���
	//int height = sampleCount * deep / iAllDeep;

	//5�����������ݴ洢ΪͼƬ�ļ�
	//int iResult = theApp.SaveAsPicV(szPicFile, channel, traceBegin, traceCount, height);
	int iResult = theApp.SaveAsPicV(szPicFile, channel, traceBegin, traceCount, deep);
	return iResult;
	if (0 != iResult)
		return ERROR_CODE_SYS;
	else
		return ERROR_CODE_SUCCESS;
}

/*
Fun:���ݲ���������ȣ�Trace��ʼ��Trace����������ˮƽɨ��ͼ
PAram:
	picFile    Ŀ��ͼ��·��
	swathName  ������
	withLine   �Ƿ���б���
	deep       ���
	traceBegin ��ʼTrace
	traceEnd   ����Trace
Return: �ɹ�����0��ʧ�ܶ�Ӧ�Ĵ�����
*/
extern "C" __declspec(dllexport) int exportRadarPicH(const char * picFile, const char * swathName, BOOL withLine, int deep, int traceBegin, int traceCount)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	length = (int)wcslen((wchar_t*)picFile);
	if (length <= 0)
		return ERROR_PARAM;
	char szPicFile[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)picFile, length, szPicFile, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3���Դ洢·�����ļ������򵥵�У��
	if (strlen(szPicFile) <= 4)
		return ERROR_PARAM;

	//4�����������ݴ洢ΪͼƬ�ļ�
	int iResult = theApp.SaveAsPicH(szPicFile, swath, deep, traceBegin, traceCount);
	return iResult;
	if (0 != iResult)
		return ERROR_CODE_SYS;
	else
		return ERROR_CODE_SUCCESS;

	return 0;
}

/*
Fun:���ݲ�������Trace�ţ���ȡ��ǰ��ץ��ͼ����
PAram:
	picFileFront    Ŀ��ͼ��·��--ǰ������ͷ
	picFileBack     Ŀ��ͼ��·��--��������ͷ
	picFileLeft     Ŀ��ͼ��·��--��������ͷ
	picFileRight    Ŀ��ͼ��·��--��������ͷ
	swathName  ������
	traceNum   Trace��
Return: �ɹ�����0��ʧ�ܶ�Ӧ�Ĵ�����
*/
extern "C" __declspec(dllexport) int takePhotoPic(char * picFileFront, char * picFileBack, char * picFileLeft, char * picFileRight, const char * swathName, int traceNum, int iOffsetTime)
{
	//�����Ϸ����ж�
	if ((!picFileFront) || (!picFileBack) || (!picFileLeft) || (!picFileRight))
		return ERROR_PARAM;

	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	char szPicFileFront[512] = { 0 };
	char szPicFileBack[512] = { 0 };
	char szPicFileLeft[512] = { 0 };
	char szPicFileRight[512] = { 0 };
	//4�����������ݴ洢ΪͼƬ�ļ�
	int iResult = theApp.TakePhotoPic(&gProjectC, szPicFileFront, szPicFileBack, szPicFileLeft, szPicFileRight, swath, traceNum, iOffsetTime);
	if (0 != iResult)
		return ERROR_CODE_SYS;

	//��.Net���ܲ��������ȴ���25�ͻ���e������ȥ���Uչ��
	szPicFileFront[21] = 0;
	szPicFileBack[21] = 0;
	szPicFileLeft[21] = 0;
	szPicFileRight[21] = 0;

	int slen = (int)strlen(szPicFileFront);
	if (0 != slen)
	{
		length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileFront, slen, NULL, 0);
		wchar_t* bufferFront = (wchar_t*)malloc(slen * 2 + 2);
		memset((char *)bufferFront, 0, slen * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileFront, slen, bufferFront, length);
		wcscpy((wchar_t *)picFileFront, bufferFront);
		free(bufferFront);
	}

	slen = (int)strlen(szPicFileBack);
	if (0 != slen)
	{
		length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileBack, slen, NULL, 0);
		wchar_t* bufferBack = (wchar_t*)malloc(slen * 2 + 2);
		memset((char *)bufferBack, 0, slen * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileBack, slen, bufferBack, length);
		wcscpy((wchar_t *)picFileBack, bufferBack);
		free(bufferBack);
	}

	slen = (int)strlen(szPicFileLeft);
	if (0 != slen)
	{
		length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileLeft, slen, NULL, 0);
		wchar_t* bufferLeft = (wchar_t*)malloc(slen * 2 + 2);
		memset((char *)bufferLeft, 0, slen * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileLeft, slen, bufferLeft, length);
		wcscpy((wchar_t *)picFileLeft, bufferLeft);
		free(bufferLeft);
	}

	slen = (int)strlen(szPicFileRight);
	if (0 != slen)
	{
		length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileRight, slen, NULL, 0);
		wchar_t* bufferRight = (wchar_t*)malloc(slen * 2 + 2);
		memset((char *)bufferRight, 0, slen * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szPicFileRight, slen, bufferRight, length);
		wcscpy((wchar_t *)picFileRight, bufferRight);
		free(bufferRight);
	}

	return 0;
}

/*��ȡ������ȣ���λcm*/
extern "C" __declspec(dllexport) int getDeep(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(1);
	if (!channel)
		return 0;

	return (int)(channel->getChannelHeader()->getTimeWindow() * channel->getChannelHeader()->getSoilVel() / 20);
}

/*��ȡ��ֱ�ֱ���*/
extern "C" __declspec(dllexport) int getSampleCount(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(1);
	if (!channel)
		return 0;

	return channel->getChannelHeader()->getSample();
}

/*----------------------------������MatGpr���㴦��-------------------------*/
/*
Fun:��ʼ������ȡָ�����ߺ�ͨ������
Param:swathName ָ���Ĳ�����
*/
extern "C" __declspec(dllexport) int initSwath(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3����ʼ����������
	int iResult = theApp.InitSwath(swath);
	if (0 != iResult)
		return ERROR_CODE_SYS;
	else
		return ERROR_CODE_SUCCESS;
}

/*
Fun:ȥ��ʼ����ጷ�ָ�����ߵ�ͨ������
Param:swathName ָ���Ĳ�����
*/
extern "C" __declspec(dllexport) int unInitSwath(const char * swathName)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3��ȥ��ʼ����������
	int iResult = theApp.UnInitSwath(swath);
	if (0 != iResult)
		return ERROR_CODE_SYS;
	else
		return ERROR_CODE_SUCCESS;
}



/*
Fun:����ԭ������
*/
extern "C" __declspec(dllexport) int BackupData(const char * swathName, const int channelID)
{
	/*
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameDat; fileNameHeader; path");
	//MessageBox(NULL, szOut, szOut, 0);

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);

	//����ԭ������
	bool bReturn = theApp.BackupData(fileNameData, fileNameHeader, path);
	if (!bReturn)
		return 0;
	else
		return -1;
	*/

	return ERROR_CODE_SUCCESS;
}


/*
Fun:ɾ����Ч��trace
Param:const char *  swathName   Ŀ�����
				int channelID   ͨ����
*/
extern "C" __declspec(dllexport) int deleteInvalidTrace(const char *  swathName, int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char path[512] = { 0 };
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//���ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//ɾ����ЧTrace�����Ҵ����µ��ļ�
	bool bReturn = theApp.deleteInvalidTrace(szSwathName, channelID, path, swath);
	if (bReturn)
		return ERROR_CODE_SUCCESS;
	else
		return ERROR_CODE_SYS;
}


/*
Fun:����ָ������ͨ����ֱ�ﲨͼ��
Param:swathName      ָ���Ĳ�����
	  channelID      ͨ����
	  sigPositionPic Ŀ��ֱ�ﲨͼ���ļ�
	  traceNum       ֱ�ﲨ��Ӧ��trace�ţ�����0����ֱ�ﲨΪ��Ӧ��trace��Ӧ��ֱ�ﲨ��С��0��ʾΪ����ͼ���ֱ�ﲨ
*/
extern "C" __declspec(dllexport) int SigPositionPicByVende(const char * swathName, const int channelID, const char * sigPositionPic, const int traceNum)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת����������
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4����ֱ�ﲨͼ��洢·�����ļ������򵥵�У��
	length = (int)wcslen((wchar_t*)sigPositionPic);
	if (length <= 0)
		return ERROR_PARAM;

	char szSigPositionPic[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת��ֱ�ﲨ·��������
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)sigPositionPic, length, szSigPositionPic, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	/*
	//���Դ�ӡ���
	//char fileNameData[512]   = { "RadarData_001_A01.iprb" };
	//char fileNameHeader[512] = { "RadarData_001_A01.rad" };
	//char path[512] = { "C:\\3D������Ϣϵͳ\\�����Ź���\\2\\0928test_2\\RadarData\\" };

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	char szOut[512] = { 0 };
	sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	MessageBox(NULL, szOut, szOut, 0);
	*/

	//����ֱ�ﲨͼ��
	return theApp.SigPosPicureByVende(channel, szSigPositionPic, traceNum);
}

/*
Fun:�г�ָ����ֱ�ﲨͼ��,ͬʱ�����г���ֱ�ﲨλ��-----�����Զ��г�ֱ�ﲨ�����ҷ����г����
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
Return: �г��ɹ��������г���λ��
		�г�ʧ�ܣ�����0
*/
extern "C" __declspec(dllexport) int SigPositionCutEx2ByVende(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת����������
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	int traceCount = channel->getChannelHeader()->getTraceCount();
	//����һ�����ݺϷ���Trace������ֱ�ﲨ����
	int traceNum = 1;
	for (; traceNum < traceCount; traceNum = traceNum + 10)
	{
		int traceValue = 0;
		theApp.TraceAvg(channel, traceNum, traceValue);

		//��Trace��ƽ��ֵ����һ����ֵ��˵����������Ч��
		if (traceValue > 100)
			break;
	}

	//��ȡֱ�ﲨλ��
	int iZero = theApp.MiniSigPosNum(channel, traceNum, 2);
	int iTmpDample = channel->getChannelHeader()->getSample();  //���ں�������

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//����ȥֱ�ﲨͼ��
	bool bReturn = theApp.SimpleSigPositionCut(fileNameData, fileNameHeader, path, channel, iZero);
	if (bReturn)
	{
		//���г������ֵ�����������зŴ���Ϊֱ�ﲨͼ��ĸ߶�Ϊ1000���أ�
		return iZero * 1000 / iTmpDample;
	}
	else
		return 0;
}

/*
Fun:�г�ָ����ȵ�ֱ�ﲨͼ��
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  iZero     �г����
*/
extern "C" __declspec(dllexport) int SigPositionCutEx3ByVende(const char * swathName, const int channelID, int iZero)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת����������
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//��ֱ�ﲨͼ���е�����ת��Ϊʵ���������
	int iTmpDample = channel->getChannelHeader()->getSample();  //���ں�������
	iZero = iZero * iTmpDample / 1000;

	//�г�ֱ�ﲨ
	return theApp.SigPositionCutByVende(fileNameData, fileNameHeader, path, channel, iZero);
}

/*
Fun:��ͼ���M�Ў�ͨ�˲�(��˹�˲���)
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  freqMax   ͨ������l��
	  freqMin   ͨ����С�l��
	  order     ������Խ��ͼ��Խ���������\���ٶ�Խ��(ȡֵ����1-10)
*/
extern "C" __declspec(dllexport) int FilterGauss(const char * swathName, const int channelID, int freqMax, int freqMin, int order)
{
	//����֧��
	return -1;
}

/*
Fun:��ͼ���M�Ў�ͨ�˲�(�����˲���)
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  freqMax   ͨ������l��
	  freqMin   ͨ����С�l��
	  order     ������Խ��ͼ��Խ���������\���ٶ�Խ��(ȡֵ����1-10)
*/
extern "C" __declspec(dllexport) int FilterIdeal(const char * swathName, const int channelID, int freqMax, int freqMin, int order)
{
	//����֧��
	return -1;

}



/*----------------------------�����Ǻ����ļӽ��ܴ���-------------------------*/
/*
 * Fun:���ݺ������ܴ���
 * Param:
 *   src �����ܵ�ԭʼ����
 *   dst ���ܺ��Ŀ������
 *   length ԭʼ���ݳ���
 */
extern "C" __declspec(dllexport) int DataSimpleCrypt(const unsigned char *src, unsigned char *dst, int length)
{
	//�����Ϸ���У�
	if ((!src) || (!dst))
		return ERROR_PARAM;

	//ÿ���ֽ�ѭ�hȡ��
	for (int i = 0; i < length; i++)
	{
		dst[i] = src[i] ^ 0xff;
	}

	return 0;
}

/*
 * Fun:���ݺ������ܴ���
 * Param:
 *   src �����ܵ�ԭʼ����
 *   dst ���ܺ��Ŀ������
 *   length ԭʼ���ݳ���
 */
extern "C" __declspec(dllexport) int DataSimpleDecrypt(const unsigned char *src, unsigned char *dst, int length)
{
	//�����Ϸ���У�
	if ((!src) || (!dst))
		return ERROR_PARAM;

	//ÿ���ֽ�ѭ�hȡ��
	for (int i = 0; i < length; i++)
	{
		dst[i] = src[i] ^ 0xff;
	}

	return 0;
}
/*----------------------------�����Ǻ����ļӽ��ܴ���-------------------------*/





//==========================����һ��ӿڣ�ֱ������ȥ������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��====================//
/*
Fun:ȥ��ֱ������
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
*/
/*
extern "C" __declspec(dllexport) int RemoveDC(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);
	//����ȥֱ�ﲨͼ��
	bool bReturn = theApp.RemoveDC(fileNameData, fileNameHeader, path);
	if (!bReturn)
		return 0;
	else
		return -1;
}
*/
//==========================����һ��ӿڣ�ֱ������ȥ������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��====================//



//==========================����һ��ӿڣ���������ȥ������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��====================//
//��ʵ��

//==========================����һ��ӿڣ���������ȥ������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��====================//

/*
Fun:��ͼ���M�Ў�ͨ�˲�(�����˲���)
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  freqMax   ͨ������l��
	  freqMin   ͨ����С�l��
	  order     ������Խ��ͼ��Խ���������\���ٶ�Խ��(ȡֵ����1-10)
*/
extern "C" __declspec(dllexport) int FilterIdealSimple(const char * swathName, const int channelID, int freqMax, int freqMin, int order)
{
	//����֧��
	return -1;

}


//==========================����һ��ӿڣ�ֱ�ﲨ������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��======================//

/*
Fun:����ָ������ͨ����ֱ�ﲨͼ��
Param:swathName      ָ���Ĳ�����
	  channelID      ͨ����
	  sigPositionPic Ŀ��ֱ�ﲨͼ���ļ�
*/
extern "C" __declspec(dllexport) int SimpleSigPositionPic(const char * swathName, const int channelID, const char * sigPositionPic)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת����������
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4����ֱ�ﲨͼ��洢·�����ļ������򵥵�У��
	length = (int)wcslen((wchar_t*)sigPositionPic);
	if (length <= 0)
		return ERROR_PARAM;

	char szSigPositionPic[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת��ֱ�ﲨ·��������
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)sigPositionPic, length, szSigPositionPic, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//����ֱ�ﲨͼ��
	return theApp.SigPosPicureByVende(channel, szSigPositionPic, 10);
}

/*
Fun:��ȡָ������ͨ����ֱ�ﲨλ��
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
*/
extern "C" __declspec(dllexport) int SimpleSigPositionGet(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת����������
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//��Ϊǰ����Trace���������׳����⣬��������ǰ����Trace���ݽ��м���
	//��ȡֱ�ﲨλ��
	return theApp.SigPosNumByVende(channel, 0, 1);
}

/*
Fun:��ȡָ������ͨ����ֱ�ﲨλ��
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  direct    ֱ�ﲨ���� -1�����ֱ�ﲨ�Ĳ��壻 0������ֱ�ﲨ�Ĳ��壻 1���ұ�ֱ�ﲨ�Ĳ��壻
	  waveNum   ָ���������µĵڼ�������
*/
extern "C" __declspec(dllexport) int SimpleSigPositionGetEx(const char * swathName, const int channelID, int direct, const int waveNum)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת����������
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//��Ϊǰ����Trace���������׳����⣬��������ǰ����Trace���ݽ��м���
	//��ȡֱ�ﲨλ��
	return theApp.SigPosNumByVende(channel, direct, waveNum);
}

/*
Fun:�г�ָ����ֱ�ﲨͼ��
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  iZero     �г����
*/
extern "C" __declspec(dllexport) int SimpleSigPositionCut(const char * swathName, const int channelID, int iZero)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת����������
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�г�ֱ�ﲨ
	return theApp.SimpleSigPositionCut(fileNameData, fileNameHeader, path, channel, iZero);
}

/*
Fun:�г�ָ����ֱ�ﲨͼ���Զ�����ֱ�ﲨ���
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
*/
extern "C" __declspec(dllexport) int SimpleSigPositionCutEx(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���---�˴�ת����������
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return ERROR_PARAM;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//��ȡֱ�ﲨλ�ã�//��Ϊǰ����Trace���������׳����⣬��������ǰ����Trace���ݽ��м���
	int iZero = theApp.SigPosNumByVende(channel, 0, 1);

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�г�ֱ�ﲨ
	return theApp.SimpleSigPositionCut(fileNameData, fileNameHeader, path, channel, iZero);
}


//==========================����һ��ӿڣ�ֱ�ﲨ������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��======================//


//==========================����һ��ӿڣ����������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��==========================//
/*
Fun:������������Զ����棬
	�ź����ӵķ������û��ֶ�ָ�����ײ��ڴ�������ʱ������ʵ�������ԭ�����ݵĵĻ��������Ӷ�Ӧ��ƫ��
	ע�⣬ʹ�����洦��ǰһ��Ҫ���ֱ�ﲨ�г�
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  order:    �������(ϵ��������)������1������������֮�䣬Խ��Խƽ��
	  offset:   ƫ��ϵ�����������ϵ��½�������
*/
extern "C" __declspec(dllexport) int SimpleGainInvDecayConst(const char * swathName, const int channelID, int order, int *offset)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4��У���������
	if (order <= 0)
		return ERROR_PARAM;    //���������������һ��Ĭ�ϵ��������
	if (!offset)
		return ERROR_PARAM;
	int iTmpSample = channel->getChannelHeader()->getSample();  //���ں�������
	if (order > iTmpSample)
		order = iTmpSample;


	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5��ʵ�����������
	return theApp.GainInvDecayConst(fileNameData, path, channel, order, offset);
}

/*
Fun:������������Զ����棬
	�ź����ӵķ������û��ֶ�ָ�����ײ��ڴ�������ʱ������ʵ�������ԭ�����ݵĵĻ��������Ӷ�Ӧ��ϵ��������ֵ
	ע�⣬ʹ�����洦��ǰһ��Ҫ���ֱ�ﲨ�г�
	Param : swathName ָ���Ĳ�����
	channelID ͨ����
	order : �������(ϵ��������)������1������������֮�䣬Խ��Խƽ��
	coef  : ����ϵ�����������ϵ��½�������(ȡֵ��Χ 1 ... ... 100)
*/
extern "C" __declspec(dllexport) int SimpleGainInvDecayCoef(const char * swathName, const int channelID, int order, int *coef)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4��У���������
	if (order <= 0)
		return ERROR_PARAM;    //���������������һ��Ĭ�ϵ��������
	if (!coef)
		return ERROR_PARAM;
	int iTmpSample = channel->getChannelHeader()->getSample();  //���ں�������
	if (order > iTmpSample)
		order = iTmpSample;


	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5��ʵ�����������
	return theApp.GainInvDecayCoef(fileNameData, path, channel, order, coef);
}

/*
Fun:������������Զ����� ----- ������
	�ź����ӵķ�����ģ�͹�ʽ���㡣
	�ź�˥��ģ��: y = b - b * (k/100) * (( x ^ n )/sample)
	ע�⣬ʹ�����洦��ǰһ��Ҫ���ֱ�ﲨ�г�
	Param : swathName ָ���Ĳ�����
	channelID ͨ����
	k : б�ʣ�ȡֵ��ΧΪ1 ... ... 100
	n : ָ����ȡֵ��ΧΪ1 ... ... 10
*/
extern "C" __declspec(dllexport) int SimpleGainInvDecayCurve(const char * swathName, const int channelID, int k, int n)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	//4��У���������
	if ((k <= 0) || (n <= 0))
	{
		//Ĭ��ֵ
		k = 25;
		n = 19;
	}

	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5��ʵ�����������
	return theApp.GainInvDecayCurve(fileNameData, path, channel, k, n);
}

//==========================����һ��ӿڣ����������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��==========================//

//==========================����һ��ӿڣ�ȥ������������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��==========================//
/*
Fun:ȥ����������
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
*/
extern "C" __declspec(dllexport) int SimpleRemoveBackgr(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5��ɾ����������
	return theApp.SimpleRemoveBackgr(fileNameData, path, channel);
}
/*
Fun:ȥ����������
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  offerset  ��������������ƫ�ƣ�ǳ�㱳����������Ҫ��һ�㣬��㱳������Ҫ������һ�㡣
*/
extern "C" __declspec(dllexport) int SimpleRemoveBackgrEx(const char * swathName, const int channelID, int offerset)
{
	return -1;
}
//==========================����һ��ӿڣ�ȥ������������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��==========================//



//==========================����һ��ӿڣ�ɾ��ֱ����������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��========================//
/*
*Fun:ɾ��ֱ������
*Param:swathName ָ���Ĳ�����
*	  channelID ͨ����
*/
extern "C" __declspec(dllexport) int SimpleRemoveDC(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//1���ж�������������Ƿ�Ϸ�
	if (strlen(szSwathName) <= 0)
		return 0;

	//2�����ݲ������ƣ�ȡ�ò��߶���
	Swath* swath = gProjectC.getSwath(szSwathName);
	if (!swath)
		return ERROR_CODE_NOSWATH;

	//3���ڲ��߶����У�����ͨ��ID��ȡ��ͨ���Ķ���
	SwathChannel* channel = swath->getChannel(channelID);
	if (!channel)
		return ERROR_CODE_NOCHANNEL;

	char fileNameData[512] = { 0 };
	char path[512] = { 0 };
	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//5��ɾ����������
	return theApp.SimpleRemoveDC(fileNameData, path, channel);
}

//==========================����һ��ӿڣ�ɾ��ֱ����������ʹ��Matlab�Ŀ⣬��C++ֱ��ʵ��========================//


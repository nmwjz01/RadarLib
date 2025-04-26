/*
 * Fun:�����ֲ�ͬ�����ݸ�ʽת��Ϊiprb��iprh
 */

#include "framework.h"
#include <io.h>
//#include <atlimage.h>

/*
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <winnls.h>
*/

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

#include "Transform2Segy\\Transform2SegyBase.h"

/*
 * Fun:��һ��DT�ļ�ת��ΪIPRH��IPRB�ļ�-------����ת���������ϣ�����ת�����Ա���һ���������
 *     �÷���֧�ֵ��ļ�ΪLA010001.DT
 */
extern "C" __declspec(dllexport) int tranfersDT(const char * swathPath, int freq, float separation, const char * swathPathDst)
{
	int length = (int)wcslen((wchar_t*)swathPath);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDT[256] = { 0 };
	memset(swathPathDT, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathPath, length, swathPathDT, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//�����ļ���·��
	char configFile[256] = { 0 };
	memset(configFile, 0, 256);

	//char configFile[256] = { 0 };
	sprintf(configFile, "%s\\Stream X1.xml", swathPathDT);
	//AfxMessageBox(configFile);
	//��ȡlast trace��sample
	FILE * config = fopen(configFile, "r");
	if (!config)
		return ERROR_DT_XML;

	//Ŀ��惦�ļ���·��
	char pathDst[256] = { 0 };
	memset(pathDst, 0, 256);

	length = (int)wcslen((wchar_t*)swathPathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathPathDst, length, pathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//��������ļ���Ϣ
	char * configContent = (char *)malloc(1024 * 8);

	//��ȡȫ�������ļ�
	char * tmp = configContent;
	size = 1024 * 8;
	while (true)
	{
		//��ȡ��������
		size = size - (int)fread(configContent, 1, size, config);
		if (size < 1024)
			break;
		if (feof(config))
			break;
	}
	//�ѽ���ȡ�ꮅ�������P�]
	fclose(config);

	//Ӌ����K��XStep
	float distance = 0.0;
	//��ȡ��step
	{
		char *tmp = strstr(configContent, "<EncoderStack");
		if (!tmp)
		{
			free(configContent);
			return ERROR_DT_NOCHANNEL;
		}
		char *xstep = strstr(tmp, "\"");
		if (!xstep)
		{
			free(configContent);
			return ERROR_DT_ERRCHANNEL;
		}

		int istep = 0;
		xstep[0] = ' ';
		sscanf(xstep, "%d", &istep);

		tmp = strstr(configContent, "<XStep value");
		if (!tmp)
		{
			tmp = "";
			//free(configContent);
			//return false;
		}
		xstep = strstr(tmp, "\"");
		if (!xstep)
		{
			xstep = "000";
			//free(configContent);
			//return false;
		}

		float fstep = (float)0.002;
		if (0 != strcmp(xstep, "000"))
		{
			xstep[0] = ' ';
			sscanf(xstep, "%f", &fstep);
		}

		//Ӌ����K��XStep
		distance = fstep * istep;
	}

	//ѭ�h����ÿ������
	int iSwathNum = 1;
	while (true)
	{
		//����Ŀ�����
		char szDTSwathFile[512] = { 0 };
		sprintf(szDTSwathFile, "%s\\RAW_%06d.scan", swathPathDT, iSwathNum);

		//��������ҵ�Ŀ����ߣ�������
		FILE *fpTmp = fopen(szDTSwathFile, "r");
		if (!fpTmp)
			break;
		fclose(fpTmp);

		//�������Q
		char szDTSwathName[64] = { 0 };
		sprintf(szDTSwathName, "%04d", iSwathNum);
		iSwathNum++;

		//ѭ�h��ȡÿ��ͨ��
		tmp = strstr(configContent, "<Channels Count=");
		if (!tmp)
			break;

		//����ҵ�Ŀ����ߣ����������Ϣ
		while (true)
		{
			//��ȡÿ��ͨ��
			tmp = strstr(tmp, "<Channel Id");
			if (!tmp)
				break;
			tmp++;

			char *channelID = strstr(tmp, "Id=\""); channelID = strstr(channelID, "\""); channelID++;
			char *channelNs = strstr(tmp, "Range_ns=\""); channelNs = strstr(channelNs, "\""); channelNs++;
			char *channelSamples = strstr(tmp, "Samples=\""); channelSamples = strstr(channelSamples, "\""); channelSamples++;

			int samples = 0;
			int iChannelID = 0;
			int ns = 0;
			sscanf(channelNs, "%d", &ns);
			sscanf(channelID, "%d", &iChannelID);
			sscanf(channelSamples, "%d", &samples);

			char szDTH[512] = { 0 };
			sprintf(szDTH, "%s\\GRED_HD\\LA%02d%s.HDR_00", swathPathDT, iChannelID, szDTSwathName);
			char szDTD[512] = { 0 };
			sprintf(szDTD, "%s\\GRED_HD\\LA%02d%s.DT", swathPathDT, iChannelID, szDTSwathName);
			char szIPRH[512] = { 0 };
			sprintf(szIPRH, "%s\\%s_A%02d.iprh", pathDst, szDTSwathName, iChannelID);
			char szIPRB[512] = { 0 };
			sprintf(szIPRB, "%s\\%s_A%02d.iprb", pathDst, szDTSwathName, iChannelID);

			int tracecount = 0;

			if (9 <= iChannelID)
				freq = 200;
			else
				freq = 600;

			TransformDT oTransformDT;
			int iResult = oTransformDT.dt2iprh(szDTH, freq, ns, distance, separation, szIPRH, tracecount);
			iResult = oTransformDT.dt2iprb(szDTD, szIPRB, samples, tracecount);
		}

		//����cor�ļ�
		char szCor[512] = { 0 };
		sprintf(szCor, "%s\\%s.cor", pathDst, szDTSwathName);
		FILE * pFileCor = fopen(szCor, "w+");
		fclose(pFileCor);

		//����time�ļ�
		char szTime[512] = { 0 };
		sprintf(szTime, "%s\\%s.time", pathDst, szDTSwathName);
		FILE * pFileTime = fopen(szTime, "w+");
		fclose(pFileTime);
	}

	//ጷſռ�
	free(configContent);

	return ERROR_CODE_SUCCESS;
}

/*
 * Fun:��IDS�ɼ�����(��������)ת��ΪIPRH/IPRB
 *     �÷���֧��RAW_000001.scan/Stream X.xml�ļ���ʽ.
 */
extern "C" __declspec(dllexport) int tranfersIDS(const char * pathIDS, const char * pathDst, int freq, float separation)
{
	//1��ȡ��IDS���ݵ�Ŀ¼
	int length = (int)wcslen((wchar_t*)pathIDS);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathIDS[256] = { 0 };
	memset(swathPathIDS, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathIDS, length, swathPathIDS, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;


	//2��ȡ�ô洢Ŀ���Ŀ¼
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3����idsת��Ϊiprh
	TransformIDS oTransformIDS;
	return oTransformIDS.transformIDS(swathPathIDS, swathPathDst, separation);
}

/*
 * Fun:��IDS��16λ���ɼ�����(��������)ת��ΪIPRH/IPRB
 *     �÷���֧�ֽ�IDS��Swath001_Array01.data/Swath001_Array01.index/Swath001.swath/Survey�ļ�תΪiprb
 */
extern "C" __declspec(dllexport) int tranfersIDSData16(const char * pathIDS, const char * pathDst)
{
	//1��ȡ��IDS���ݵ�Ŀ¼
	int length = (int)wcslen((wchar_t*)pathIDS);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathIDS[256] = { 0 };
	memset(swathPathIDS, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathIDS, length, swathPathIDS, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2��ȡ�ô洢Ŀ���Ŀ¼
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3����ids data��ʽ���ļ�ת��Ϊiprh��iprb
	TransformIDS oTransformIDS;
	return oTransformIDS.transformIDS16(swathPathIDS, swathPathDst);
}

/*
 * Fun:��IDS��8λ���ɼ�����(��������)ת��ΪIPRH/IPRB ---------�������ã�����Stream_DP�״�
 *     �÷���֧�ֽ�IDS��Swath001_Array01.data�ļ�תΪiprb
 */
extern "C" __declspec(dllexport) int tranfersIDSData08(const char * pathIDS, const char * pathDst, int iSample)
{
	//1��ȡ��IDS���ݵ�Ŀ¼
	int length = (int)wcslen((wchar_t*)pathIDS);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathIDS[256] = { 0 };
	memset(swathPathIDS, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathIDS, length, swathPathIDS, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2��ȡ�ô洢Ŀ���Ŀ¼
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3����ids data��ʽ���ļ�ת��Ϊiprh��iprb
	TransformIDS oTransformIDS;
	oTransformIDS.setSample(iSample);
	return oTransformIDS.transformIDS08(swathPathIDS, swathPathDst);
}

/*
 * Fun:��Mala����ת��ΪIPRH��IPRB��ʽ
 * Param:  pathMala  Mala���ݴ��Ŀ¼
 *         pathDst   Ŀ�����ݴ��Ŀ¼
 */
extern "C" __declspec(dllexport) int tranfersMala(const char * pathMala, const char * pathDst)
{
	//1��ȡ��Mala���ݵ�Ŀ¼
	int length = (int)wcslen((wchar_t*)pathMala);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathMala[256] = { 0 };
	memset(swathPathMala, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathMala, length, swathPathMala, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2��ȡ�ô洢Ŀ���Ŀ¼
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//Mala����ת������
	TransformMala oTransformMala;

	//3��Ѱ��Mala32
	char szPathRd7[512] = { 0 };
	//����Ѱ��Mala32�ı�־�ļ�
	sprintf(szPathRd7, "%s\\*.rd7", swathPathMala);
	intptr_t hFileRd7;
	struct _finddata_t oFileInfoRd7;
	if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) == -1L)
	{
		printf("û�з���Mala32�Ĳ�������\n");
		_findclose(hFileRd7);
	}
	else
	{
		_findclose(hFileRd7);

		//4����Malaת��Ϊiprh/iprb
		return oTransformMala.transformMala32(swathPathMala, swathPathDst);
	}

	//===================���������ȼ��16λ�״�����ά���ά��Ȼ�����ʵ���������ת��=====================//
	char szPathRad[512] = { 0 };
	//Ѱ��RAD�ļ������Ҷ�ȡRAD�ļ��е�Channels���ݣ�ͨ��RAD�ļ��ж���3ά���ݻ���2ά����
	sprintf(szPathRad, "%s\\*.rad", swathPathMala);
	intptr_t hFileRad;
	struct _finddata_t oFileInfoRad;
	if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
	{
		_findclose(hFileRad);
		printf("û�з���Mala16�Ĳ�������\n");
		return 9;
	}
	//�ر�ԭ�е��������
	_findclose(hFileRad);

	//������16λMala���ݵĴ�����ȡRad�ļ��е�Channels����
	//���ú����״�������3ά��2ά����
	memset(szPathRad, 0, 512);
	sprintf(szPathRad, "%s\\%s", swathPathMala, oFileInfoRad.name);

	//��ȡRad�ļ�����������
	FILE *fpRad = fopen(szPathRad, "r");
	char *szBuff = (char *)malloc(2048);
	int iIndex = 0;
	int iLength = 0;
	while (true)
	{
		iLength = (int)fread(szBuff + iIndex, 1, 2048 - iIndex, fpRad);
		iIndex = iIndex + iLength;
		if (!iLength)
			break;
	}
	fclose(fpRad);

	//����Channels
	char *szChannel = strstr(szBuff, "CHANNELS:");
	if (szChannel)
		//6����Mala16��ά����ת��Ϊiprh
		return oTransformMala.transformMala16(swathPathMala, swathPathDst);
	else
		//8����Mala16��ά����ת��Ϊiprh
		return oTransformMala.transformMala16Ex(swathPathMala, swathPathDst);
}

/*
 * Fun:��Mala����ת��ΪIPRH��IPRB��ʽ
 * Param:  pathMala  һ�������32λMala���ݴ��Ŀ¼-----���������Mala32�Ķ�ά�״�
 *         pathDst   Ŀ�����ݴ��Ŀ¼
 */
extern "C" __declspec(dllexport) int tranfersMala32Ex(const char * pathMala, const char * pathDst)
{
	//1��ȡ��Mala���ݵ�Ŀ¼
	int length = (int)wcslen((wchar_t*)pathMala);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathMala[256] = { 0 };
	memset(swathPathMala, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathMala, length, swathPathMala, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2��ȡ�ô洢Ŀ���Ŀ¼
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3��Ѱ��Mala32
	char szPathMcor[512] = { 0 };
	//����Ѱ��Mala32�ı�־�ļ�
	sprintf(szPathMcor, "%s\\*.cor", swathPathMala);
	intptr_t hFileMcor;
	struct _finddata_t oFileInfoMcor;
	if ((hFileMcor = (intptr_t)_findfirst(szPathMcor, &oFileInfoMcor)) == -1L)
	{
		printf("û�з���Mala32�Ĳ�������\n");
		_findclose(hFileMcor);
	}
	else
	{
		_findclose(hFileMcor);

		//4����Malaת��Ϊiprh
		TransformMala oTransformMala;
		return oTransformMala.transformMala32Ex(swathPathMala, swathPathDst);
	}

	return ERROR_NOFILE_MALA;
}

/*
 * Fun:��3DRadar����ת��ΪIPRH��IPRB��ʽ(ŷ����ء���ͼ)
 * Param:  path3DRadar  3DRadar���ݴ��Ŀ¼
 *         pathDst   Ŀ�����ݴ��Ŀ¼
 */
extern "C" __declspec(dllexport) int tranfers3DRadar(const char * path3DRadar, const char * pathDst)
{
	//1��ȡ��3DRadar���ݵ�Ŀ¼
	int length = (int)wcslen((wchar_t*)path3DRadar);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPath3DRadar[256] = { 0 };
	memset(swathPath3DRadar, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)path3DRadar, length, swathPath3DRadar, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2��ȡ�ô洢Ŀ���Ŀ¼
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3��Ѱ��3DRadar
	char szPathTxt[512] = { 0 };
	//����Ѱ��3DRadar 32�ı�־�ļ�
	sprintf(szPathTxt, "%s\\*.txt", swathPath3DRadar);
	intptr_t hFileTxt;
	struct _finddata_t oFileInfoMcor;
	if ((hFileTxt = (intptr_t)_findfirst(szPathTxt, &oFileInfoMcor)) == -1L)
	{
		printf("û�з���3DRadar�Ĳ�������\n");
		_findclose(hFileTxt);
	}
	else
	{
		_findclose(hFileTxt);

		//4����3DRadarת��Ϊiprh
		Transform3DRadar oTransform3DRadar;
		return oTransform3DRadar.transform3DRadar(swathPath3DRadar, swathPathDst);
	}

	return ERROR_NOFILE_3DRADAR;
}

/*
 * Fun:��Segy����ת��ΪIPRH��IPRB��ʽ(ŷ����ء���ͼ) --- ֧��32λ��16λ
 * Param:  path      Segy���ݴ��Ŀ¼
 *         pathDst   Ŀ�����ݴ��Ŀ¼
 */
extern "C" __declspec(dllexport) int tranfersSegy(const char * path, const char * pathDst)
{
	//1��ȡ��Segy���ݵ�Ŀ¼
	int length = (int)wcslen((wchar_t*)path);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathSegy[256] = { 0 };
	memset(swathPathSegy, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)path, length, swathPathSegy, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2��ȡ�ô洢Ŀ���Ŀ¼
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//3��Ѱ��Segy
	char szPathTmp[512] = { 0 };
	//����Ѱ��Segy�ı�־�ļ�
	sprintf(szPathTmp, "%s\\*.sgy", swathPathSegy);
	intptr_t hFileFd;
	struct _finddata_t oFileInfo;
	if ((hFileFd = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
	{
		printf("û�з���sgy�Ĳ�������\n");
		_findclose(hFileFd);
	}
	else
	{
		_findclose(hFileFd);

		TransformSegy theAppTransformSegy;
		//4����Segyת��Ϊiprh
		return theAppTransformSegy.transformSegy(swathPathSegy, swathPathDst);
	}

	return ERROR_NOFILE_SEGY;
}

/*
 * Fun:��GSSI����ת��ΪIPRH��IPRB��ʽ(ŷ����ء���ͼ)
 * Param:  pathGSSI  GSSI���ݴ��Ŀ¼
 *         pathDst   Ŀ�����ݴ��Ŀ¼
 */
extern "C" __declspec(dllexport) int tranfersGSSI(const char * pathGSSI, const char * pathDst)
{
	return -1;
}

/*
 * Fun:��һ����ͨ����Segy�ļ��ֽ�Ϊ�������ͨ����Segy�ļ�
 * Param:  pathFile  ��ͨ����Segy�ļ�(����·��)
 *         pathDst   Ŀ��Ŀ¼����ŵ�ͨ����Segy�ļ�
 */
extern "C" __declspec(dllexport) int tranfersSegySplit(const char * pathFile, const char * pathDst)
{
	//1��ȡ��Segy���ݵ�Ŀ¼
	int length = (int)wcslen((wchar_t*)pathFile);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��--�����ļ�
	char swathPathSegySrc[256] = { 0 };
	memset(swathPathSegySrc, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathFile, length, swathPathSegySrc, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//2��ȡ�ô洢Ŀ���Ŀ¼
	length = (int)wcslen((wchar_t*)pathDst);
	if (length <= 0)
		return ERROR_PARAM;

	//ԭʼ·��
	char swathPathDst[256] = { 0 };
	memset(swathPathDst, 0, 256);

	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)pathDst, length, swathPathDst, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	Transform2SegyBase *pTransform2SegyBase = new Transform2SegyBase();

	int iResult = pTransform2SegyBase->SplitChannel(swathPathDst, swathPathSegySrc);

	delete pTransform2SegyBase;

	return iResult;
}

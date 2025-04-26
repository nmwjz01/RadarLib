#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\\pch.h"
#include "..\\framework.h"
#include <atlimage.h>

class TransferIDSData16
{
public:
	TransferIDSData16()
	{
		//����һ���ṩͨ�����Զ���
		headVersion = 10;        // header version: 10
		dataVersion = 16;        // data version: 16-16b; 32-32b 
		strcpy(date     , "2023-10-22");    //���߲�������
		strcpy(startTime, "00:00:00"  );    //���߿�ʼʱ��
		strcpy(stopTime , "23:59:59"  );    //���߽���ʱ��
		strcpy(ANTENNA  , "200 MHz"   );    //Ƶ�ʣ�
		separation  = 0.08;
		samples     = 512;        //һ��trace�У���������
		runs        = 0;
		maxStacks   = 0;
		frequency   = 200;        // Sampling Frequency (MHz)����һ��Ƶ�ʣ�
		timeWindow  = 60;         //ʱ�䴰�����İl�����ص�ʱ��
		lastTrace   = 0;
		strcpy(trigSource, "wheel");    //
		intervalTime = 0.05;       // time interval (seconds)
		intervalDist = 0.2;        // real distance interval (meters)
		intervalUser = 0.05;       // user distance interval (meters)
		zeroLevel    = 0;
		soilVel      = 100;        // soil velocity (m/us)
		positioning  = 0;          // it is used for combined files only
		channels     = 0;
		memset(chanName, 0 , 16);      // channel name (like "T1-R1")
	}
	~TransferIDSData16()
	{
		printf("ת����һ�������ļ�����\n");
	}

	/*
	 * Fun:ת��data��չ��������Ϊiprh/iprb
	 * Param: pathSrc�ļ����Ŀ¼
	 *        pathDst iprb�ļ����Ŀ¼
	 *        swathName ������
	 *        swathID   ����ID
	 * ����:�ɹ�����true��ʧ�ܷ���false
	 */
	bool transferData(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
	{
		char swathFile[128] = { 0 };
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


private:

	/*
	 * Fun:��Data��ʽ��IDS�ļ�ת��Ϊiprh��iprb
	 * Param: pathSrc�ļ����Ŀ¼
	 *        pathDst iprb�ļ����Ŀ¼
	 *        swathFile�ļ����� ... ... ����:Swath001_Array01.data
	 *        swathName������   ... ... ����:Swath001
	 *        sampleÿ��trace����������
	 * ����:�ɹ�����true��ʧ�ܷ���false
	 * Example:Swath001_Array01.data -->Swath001_Array01_A01.iprb��Swath001_A02.iprb ... ... Swath001_A10.iprb
	 */
	bool transferFirst(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
	{
		//�����Ϸ����ж�
		if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
			return false;
		if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
			return false;

		//200M hz�ĵ������8
		separation = 0.08;
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
			printf( "ԭ�ļ�(%s)������", pathIDSFile);
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
			saveHeader( pathDst, swathName, index);
		}

		return true;
	}

	/*
	 * Fun:��Data��ʽ��IDS�ļ�ת��Ϊiprh��iprb
	 * Param: pathSrc�ļ����Ŀ¼
	 *        pathDst iprb�ļ����Ŀ¼
	 *        swathFile�ļ����� ... ... ����:Swath001_Array02.data
	 *        swathName������   ... ... ����:Swath002
	 *        sampleÿ��trace����������
	 * ����:�ɹ�����true��ʧ�ܷ���false
	 * Example:Swath001_Array02.data -->Swath001_Array02_A01.iprb��Swath001_A11.iprb ... ... Swath001_A29.iprb
	 */
	bool transferSecond(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
	{
		//�����Ϸ����ж�
		if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
			return false;
		if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
			return false;

		//600M hz�ĵ������8
		separation = 0.04;
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
			int buffSize    = 0;         //��¼��ȡ/д��ÿ��Traceʱ�Լ���ȡ�����ֽ�����
			char *tmpStart  = szBuff;  //��¼��ȡ/д��ÿ��Traceʱ�Ŀ�ͷλ��

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

		strcpy(ANTENNA, "600 MHz");    //Ƶ�ʣ�
		frequency = 600;        // Sampling Frequency (MHz)����һ��Ƶ�ʣ�

		//д��iprhͷ�ļ�
		for (int index = 11; index <= 29; index++)
		{
			//д��iprh
			saveHeader(pathDst, swathName, index);
		}

		return true;
	}

	/*
	 * Fun:����cor�ļ�
	 * Param: pathDst�ļ����Ŀ¼
	 *        swathName ������
	 *        swathID ����ID
	 * ����:�ɹ�����true��ʧ�ܷ���false
	 */
	bool makeFileCor(const char * pathDst, const char *swathName, int swathID)
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
	bool makeFileTime(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
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
		char fileTimeIDS[1024]     = { 0 }; sprintf(fileTimeIDS    , "%s\\%s_Array01.index", pathSrc, swathName);
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

		//����ѭ���������trace,һֱ���ļ���β
		while (!feof(fpTimeIDS))
		{
			fread(buffTime, 1, 20, fpTimeIDS);
			int traceNum         = *((int *)buffTime);        //ȡ��Trace��
			long long traceTimeMsecond = buffTime[7] * 256;   //ȡ��Traceʱ�䣨��Կ�ʼ��ʱ��--���룩
                      traceTimeMsecond = (traceTimeMsecond + buffTime[6]) * 256;
                      traceTimeMsecond = (traceTimeMsecond + buffTime[5]) * 256;
                      traceTimeMsecond = (traceTimeMsecond + buffTime[4]) - offset;
			long long  traceTime = traceTimeMsecond/1000 + timeStart * 1000;    //Traceʱ�䣬����ʱ��--����
			//long long  traceTime = timeStart * 1000 + 900;    //Traceʱ�䣬����ʱ��--����
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
	int saveHeader(const char * pathDst, const char *swathName, int channelNum)
	{
		//�����Ϸ���У��
		if ((nullptr == pathDst) || (nullptr == swathName) )
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

		sprintf(s, "HEADER VERSION: %d\n"    , headVersion     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DATA VERSION: %d\n"      , dataVersion     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DATE: %s\n"              , date            );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "START TIME: %s\n"        , startTime       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "STOP TIME: %s\n"         , stopTime        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA: %s\n"           , ANTENNA         );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA SEPARATION: %f\n", separation      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SAMPLES: %d\n"           , samples         );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SIGNAL POSITION: 12\n"                     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CLIPPED SAMPLES: 0\n"                      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "RUNS: %d\n"              , runs            );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "MAX STACKS: %d\n"        , maxStacks       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "AUTOSTACKS: 1\n"                           );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "FREQUENCY: %d\n"         , (int)frequency  );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TIMEWINDOW: %d\n"        , (int)timeWindow );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "LAST TRACE: %d\n"        , lastTrace       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TRIG SOURCE: %s\n"       , trigSource      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TIME INTERVAL: %f\n"     , intervalTime    );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DISTANCE INTERVAL: %f\n" , intervalDist    );  fwrite(s, 1, strlen(s), m_pFile);

		if (channelNum < 11)
		{
			sprintf(s, "USER DISTANCE INTERVAL: %f\n", 0.05);  fwrite(s, 1, strlen(s), m_pFile);
		}
		else
		{
			sprintf(s, "USER DISTANCE INTERVAL: %f\n", 0.1);  fwrite(s, 1, strlen(s), m_pFile);
		}

		sprintf(s, "STOP POSITION: %f\n"     , 1.0             );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "WHEEL NAME: WENDE\n"                       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "WHEEL CALIBRATION: 1.0\n"                  );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ZERO LEVEL: %d\n"        , zeroLevel       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SOIL VELOCITY: %f\n"     , soilVel         );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "PREPROCESSING: 0\n"                        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "OPERATOR COMMENT: _\n"                     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA F/W: 48001262\n"                   );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA H/W: 0\n"                          );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA FPGA: WENDE\n"                     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA SERIAL: 001\n"                     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SOFTWARE VERSION: 1.0.0\n"                 );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "POSITIONING: %d\n"       , positioning     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CHANNELS: %d\n"          , channels        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CHANNEL CONFIGURATION: T2 - R2\n"          );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CH_X_OFFSET: 0.000000\n"                   );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CH_Y_OFFSET: 0.000000\n"                   );  fwrite(s, 1, strlen(s), m_pFile);
		fclose(m_pFile);

		return 0;
	}


private:
	//����һ���ṩͨ�����Զ���
	int headVersion     = 0;        // header version: 10
	int dataVersion     = 0;        // data version: 16-16b; 32-32b 
	char date[16]       = { 0 };    //���߲�������
	char startTime[9]   = { 0 };    //���߿�ʼʱ��
	char stopTime[9]    = { 0 };    //���߽���ʱ��
	char ANTENNA[10]    = { 0 };    //Ƶ�ʣ�
	double separation   = 0;
	int samples         = 0;        //һ��trace�У���������
	int runs            = 0;
	int maxStacks       = 0;
	double frequency    = 0;        // Sampling Frequency (MHz)����һ��Ƶ�ʣ�
	double timeWindow   = 0;        //ʱ�䴰�����İl�����ص�ʱ��
	int lastTrace       = 0;        //һ��������trace������
	char trigSource[16] = { 0 };
	double intervalTime = 0;        // time interval (seconds)
	double intervalDist = 0;        // real distance interval (meters)
	double intervalUser = 0;        // user distance interval (meters)
	int    zeroLevel    = 0;
	double soilVel      = 0;        // soil velocity (m/us)
	int    positioning  = 0;        // it is used for combined files only
	int    channels     = 0;
	char   chanName[16] = { 0 };    // channel name (like "T1-R1")

};

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
#include "TransformSegy.h"


/*
* Fun：将指定目录下的Segy数据转化为IPRH、IPRD，存储到指定目录下
* Param：
*      swathPathSegy    Segy数据目录(sgy格式数据所在目录)
*      pathDst          IPRH、IPRD目标目录
* Return：成功返回0，失败返回错误码
*/
int TransformSegy::transformSegy(const char * swathPathSegy, const char * pathDst)
{
	//搜索雷达目录下，指定文件,Rd数据
	char szPathSegy[512] = { 0 };

	//首先寻找Mala16的标志文件
	sprintf(szPathSegy, "%s\\*.sgy", swathPathSegy);
	intptr_t hFileTxt;
	struct _finddata_t oFileInfo;
	if ((hFileTxt = (intptr_t)_findfirst(szPathSegy, &oFileInfo)) == -1L)
	{
		_findclose(hFileTxt);
		printf("没有发现Segy的测线数据\n");
		return ERROR_NOFILE_SEGY;
	}

	int swathIndex = 0;
	int iResult = ERROR_CODE_SUCCESS;

	//1、循环处理Segy的所有测线
	while (true)
	{
		//获取测线名称
		char swathName[256] = { 0 };
		if (strlen(oFileInfo.name) > 4)
			strncpy(swathName, oFileInfo.name, strlen(oFileInfo.name) - 4);

		swathIndex++;

		//这里得到测线文件名，将该文件转化位Impluse
		//转化一个测线文件
		iResult = transformSegySwath(swathPathSegy, swathName, pathDst, swathIndex );
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		//搜索下一个，没有搜索到就返回
		if (_findnext(hFileTxt, &oFileInfo) == -1L)
			break;
	}
	_findclose(hFileTxt);

	return iResult;
}

//将一个segy文件转化为一个iprh\iprb测线
int TransformSegy::transformSegySwath(const char *szPathSegy, const char *swathName, const char *pathDst, int swathIndex)
{
	char swathPathFileSegy[512] = { 0 };
	sprintf(swathPathFileSegy, "%s\\%s.sgy", szPathSegy, swathName);

	//将segy文件打开
	FILE * srcFile = fopen(swathPathFileSegy, "rb");
	if (!srcFile)
	{
		//MessageBox("打开文件失败", "提示", MB_OK);
		return ERROR_NOFILE_SEGY;
	}

	unsigned char szBuff[64] = { 0 };

	//跳过3200字节的EBCDIC
	//fseek(srcFile, 3200, SEEK_SET);

	//（跳过的字节中，包括3200字节的EBCDIC）
	fseek(srcFile, 3220, SEEK_SET);
	//读取对应的数据：
	//1、采样点数；2、每个采样点的数据长度
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

	//一个Trace中采样的点数
	int samples  = byte0 * 256 + byte1;
	int dataType = byte4 * 256 + byte5; (3 == dataType) ? dataType = 16 : dataType = 32; // else dataType = 32;

	//测线中通道的数量
	int iChannel = 1;

	//跳到数据开始，数据组成 240字节数据头 + 数据内容
	fseek(srcFile, 3600, SEEK_SET);

	int iResult = ERROR_CODE_SUCCESS;

	//循环读取
	while (!feof(srcFile))
	{
		std::map<int, void*>*lstDataTmp = new std::map<int, void*>();
		int iTraceCount = 0;


		//创建一个iprb文件
		iResult = CreateIprb(pathDst, swathName, swathIndex, iChannel);
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		iResult = ERROR_CODE_SUCCESS;

		//循环读取一个通道的iprb数据
		while (!feof(srcFile))
		{
			int iReadTimes = 5;    //最多只能读取5次
			int iReadLen  = 32;    //要求读取32字节数据
			while (iReadLen > 0)
			{
				iReadLen = iReadLen - (int)fread(szBuff + 32 - iReadLen, 1, iReadLen, srcFile);

				iReadTimes--;
				if (iReadTimes <= 0)
					break;
			}
			//读取不到，说明数据已经没有了
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

			//计算通道号
			int iChannelTmp = ((byte4 * 256  + byte5 ) * 256 + byte6 ) * 256 + byte7;
			//计算Trace号
			//int iTraceNum   = ((byte20 * 256 + byte21) * 256 + byte22) * 256 + byte23;

			//计算一个Trace的数据量
			int iDataLen = samples * dataType / 8;

			//跳过每个Trace头的240字节(之所以减去32字节，那是因为前面读取了32字节)
			fseek(srcFile, 240 - 32, SEEK_CUR);

			unsigned char *szData = (unsigned char *)malloc(iDataLen);
			iReadTimes = 5;           //最多只能读取5次
			iReadLen   = iDataLen;    //要求读取字节数是一个Trace的数据量
			while (iReadLen > 0)
			{
				int iLengthTmp = (int)fread(szData + iDataLen - iReadLen, 1, iReadLen, srcFile);
				iReadLen = iReadLen - iLengthTmp;

				iReadTimes--;
				if (iReadTimes <= 0)
					break;
			}
			//读取不到，说明数据已经没有了
			if (0 != iReadLen)
			{
				iResult = ERROR_FILE_READ;
				break;
			}
			if (16 == dataType)
			{
				//整个Buff大小头转化 --- 16位数据转化
				for (int i = 0; i < samples; i++)
				{
					unsigned char iTmp = szData[2 * i];
					szData[2 * i] = szData[2 * i + 1];
					szData[2 * i + 1] = iTmp;
				}
			}
			else
			{
				//整个Buff大小头转化 --- 16位数据转化
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
			//通道号不一样，说明已经完成一个通道的读取
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

		//清除原有的数据
		oIprb.clearTraceData();
		//设置iprb的参数
		oIprb.setChannelParam(dataType, samples, 0);
		//添加新数据
		oIprb.AppendTraceData(lstDataTmp, dataType);
		lstDataTmp->clear();
		//存储iprb
		iResult = SaveIprb();
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		//创建一个iprh文件
		iResult = CreateIprh(pathDst, swathName, swathIndex, iChannel);
		if (ERROR_CODE_SUCCESS != iResult)
			break;
		//设置iprh文件内容
		oIprh.setDataVersion(dataType);
		oIprh.setSample(samples);
		oIprh.setFrequency(5120.0);
		oIprh.setSoilVel(100.0);
		oIprh.setTraceCount(iTraceCount);
		//存储iprh文件内容
		iResult = SaveIprh();
		if (ERROR_CODE_SUCCESS != iResult)
			break;

		//通道号增加一
		iChannel++;
	}

	fclose(srcFile);


	//创建iprh通道数据--创建cor文件
	iResult = CreateCor(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;
	//创建iprh通道数据--创建gps文件
	iResult = CreateGps(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;
	//创建iprh通道数据--创建mrk文件
	iResult = CreateMrk(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;
	//创建iprh通道数据--创建ord文件
	iResult = CreateOrd(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;
	//创建iprh通道数据--创建time文件
	iResult = CreateTime(pathDst, swathName, swathIndex);
	if (ERROR_CODE_SUCCESS != iResult)
		return iResult;

	return iResult;
}

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
#include "TransformIDS.h"



TransformIDS::TransformIDS()
{
	//下面一组提供通道属性定义
	headVersion = 10;        // header version: 10
	dataVersion = 16;        // data version: 16-16b; 32-32b 
	strcpy(date, "2023-10-22");       //测线操作日期
	strcpy(startTime, "00:00:00");    //测线开始时间
	strcpy(stopTime, "23:59:59");     //测线结束时间
	strcpy(ANTENNA, "200 MHz");       //频率？
	separation   = 0.01;              //默认光栅距离0.01
	samples      = 512;               //一个trace中，采样数量
	runs         = 0;
	maxStacks    = 0;
	frequency    = 200;        // Sampling Frequency (MHz)又是一个频率？
	timeWindow   = 60;         //时间窗，波從發出返回的时间
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
	printf("转化第一个测线文件出错\n");
}

/*
 * Fun:产生cor文件
 * Param: pathDst文件存放目录
 *        swathName 测线名
 *        swathID 测线ID
 * 返回:成功返回true，失败返回false
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
 * Fun:产生time文件
 * Param: pathDst文件存放目录
 *        swathName 测线名
 *        swathID 测线ID
 * 返回:成功返回true，失败返回false
 */
bool TransformIDS::makeFileTime(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
{
	//===========下面为转化IDS的Index文件到Impluse的Time文件==========//
	//取得起始时间
	char fileSurvey[1024] = {}; sprintf(fileSurvey, "%s\\Survey", pathSrc);
	//定义并且打开IDS的Survey文件
	FILE *fpSurvey = fopen(fileSurvey, "r");
	if (!fpSurvey)
		return false;
	char szSurvey[256] = { 0 };
	fread(szSurvey, 1, 250, fpSurvey);
	fclose(fpSurvey);
	char *tmp = strstr(szSurvey, "Begin=\"");   //时间精确到秒
	tmp = tmp + 7;

	long long timeStart = 0;
	//将字符串转化为整数
	Utils::char2int(tmp, timeStart);

	//写入time文件
	char fileTimeIDS[1024] = { 0 }; sprintf(fileTimeIDS, "%s\\%s_Array01.index", pathSrc, swathName);
	char fileTimeImpluse[1024] = { 0 }; sprintf(fileTimeImpluse, "%s\\Swath%03d.time", pathDst, swathID);
	//定义并且打开IDS的index文件
	FILE *fpTimeIDS = fopen(fileTimeIDS, "rb");
	if (!fpTimeIDS)
		return false;
	//定义并且打开impluse的time文件
	FILE *fpTimeImpluse = fopen(fileTimeImpluse, "wb");
	if (!fpTimeImpluse)
	{
		fclose(fpTimeIDS);
		return false;
	}

	//用于存储每行时间的读取数据
	unsigned char buffTime[20] = { 0 };
	static long long offset = 0;
	/*
	//如果是第一个测线，则记录测线起始时间偏移
	if (1 == swathID)
	{
		//跳过IDS index的文件头(120字节，包含trace号为0的数据)
		fseek(fpTimeIDS, 100, SEEK_SET);
		fread(buffTime, 1, 20, fpTimeIDS);

		offset = buffTime[7] * 256;   //取得Trace时间（相对开始的时间--毫秒）
		offset = (offset + buffTime[6]) * 256;
		offset = (offset + buffTime[5]) * 256;
		offset = (offset + buffTime[4]);
	}
	else
		//跳过IDS index的文件头(120字节，包含trace号为0的数据)
		fseek(fpTimeIDS, 120, SEEK_SET);
	*/

	//*
	int channelCount = 0;
	unsigned char buffChannel[20] = { 0 };
	//跳到通道数量的位置
	fseek(fpTimeIDS, 44, SEEK_SET);
	//读取通道数量，数据长度是4字节，我们只需要1个字节即可
	fread(buffChannel, 1, 4, fpTimeIDS);
	channelCount = buffChannel[0];
	//计算文件头的长度：
	//             4字节文件标识   + 32字节MD5    + 4字节标志测线的第几个文件 + 4字节天线频率 + 
	//             4字节通道数     + 4字节Sample数+ 4字节未知                 + 4字节未知     + 通道号列表（通道数量*4）
	int lenHeader = 4 + 32 + 4 + 4 +
	                4 +  4 + 4 + 4 + channelCount * 4;
	//跳过文件头
	fseek(fpTimeIDS, lenHeader, SEEK_SET);
	//*/

	//下面循环处理后续trace,一直到文件结尾
	while (!feof(fpTimeIDS))
	{
		fread(buffTime, 1, 20, fpTimeIDS);
		int traceNum = *((int *)buffTime);        //取得Trace号
		long long traceTimeMsecond = buffTime[7] * 256;   //取得Trace时间（相对开始的时间--毫秒）
		traceTimeMsecond = (traceTimeMsecond + buffTime[6]) * 256;
		traceTimeMsecond = (traceTimeMsecond + buffTime[5]) * 256;
		traceTimeMsecond = (traceTimeMsecond + buffTime[4]) - offset;
		long long  traceTime = traceTimeMsecond / 1000 + timeStart * 1000;    //Trace时间，绝对时间--毫秒
		char szTmp[128] = { 0 };
		char szLine[128] = { 0 };

		Utils::long2date(traceTime, szTmp);               //将时间转化为字符串格式
		sprintf(szLine, "%d	%s\r\n", traceNum, szTmp);
		fwrite(szLine, 1, strlen(szLine), fpTimeImpluse);
	}
	fclose(fpTimeIDS);
	fclose(fpTimeImpluse);

	return true;
}

/*
 * Fun:存储iprh文件头信息
 * Param:
 * Return:成功返回true，失败返回false
 */
int TransformIDS::saveHeader(const char * pathDst, const char *swathName, int channelNum)
{
	frequency = channelFreq[channelNum];
	intervalDist = traceStep[channelNum];

	//参数合法性校验
	if ((nullptr == pathDst) || (nullptr == swathName))
		return -1;
	if ((0 >= strlen(pathDst)) || (0 >= strlen(swathName)))
		return -1;

	char pathFile[1024] = { 0 };
	sprintf(pathFile, "%s\\%s_A%02d.iprh", pathDst, swathName, channelNum);

	FILE * m_pFile = fopen(pathFile, "wt+");
	if (nullptr == m_pFile)
		return -2;

	//循环读取各个配置参数
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

//======================================下面函数，后续备用，用于Stream_DP雷达========================================//
/*
 * Fun:将Data格式的IDS(8位)文件转化为iprh和iprb-------------后续备用，用于Stream_DP雷达
 * Param: pathSrc文件存放目录
 *        pathDst iprb文件存放目录
 *        swathFile文件名称 ... ... 例如:Swath001_Array01.data
 *        swathName测线名   ... ... 例如:Swath001
 *        sample每个trace的样本数量
 * 返回:成功返回true，失败返回false
 * Example:Swath001_Array01.data -->Swath001_Array01_A01.iprb、Swath001_A02.iprb ... ... Swath001_A10.iprb
 */
bool TransformIDS::transfer08First(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
{
	//参数合法性判断
	if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
		return false;
	if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
		return false;

	//200M hz的时间窗是60ns
	timeWindow = 60;

	//trace数量计数器
	lastTrace = 0;
	//输入的ids文件
	char pathIDSFile[1024] = { 0 };
	sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
	FILE *fpSrc = fopen(pathIDSFile, "rb");
	if (!fpSrc)
	{
		printf("原文件(%s)不存在", pathIDSFile);
		return false;
	}

	char pathDstFile[1024] = { 0 };
	FILE *fpDst[20] = { NULL };
	for (int index = 1; index <= 19; index++)
	{
		//打开输出的文件
		sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
		fpDst[index] = fopen(pathDstFile, "wb");
	}

	samples = 320;
	int traceSize = samples * 2;
	char *szBuff = (char *)malloc(traceSize + 256);

	//跳过36字节头信息
	fread(szBuff, 1, 36, fpSrc);

	while (!feof(fpSrc))
	{
		int buffSize = 0;         //记录读取/写入每个Trace时以及读取的总字节数字
		char *tmpStart = szBuff;  //记录读取/写入每个Trace时的开头位置

		//第一个通道
		{
			buffSize = 0;
			//读取第一个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第一个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[1]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第二个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第二个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第二个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[2]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第三个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第三个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[3]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第四个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第四个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第四个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[4]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

		}

		//第五个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第五个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第五个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[5]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第六个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第六个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第六个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[6]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第七个通道
		{
			tmpStart = szBuff;
			//读取第七个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第七个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[7]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第八个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第八个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}

			//写入第八个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[8]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第九个通道
		{
			tmpStart = szBuff;
			//读取第九个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第九个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[9]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第十个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第十个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第十个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[10]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//第十一个通道
		{
			tmpStart = szBuff;
			//读取第十一个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第十一个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[11]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//十二个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第十二个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第十二个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[12]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//十三个通道
		{
			tmpStart = szBuff;
			//读取第十三个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第十三个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[13]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//十四个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第十四个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第十四个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[14]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}

		}

		//十五个通道
		{
			tmpStart = szBuff;
			//读取第十五个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第十五个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[15]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//十六个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第十六个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			//写入第十六个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[16]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//十七个通道
		{
			tmpStart = szBuff;
			//读取第十七个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第十七个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[17]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//十八个通道
		{
			tmpStart = szBuff;
			buffSize = 0;
			//读取第十八个通道的数据
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}

			//写入第十八个通道的数据
			tmpStart = szBuff;
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[18]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//十九个通道
		{
			tmpStart = szBuff;
			//读取第十九个通道的数据
			buffSize = 0;
			//从原始文件读取一个Trace
			while (buffSize < traceSize)
			{
				int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
				buffSize = buffSize + tmpLen;

				if (feof(fpSrc))
					break;
			}
			tmpStart = szBuff;
			//写入第十九个通道的数据
			while (buffSize > 0)
			{
				int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[19]);
				buffSize = buffSize - tmpLen;
				tmpStart = szBuff + tmpLen;
			}
		}

		//测线数量计数
		lastTrace++;
	}

	free(szBuff);

	//关闭数据输入文件和数据输出文件(iprb)
	fclose(fpSrc);
	for (int index = 1; index <= 19; index++)
	{
		fclose(fpDst[index]);
	}

	//记录到Trace数量
	lastTrace--;

	//写入iprh头文件
	for (int index = 1; index <= 19; index++)
	{
		//写入iprh
		saveHeader(pathDst, swathName, index);
	}

	return true;
}

/*
 * Fun:将Data格式的IDS(8位)文件转化为iprh和iprb-------------后续备用，用于Stream_DP雷达
 * Param: pathSrc文件存放目录
 *        pathDst iprb文件存放目录
 *        swathFile文件名称 ... ... 例如:Swath001_Array02.data
 *        swathName测线名   ... ... 例如:Swath002
 *        sample每个trace的样本数量
 * 返回:成功返回true，失败返回false
 * Example:Swath001_Array02.data -->Swath001_Array02_A01.iprb、Swath001_A11.iprb ... ... Swath001_A29.iprb
 */
bool TransformIDS::transfer08Second(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
{
	//参数合法性判断
	if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
		return false;
	if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
		return false;

	//600M hz的时间窗是80ns
	timeWindow = 80;

	//trace数量计数器
	lastTrace = 0;
	//输入的ids文件
	char pathIDSFile[1024] = { 0 };
	sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
	FILE *fpSrc = fopen(pathIDSFile, "rb");
	if (!fpSrc)
	{
		printf("原文件(%s)不存在", pathIDSFile);
		return false;
	}

	char pathDstFile[1024] = { 0 };
	FILE *fpDst[31] = { NULL };
	for (int index = 20; index <= 30; index++)
	{
		//打开输出的文件
		sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
		fpDst[index] = fopen(pathDstFile, "wb");
	}

	samples = 320;

	int traceSize = samples * 2;
	char *szBuff = (char *)malloc(traceSize);

	//跳过36字节头信息
	fread(szBuff, 1, 36, fpSrc);

	while (!feof(fpSrc))
	{
		int buffSize = 0;         //记录读取/写入每个Trace时以及读取的总字节数字
		char *tmpStart = szBuff;  //记录读取/写入每个Trace时的开头位置

		buffSize = 0;
		//读取第11个通道的数据
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第11个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[20]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第12个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第12个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[21]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第13个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第13个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[22]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第14个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第14个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[23]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第15个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第15个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[24]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第16个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第16个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[25]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第17个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第17个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[26]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第18个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第18个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[27]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第19个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第19个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[28]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第20个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第20个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[29]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第21个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第21个通道的数据
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[30]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//测线数量计数
		lastTrace++;
	}

	free(szBuff);

	//关闭
	fclose(fpSrc);
	for (int index = 20; index <= 30; index++)
	{
		fclose(fpDst[index]);
	}

	lastTrace--;

	//写入iprh头文件
	for (int index = 20; index <= 30; index++)
	{
		//写入iprh
		saveHeader(pathDst, swathName, index);
	}

	return true;
}

/*
 * Fun:转化data扩展名的数据为iprh/iprb---应用于IDS8位雷达-------------后续备用，用于Stream_DP雷达
 * Param: pathSrc文件存放目录
 *        pathDst iprb文件存放目录
 *        swathName 测线名
 *        swathID   测线ID
 * 返回:成功返回true，失败返回false
 */
bool TransformIDS::transferData08(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
{
	char swathFile[128] = { 0 };
	//处理雷达目录下，指定文件(第一个IDS数据文件)
	sprintf(swathFile, "%s_Array01.data", swathName);
	//这里第一个data文件转化为iprb文件
	if (false == transfer08First(pathSrc, pathDst, swathFile, swathName))
	{
		printf("转化第一个测线文件出错\n");
		return false;
	}

	/* --------------------Array01和Array02处理交界--------------------*/
	//处理雷达目录下，指定文件(第二个IDS数据文件)
	sprintf(swathFile, "%s_Array02.data", swathName);
	//这里第一个data文件转化为iprb文件
	if (false == transfer08Second(pathSrc, pathDst, swathFile, swathName))
	{
		printf("转化第二个测线文件出错\n");
		return false;
	}

	/* --------------------生成cor文件--------------------*/
	//生成cor文件
	if (false == makeFileCor(pathDst, swathName, swathID))
	{
		printf("产生cor文件出错\n");
		return false;
	}

	/* --------------------生成time文件-------------------*/
	if (false == makeFileTime(pathSrc, pathDst, swathName, swathID))
	{
		printf("产生time文件出错\n");
		return false;
	}

	return true;
}

/*
* Fun：将指定目录下的ids数据转化为IPRH、IPRD，存储到指定目录下---应用于IDS8位雷达-------------后续备用，用于Stream_DP雷达
* Param：
*      pathIDS     IDS数据文件（Swath001_Array01.data）
*      pathDst     IPRH、IPRD目标目录
* Return：成功返回TRUE，失败返回FALSE
*/
int TransformIDS::transformIDS08(const char * pathIDS, const char * pathDst)
{
	int swathID = 1;
	char swathName[128] = { 0 };
	//搜索雷达目录下，指定文件(第一个IDS数据文件)
	char szPathTmp[512] = { 0 };
	sprintf(szPathTmp, "%s\\*.swath", pathIDS);
	intptr_t hFile = 0;
	struct _finddata_t oFileInfo;

	//查询是否存在第一条测线
	if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
	{
		printf("没有发现IDS的测线数据（*.swath）\n");
		return -1;
	}


	do
	{
		//获取第一个文件的文件名
		sprintf(szPathTmp, "%s\\%s", pathIDS, oFileInfo.name);

		//得到测线名
		strncpy(swathName, oFileInfo.name, strlen(oFileInfo.name) - 6);
		//MessageBox(NULL, swathName, swathName, 0);
		//将IDS data文件转化为iprh、iprb
		bool result = transferData08(pathIDS, pathDst, swathName, swathID);
		if (true != result)
		{
			//关闭文件搜索
			_findclose(hFile);
			return -2;
		}

		swathID++;
		//} while ((hFile = (intptr_t)_findnext(hFile, &oFileInfo)) != -1L);
	} while (_findnext(hFile, &oFileInfo) == 0);

	//关闭文件搜索
	if (0 != hFile)
		_findclose(hFile);

	return 0;
}
//======================================上面函数，后续备用，用于Stream_DP雷达========================================//

/*
 * Fun:将Data格式的IDS(16位)文件转化为iprh和iprb
 * Param: pathSrc文件存放目录
 *        pathDst iprb文件存放目录
 *        swathFile文件名称 ... ... 例如:Swath001_Array01.data
 *        swathName测线名   ... ... 例如:Swath001
 *        sample每个trace的样本数量
 * 返回:成功返回true，失败返回false
 * Example:Swath001_Array01.data -->Swath001_Array01_A01.iprb、Swath001_A02.iprb ... ... Swath001_A10.iprb
 */
bool TransformIDS::transferFirst(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
{
	//参数合法性判断
	if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
		return false;
	if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
		return false;

	//200M hz的时间窗是60ns
	timeWindow = 60;

	//trace数量计数器
	lastTrace = 0;
	//输入的ids文件
	char pathIDSFile[1024] = { 0 };
	sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
	FILE *fpSrc = fopen(pathIDSFile, "rb");
	if (!fpSrc)
	{
		printf("原文件(%s)不存在", pathIDSFile);
		return false;
	}

	char pathDstFile[1024] = { 0 };
	FILE *fpDst[11] = { NULL };
	for (int index = 1; index <= 10; index++)
	{
		//打开输出的文件
		sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
		fpDst[index] = fopen(pathDstFile, "wb");
	}

	int traceSize = samples * 2;
	char *szBuff = (char *)malloc(traceSize);

	//跳过36字节头信息
	fread(szBuff, 1, 36, fpSrc);

	while (!feof(fpSrc))
	{
		int buffSize = 0;         //记录读取/写入每个Trace时以及读取的总字节数字
		char *tmpStart = szBuff;  //记录读取/写入每个Trace时的开头位置

		buffSize = 0;
		//读取第一个通道的数据
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第一个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[1]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第二个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第二个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[2]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第三个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第三个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[3]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第四个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第四个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[4]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第五个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第五个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[5]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第六个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第六个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[6]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第七个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第七个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[7]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第八个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第八个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[8]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第九个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第九个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[9]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第十个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		tmpStart = szBuff;
		//写入第十个通道的数据
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[10]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//测线数量计数
		lastTrace++;
	}

	free(szBuff);

	//关闭数据输入文件和数据输出文件(iprb)
	fclose(fpSrc);
	for (int index = 1; index <= 10; index++)
	{
		fclose(fpDst[index]);
	}

	//记录到Trace数量
	lastTrace--;

	//写入iprh头文件
	for (int index = 1; index <= 10; index++)
	{
		//写入iprh
		saveHeader(pathDst, swathName, index);
	}

	return true;
}

/*
 * Fun:将Data格式的IDS(16位)文件转化为iprh和iprb
 * Param: pathSrc文件存放目录
 *        pathDst iprb文件存放目录
 *        swathFile文件名称 ... ... 例如:Swath001_Array02.data
 *        swathName测线名   ... ... 例如:Swath002
 *        sample每个trace的样本数量
 * 返回:成功返回true，失败返回false
 * Example:Swath001_Array02.data -->Swath001_Array02_A01.iprb、Swath001_A11.iprb ... ... Swath001_A29.iprb
 */
bool TransformIDS::transferSecond(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName)
{
	//参数合法性判断
	if ((NULL == pathSrc) || (NULL == pathDst) || (NULL == swathFile) || (NULL == swathName))
		return false;
	if ((0 >= strlen(swathFile)) || (0 >= strlen(swathName)))
		return false;

	//600M hz的时间窗是80ns
	timeWindow = 80;

	//trace数量计数器
	lastTrace = 0;
	//输入的ids文件
	char pathIDSFile[1024] = { 0 };
	sprintf(pathIDSFile, "%s\\%s", pathSrc, swathFile);
	FILE *fpSrc = fopen(pathIDSFile, "rb");
	if (!fpSrc)
	{
		printf("原文件(%s)不存在", pathIDSFile);
		return false;
	}

	char pathDstFile[1024] = { 0 };
	FILE *fpDst[30] = { NULL };
	for (int index = 11; index <= 29; index++)
	{
		//打开输出的文件
		sprintf(pathDstFile, "%s\\%s_A%02d.iprb", pathDst, swathName, index);
		fpDst[index] = fopen(pathDstFile, "wb");
	}

	int traceSize = samples * 2;
	char *szBuff = (char *)malloc(traceSize);

	//跳过36字节头信息
	fread(szBuff, 1, 36, fpSrc);

	while (!feof(fpSrc))
	{
		int buffSizeOld = 0;
		int buffSize = 0;         //记录读取/写入每个Trace时以及读取的总字节数字
		char *tmpStart = szBuff;  //记录读取/写入每个Trace时的开头位置

		buffSize = 0;
		//读取第11个通道的数据
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第11个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[11]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[11]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第12个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第12个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[12]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[12]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第13个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第13个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[13]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[13]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第14个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第14个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[14]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[14]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第15个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第15个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[15]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[15]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第16个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第16个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[16]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[16]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第17个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第17个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[17]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[17]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第18个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第18个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[18]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[18]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第19个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第19个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[19]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[19]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}


		//读取第20个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第20个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[20]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[20]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第21个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第21个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[21]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[21]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第22个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第22个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[22]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[22]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第23个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第23个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[23]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[23]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第24个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第24个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[24]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[24]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第25个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第25个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[25]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[25]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第26个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第26个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[26]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[26]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第27个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第27个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[27]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[27]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第28个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第28个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[28]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[28]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//读取第29个通道的数据
		buffSize = 0;
		//从原始文件读取一个Trace
		while (buffSize < traceSize)
		{
			int tmpLen = (int)fread(szBuff + buffSize, 1, traceSize - buffSize, fpSrc);
			buffSize = buffSize + tmpLen;

			if (feof(fpSrc))
				break;
		}
		//写入第29个通道的数据
		buffSizeOld = buffSize;
		tmpStart = szBuff;
		while (buffSize > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSize, fpDst[29]);
			buffSize = buffSize - tmpLen;
			tmpStart = szBuff + tmpLen;
		}
		//再次写入一次，因为11-29通道的数据是1-10的一半
		tmpStart = szBuff;
		while (buffSizeOld > 0)
		{
			int tmpLen = (int)fwrite(tmpStart, 1, buffSizeOld, fpDst[29]);
			buffSizeOld = buffSizeOld - tmpLen;
			tmpStart = szBuff + tmpLen;
		}

		//测线数量计数
		lastTrace++;
	}

	free(szBuff);

	//关闭
	fclose(fpSrc);
	for (int index = 11; index <= 29; index++)
	{
		fclose(fpDst[index]);
	}

	//记录到Trace数量
	lastTrace--;
	lastTrace = lastTrace * 2;

	//写入iprh头文件
	for (int index = 11; index <= 29; index++)
	{
		//写入iprh
		saveHeader(pathDst, swathName, index);
	}

	return true;
}


/*
 * Fun:转化data扩展名（Swath001_Array01.data、Swath001_Array01.index、Swath001.swath、Survey）的数据为iprh/iprb，转化一个IDS测线为iprh/iprb
 * Param: pathSrc文件存放目录
 *        pathDst iprb文件存放目录
 *        swathName 测线名
 *        swathID   测线ID
 * 返回:成功返回true，失败返回false
 */
bool TransformIDS::transferData16(const char * pathSrc, const char * pathDst, const char *swathName, int swathID)
{
	char swathFile[128] = { 0 };
	//Survey文件，包含路径
	sprintf(swathFile, "%s/Survey", pathSrc);
	//在雷达目录下，读取指定文件(Survey)，并且将信息写入成员变量头
	getHeaderInfo(swathFile);

	/* ---------------------Survey和Array01处理交界--------------------*/
	//处理雷达目录下，指定文件(第一个IDS数据文件)
	sprintf(swathFile, "%s_Array01.data", swathName);
	//这里第一个data文件转化为iprb文件
	if (false == transferFirst(pathSrc, pathDst, swathFile, swathName))
	{
		printf("转化第一个测线文件出错\n");
		return false;
	}

	/* --------------------Array01和Array02处理交界--------------------*/
	//处理雷达目录下，指定文件(第二个IDS数据文件)
	sprintf(swathFile, "%s_Array02.data", swathName);
	//这里第一个data文件转化为iprb文件
	if (false == transferSecond(pathSrc, pathDst, swathFile, swathName))
	{
		printf("转化第二个测线文件出错\n");
		return false;
	}

	/* --------------------生成cor文件--------------------*/
	//生成cor文件
	if (false == makeFileCor(pathDst, swathName, swathID))
	{
		printf("产生cor文件出错\n");
		return false;
	}

	/* --------------------生成time文件-------------------*/
	if (false == makeFileTime(pathSrc, pathDst, swathName, swathID))
	{
		printf("产生time文件出错\n");
		return false;
	}

	return true;
}

/*
 * Fun：将指定目录下的ids数据（Swath001_Array01.data、Swath001_Array01.index、Swath001.swath、Survey）转化为IPRH、IPRD，存储到指定目录下
 * Param：
 *      pathIDS     IDS数据文件（Swath001_Array01.data）
 *      pathDst     IPRH、IPRD目标目录
 * Return：成功返回TRUE，失败返回FALSE
 */
int TransformIDS::transformIDS16(const char * pathIDS, const char * pathDst)
{
	int swathID = 1;
	char swathName[128] = { 0 };
	//搜索雷达目录下，指定文件(第一个IDS数据文件)
	char szPathTmp[512] = { 0 };
	sprintf(szPathTmp, "%s\\*.swath", pathIDS);
	intptr_t hFile = 0;
	struct _finddata_t oFileInfo;

	//查询是否存在第一条测线
	if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
	{
		printf("没有发现IDS的测线数据（*.swath）\n");
		return -1;
	}


	do
	{
		//获取第一个文件的文件名
		sprintf(szPathTmp, "%s\\%s", pathIDS, oFileInfo.name);

		//得到测线名
		strncpy(swathName, oFileInfo.name, strlen(oFileInfo.name) - 6);

		//将IDS data文件转化为iprh、iprb
		bool result = transferData16(pathIDS, pathDst, swathName, swathID);
		if (true != result)
		{
			//关闭文件搜索
			_findclose(hFile);
			return -2;
		}

		swathID++;
	} while (_findnext(hFile, &oFileInfo) == 0);

	//关闭文件搜索
	if (0 != hFile)
		_findclose(hFile);

	return 0;
}





/*
* Fun：将指定目录下的ids数据（*.scan）转化为IPRH、IPRD，存储到指定目录下
* Param：
*      pathIDS     IDS数据目录
*      pathDst     IPRH、IPRD目标目录
*      separation  通道间隔
* Return：成功返回TRUE，失败返回FALSE
*/
int TransformIDS::transformIDS(const char * pathIDS, const char * pathDst, float separation)
{
	//测线ID
	int swathID = 1;
	IDSSwath idsSwath;
	idsSwath.setID(swathID);    //第一条测线ID固定为1


	//搜索雷达目录下，指定xml文件,获取头信息
	char szPathXML[512] = { 0 };
	sprintf(szPathXML, "%s\\*.xml", pathIDS );
	intptr_t hFileXML;
	struct _finddata_t oFileInfoXML;
	if ((hFileXML = (intptr_t)_findfirst(szPathXML, &oFileInfoXML)) != -1L)
	{
		char szPathTmp[512] = { 0 };
		//获取第一个文件的文件名
		sprintf(szPathTmp, "%s\\%s", pathIDS, oFileInfoXML.name);

		//找到XML的情况下，读取头信息
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
		//搜索雷达目录下，指定文件,IDS数据           //64位参考：  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathTmp[512] = { 0 };
		sprintf(szPathTmp, "%s\\RAW_%06d.scan", pathIDS, i);
		intptr_t hFile;
		struct _finddata_t oFileInfo;
		if ((hFile = (intptr_t)_findfirst(szPathTmp, &oFileInfo)) == -1L)
		{
			//取得测线的所有数据
			std::map<long, IDSChannel*> * lstData = idsSwath.getData();

			//产生cor文件
			CreateCor(pathDst, "Swath", swathID);

			//产生time文件
			CreateTime(pathDst, "Swath", swathID);

			//将idsSwath中的IDS测线转化为IPRH、IPRB测线
			IDSTransferData(lstData, swathID, (char *)pathDst, separation);

			//测线号增加一
			swathID++;
			//清除已经完成的数据
			idsSwath.swathDataClear();
			idsSwath.setID(swathID);

			_findclose(hFile);
			printf("没有发现IDS的测线数据\n");
			break;
		}

		//使用ids文件初始化ids测线片段
		IDSSwathFragment idsSwathFragment;
		int iResult = idsSwathFragment.init(szPathTmp);
		if (0 != iResult)
		{
			_findclose(hFile);
			printf("解析IDS的测线数据出错\n");
			break;
		}

		//文件没有数据
		if (idsSwathFragment.getID() == 0)
		{
			_findclose(hFile);
			printf("测线ID错误\n");
			break;
		}

		//如果测线片段中的trace数量为0，则不需要处理
		if (0 == idsSwathFragment.getTraceCount())
		{
			_findclose(hFile);
			printf("文件中的trace数量为0\n");
			i++;
			continue;
		}

		//如果最新的测线和上次的测线ID不一样，说明是不同的测线，需要将上次完整的测线数据转存为iprh、iprd格式
		if (idsSwathFragment.getID() != swathID)
		{
			//取得测线的所有数据
			std::map<long, IDSChannel*> * lstData = idsSwath.getData();

			//产生cor文件
			CreateCor(pathDst, "Swath", swathID);

			//产生time文件
			CreateTime(pathDst, "Swath", swathID);

			//将idsSwath中的IDS测线转化为IPRH、IPRB测线
			IDSTransferData(lstData, swathID, (char *)pathDst, separation);

			//测线号增加一
			swathID++;
			//清除已经完成的数据
			idsSwath.swathDataClear();
			idsSwath.setID(swathID);
		}
		//取得新增测线数据
		std::map<long, IDSChannel*> * swathData = idsSwathFragment.getDataList();

		//如果测线号和上次一致，那么统一添加到工程中
		idsSwath.swathDataAdd(swathData);

		//关闭搜索器
		_findclose(hFile);
		i++;
	}

	return 0;
}

//将一个IDS的原始数据(整个工程)转化为IPRH、IPRB
int TransformIDS::IDSTransferData(std::map<long, IDSChannel*> * lstData, int swathID, char *swathPathDst, float separation )
{
	if (NULL == lstData)
	{
		//参数错误
		return -1;
	}
	if (0 == lstData->size())
	{
		//数据列表中，没有数据
		return 0;
	}

	//遍历所有通道数据
	for (std::map<long, IDSChannel*>::iterator iter = lstData->begin(); iter != lstData->end(); iter++)
	{
		IDSChannel* p = iter->second;
		if (!p)
			continue;

		//获取频率和道间距
		frequency    = channelFreq[iter->first];
		intervalDist = traceStep[iter->first];
		samples      = channelSample[iter->first];

		//通道数据
		IDSChannelHeader *header = p->getHeader();
		IDSChannelBlob   *blob = p->getBlob();

		//对数据进行非空判断，做一些处理保护
		if ((!header) || (!blob))
			continue;

		//取得trace数量
		int lastTrace = header->GetTraceCount();

		//存储的目标文件
		char szSwathFileHeader[512] = { 0 };
		char szSwathFileBlob[512] = { 0 };
		sprintf(szSwathFileHeader, "%s\\Swath_%04d_A%02d.iprh", swathPathDst, swathID, p->getID());
		sprintf(szSwathFileBlob, "%s\\Swath_%04d_A%02d.iprb", swathPathDst, swathID, p->getID());

		//打开目标文件
		FILE *fpHeader = fopen(szSwathFileHeader, "w+");
		FILE *fpBlob = fopen(szSwathFileBlob, "wb+");

		//转存到目标文件---Header文件
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

		//转存到目标文件---Blob文件，获取TraceList
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

//从Survey中获取头信息
bool TransformIDS::getHeaderInfo(const char * fileSurvey)
{
	if (!fileSurvey)
		return false;

	FILE *fp = fopen(fileSurvey, "r");

	char *buff = (char *)malloc( 1024*30 );
	char *tmp = buff;
	//读取文件所有信息
	while ( true )
	{
		int iLen = (int)fread(tmp, 1, 20, fp);
		if (iLen <= 0)
			break;

		tmp = tmp + iLen;
	}

	int channID = 0;
	//从头开始查找Channel的EncoderStack
	tmp = buff;
	while ( tmp )
	{
		//1、读取ChannelID
		char *tmp1 = strstr(tmp, "<Channel Id=\"");
		if (!tmp1)
			break;
		tmp1 = tmp1 + 13;
		tmp = tmp1;
		char *tmp2 = strstr(tmp1, "\"");

		char szChannelID[8] = { 0 };
		strncpy(szChannelID, tmp1, tmp2 - tmp1);
		channID = atoi(szChannelID);

		//2、读取DadID
		tmp1 = strstr(tmp, "DadId=\"");
		if (!tmp1)
			break;
		tmp2 = strstr(tmp1 + 8, "\"");
		char szDadID[32] = { 0 };
		strncpy(szDadID, tmp1, tmp2 - tmp1 + 1);

		//3、通过DadID，读取天线类型
		char *antennaType = strstr(buff, szDadID);
		while ('=' != *antennaType)
		{
			antennaType--;
		}
		char szAntennaType[32] = { 0 };
		strncpy(szAntennaType, antennaType+2, 7);

		//根据天线类型，查找天线频率
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

		//========读取EncoderStack Begin=======//
		char szEncoderStack[8] = { 0 };
		tmp1 = strstr(buff, "<EncoderStack value=");
		//如果不存在客户指定的EncoderStack，那么读取通道自己的EncoderStack
		if (!tmp1)
		{
			//读取EncoderStack
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
		//========读取EncoderStack  End=======//

		//读取Sample
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
	//光栅距离    //光栅距离记录到成员变量，用于后续转化后的iprh文件
	separation = (float)atof(tmp1);

	//查找Custom定义的XStep，如果存在，则使用Custom定义
	tmp2++;
	tmp1 = strstr(tmp2, "<XStep value=\"");
	if (tmp1)
	{
		tmp1 = tmp1 + 14;
		tmp2 = strstr(tmp1, "\"");
		*tmp2 = 0;
		//光栅距离    //光栅距离记录到成员变量，用于后续转化后的iprh文件
		separation = (float)atof(tmp1);
	}
	for (; channID >= 1; channID--)
	{
		traceStep[channID] = (float)(traceStep[channID] * separation);
	}

	return true;
}

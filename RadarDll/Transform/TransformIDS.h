/*
* Fun:将Segy数据转化为iprb、iprh格式
*/
#include "TransformBase.h"

#pragma once
class TransformIDS:public TransformBase
{
public:
	TransformIDS();
	~TransformIDS();

public:
	int transformIDS08(const char * pathIDS, const char * pathDst);
	int transformIDS16(const char * pathIDS, const char * pathDst);
	int transformIDS(const char * pathIDS, const char * pathDst, float separation);

	void setSample(int iSample) {
		samples = iSample;
	}
private:
	bool transferData16(const char * pathSrc, const char * pathDst, const char *swathName, int swathID);
	bool transferFirst(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName);
	bool transferSecond(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName);
	bool makeFileCor(const char * pathDst, const char *swathName, int swathID);
	bool makeFileTime(const char * pathSrc, const char * pathDst, const char *swathName, int swathID);
	int saveHeader(const char * pathDst, const char *swathName, int channelNum);
	int IDSTransferData(std::map<long, IDSChannel*> * lstData, int swathID, char *swathPathDst, float separation);

	//IDS8位雷达转化
	bool transferData08(const char * pathSrc, const char * pathDst, const char *swathName, int swathID);
	bool transfer08First(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName);
	bool transfer08Second(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName);

	bool getHeaderInfo(const char * fileSurvey );
private:
	//下面一组提供通道属性定义
	int headVersion = 0;        // header version: 10
	int dataVersion = 0;        // data version: 16-16b; 32-32b 
	char date[16] = { 0 };    //测线操作日期
	char startTime[9] = { 0 };    //测线开始时间
	char stopTime[9] = { 0 };    //测线结束时间
	char ANTENNA[10] = { 0 };    //频率？
	double separation = 0;
	int samples = 0;        //一个trace中，采样数量
	int runs = 0;
	int maxStacks = 0;
	double frequency = 0;        // Sampling Frequency (MHz)又是一个频率？
	double timeWindow = 0;        //时间窗，波從發出返回的时间
	int lastTrace = 0;        //一个测线中trace的数量
	char trigSource[16] = { 0 };
	double intervalTime = 0;        // time interval (seconds)
	double intervalDist = 0;        // real distance interval (meters)
	double intervalUser = 0;        // user distance interval (meters)
	int    zeroLevel = 0;
	double soilVel = 0;        // soil velocity (m/us)
	int    positioning = 0;        // it is used for combined files only
	int    channels = 0;
	char   chanName[16] = { 0 };    // channel name (like "T1-R1")

	//下面两个数据用于在解析Survey文件时，临时存储每个通道的道间距和各自的频率值
	float traceStep[64]     = { 0 };
	int   channelFreq[64]   = { 0 };
	int   channelSample[64] = { 0 };
};


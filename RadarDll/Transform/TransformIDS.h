/*
* Fun:��Segy����ת��Ϊiprb��iprh��ʽ
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

	//IDS8λ�״�ת��
	bool transferData08(const char * pathSrc, const char * pathDst, const char *swathName, int swathID);
	bool transfer08First(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName);
	bool transfer08Second(const char * pathSrc, const char * pathDst, const char * swathFile, const char *swathName);

	bool getHeaderInfo(const char * fileSurvey );
private:
	//����һ���ṩͨ�����Զ���
	int headVersion = 0;        // header version: 10
	int dataVersion = 0;        // data version: 16-16b; 32-32b 
	char date[16] = { 0 };    //���߲�������
	char startTime[9] = { 0 };    //���߿�ʼʱ��
	char stopTime[9] = { 0 };    //���߽���ʱ��
	char ANTENNA[10] = { 0 };    //Ƶ�ʣ�
	double separation = 0;
	int samples = 0;        //һ��trace�У���������
	int runs = 0;
	int maxStacks = 0;
	double frequency = 0;        // Sampling Frequency (MHz)����һ��Ƶ�ʣ�
	double timeWindow = 0;        //ʱ�䴰�����İl�����ص�ʱ��
	int lastTrace = 0;        //һ��������trace������
	char trigSource[16] = { 0 };
	double intervalTime = 0;        // time interval (seconds)
	double intervalDist = 0;        // real distance interval (meters)
	double intervalUser = 0;        // user distance interval (meters)
	int    zeroLevel = 0;
	double soilVel = 0;        // soil velocity (m/us)
	int    positioning = 0;        // it is used for combined files only
	int    channels = 0;
	char   chanName[16] = { 0 };    // channel name (like "T1-R1")

	//�����������������ڽ���Survey�ļ�ʱ����ʱ�洢ÿ��ͨ���ĵ����͸��Ե�Ƶ��ֵ
	float traceStep[64]     = { 0 };
	int   channelFreq[64]   = { 0 };
	int   channelSample[64] = { 0 };
};


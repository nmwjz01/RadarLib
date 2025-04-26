#pragma once

class IDSChannelHeader
{
public:
	IDSChannelHeader() {};
	~IDSChannelHeader() {};

	int GetTraceCount()
	{
		return m_iTraceCount;
	};
	void SetTraceCount(int count)
	{
		m_iTraceCount = count;
	};

private:

	//����һ���ṩͨ�����Զ���
	int m_iHeadVersion = 0;        // header version: 10
	int m_iDataVersion = 0;        // data version: 16-16b; 32-32b 
	char m_szDate[11] = { 0 };    //���߲�������
	char m_szStartTime[9] = { 0 };    //���߿�ʼʱ��
	char m_szStopTime[9] = { 0 };    //���߽���ʱ��
	char m_szANTENNA[10] = { 0 };    //Ƶ�ʣ�
	int m_iSeparation = 0;
	int m_iSamples = 0;        //һ��trace�У���������
	int m_iRuns = 0;
	int m_iMaxStacks = 0;
	double m_dFrequency = 0;        // Sampling Frequency (MHz)����һ��Ƶ�ʣ�
	double m_dTimeWindow = 0;        //ʱ�䴰�����İl�����ص�ʱ��
	int m_iTraceCount = 0;        //һ��������trace������
	char m_szTrigSource[16] = { 0 };
	double m_dIntervalTime = 0;        // time interval (seconds)
	double m_dIntervalDist = 0;        // real distance interval (meters)
	double m_dIntervalUser = 0;        // user distance interval (meters)
	int    m_iZeroLevel = 0;
	double m_dSoilVel = 0;        // soil velocity (m/us)
	int    m_iPositioning = 0;        // it is used for combined files only
	int    m_iChannels = 0;
	char   m_szChanName[16] = { 0 };    // channel name (like "T1-R1")

	int    m_iTrigSource = 0;        //trig source


	double m_tgCoef[MAX_SAMPLES];
};

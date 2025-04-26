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

	//下面一组提供通道属性定义
	int m_iHeadVersion = 0;        // header version: 10
	int m_iDataVersion = 0;        // data version: 16-16b; 32-32b 
	char m_szDate[11] = { 0 };    //测线操作日期
	char m_szStartTime[9] = { 0 };    //测线开始时间
	char m_szStopTime[9] = { 0 };    //测线结束时间
	char m_szANTENNA[10] = { 0 };    //频率？
	int m_iSeparation = 0;
	int m_iSamples = 0;        //一个trace中，采样数量
	int m_iRuns = 0;
	int m_iMaxStacks = 0;
	double m_dFrequency = 0;        // Sampling Frequency (MHz)又是一个频率？
	double m_dTimeWindow = 0;        //时间窗，波從發出返回的时间
	int m_iTraceCount = 0;        //一个测线中trace的数量
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

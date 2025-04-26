#pragma once

#include <fstream>
//#include "Utils.h"

//内部使用常量(计算过程中使用)
const int   MAX_SAMPLES  = 2048;
const float CONTRAST     = 1.5F;
const int   TIME_GAIN    = 1;

class SwathChannelHeader
{
public:
	const int TRIG_TIME     = 84;
	const int TRIG_WHEEL    = 87;
	const int TRIG_MANUAL   = 77;
	const int TRIG_EXTERNAL = 69;

public:
	SwathChannelHeader(){}
	~SwathChannelHeader(){}

	//使用通道配置文件，初始化通道的各个属性值
	int init(char* pPathFile)
	{
		//参数合法性校验
		if (nullptr == pPathFile)
			return -1;
		if (0 >= strlen(pPathFile))
			return -1;

		//open 通道配置文件
		std::ifstream ifs;
		ifs.open(pPathFile);
		if (ifs.fail())
			return -2;

		//循环读取各个配置参数
		char s[300];
		while (!ifs.eof())
		{
			ifs.getline(s, 300);
			Utils::ReadParameterInt(s,    (char *)"HEADER VERSION:"        , m_iHeadVersion);
			Utils::ReadParameterInt(s,    (char *)"DATA VERSION:"          , m_iDataVersion);
			Utils::ReadParameterStr(s,    (char *)"DATE:"                  , m_szDate);
			Utils::ReadParameterStr(s,    (char *)"START TIME:"            , m_szStartTime);
			Utils::ReadParameterStr(s,    (char *)"STOP TIME:"             , m_szStopTime);
			Utils::ReadParameterStr(s,    (char *)"ANTENNA:"               , m_szANTENNA );
			Utils::ReadParameterDouble(s, (char *)"ANTENNA SEPARATION:"    , m_dSeparation);
			Utils::ReadParameterInt(s,    (char *)"SAMPLES:"               , m_iSamples);
			Utils::ReadParameterInt(s,    (char *)"RUNS:"                  , m_iRuns);
			Utils::ReadParameterInt(s,    (char *)"MAX STACKS:"            , m_iMaxStacks);
			Utils::ReadParameterDouble(s, (char *)"FREQUENCY:"             , m_dFrequency);
			Utils::ReadParameterDouble(s, (char *)"TIMEWINDOW:"            , m_dTimeWindow);
			Utils::ReadParameterInt(s,    (char *)"LAST TRACE:"            , m_iTraceCount );
			Utils::ReadParameterStr(s,    (char *)"TRIG SOURCE:"           , m_szTrigSource);
			Utils::ReadParameterDouble(s, (char *)"TIME INTERVAL:"         , m_dIntervalTime);
			Utils::ReadParameterDouble(s, (char *)"DISTANCE INTERVAL:"     , m_dIntervalDist);
			Utils::ReadParameterDouble(s, (char *)"USER DISTANCE INTERVAL:", m_dIntervalUser);
			Utils::ReadParameterInt(s,    (char *)"ZERO LEVEL:"            , m_iZeroLevel);
			Utils::ReadParameterDouble(s, (char *)"SOIL VELOCITY:"         , m_dSoilVel);
			Utils::ReadParameterInt(s,    (char *)"POSITIONING:"           , m_iPositioning);
			Utils::ReadParameterInt(s,    (char *)"CHANNELS:"              , m_iChannels);
			Utils::ReadParameterStr(s,    (char *)"CHANNEL CONFIGURATION:" , m_szChanName);
		}
		if( 0 == strncmp( m_szTrigSource , "time" , 4 ) )
			m_iTrigSource = TRIG_TIME;
		else if(0 == strncmp(m_szTrigSource, "wheel", 5))
			m_iTrigSource = TRIG_WHEEL;
		else if (0 == strncmp(m_szTrigSource, "manual", 6))
			m_iTrigSource = TRIG_MANUAL;
		else if (0 == strncmp(m_szTrigSource, "external", 8))
			m_iTrigSource = TRIG_EXTERNAL;
		else
			m_iTrigSource = 0;

		ifs.close();

		//计算图像矫正参数
		setCoef( ( int )CONTRAST * 10 , TIME_GAIN );

		return 0; //OK
	}

	//下面一组对外提供通道信息
	int getDataVersion()
	{
		return m_iDataVersion;
	}
	void setDataVersion(int iVersion)
	{
		m_iDataVersion = iVersion;
	}

	char *getDate()
	{
		return m_szDate;
	}
	void setDate(char *date)
	{
		strncpy(m_szDate, date, 16 );
	}

	//天线频率
	char *getAntenna()
	{
		return m_szANTENNA;
	}
	void setAntenna(const char * szANTENNA)
	{
		strncpy(m_szANTENNA, szANTENNA, 32);
	}

	//天线间隔
	double getAntennaSeparation()
	{
		return m_dSeparation;
	}
	void setAntennaSeparation(double separation)
	{
		m_dSeparation = separation;
	}

	int getSample()
	{
		return m_iSamples;
	}
	void setSample( int iSample)
	{
		m_iSamples = iSample;
	}

	int getChannelCount()
	{
		return m_iChannels;
	}
	void setChannels(int iChannels)
	{
		m_iChannels = iChannels;
	}

	char* getChannelName()
	{
		return m_szChanName;
	}
	void setChannelName( const char * channelName )
	{
		strncpy(m_szChanName, channelName, 16);
	}

	double getFrequency()
	{
		return m_dFrequency;
	}
	void setFrequency(double freq)
	{
		m_dFrequency = freq;
	}

	double getTimeWindow()
	{
		return m_dTimeWindow;
	}
	void setTimeWindow(double timeWindow)
	{
		m_dTimeWindow = timeWindow;
	}

	int getTraceCount()
	{
		return m_iTraceCount;
	}
	void setTraceCount( int iTraceCount )
	{
		m_iTraceCount = iTraceCount;
	}

	char* getStartTime()
	{
		return m_szStartTime;
	}
	void setStartTime( const char * szStartTime)
	{
		strncpy( m_szStartTime, szStartTime , 9);
	}

	char* getStopTime()
	{
		return m_szStopTime;
	}
	void setStopTime(const char * szStopTime)
	{
		strncpy(m_szStopTime, szStopTime, 9);
	}

	double getIntervalTime()
	{
		return m_dIntervalTime;
	}
	void setIntervalTime( double dIntervalDist)
	{
		m_dIntervalTime = dIntervalDist;
	}

	double getIntervalDist()
	{
		return m_dIntervalDist;
	}
	void setIntervalDist(double dIntervalDist)
	{
		m_dIntervalDist = dIntervalDist;
	}

	int getZeroLevel()
	{
		return m_iZeroLevel;
	}
	void setZeroLevel(int iZeroLevel)
	{
		m_iZeroLevel = iZeroLevel;
	}

	double getSoilVel()
	{
		return m_dSoilVel;
	}
	void setSoilVel(double dSoilVel)
	{
		m_dSoilVel = dSoilVel;
	}

	double* getCoef()
	{
		return m_tgCoef;
	}

	//存储iprh头信息
	int saveHeader(const char * pathFile)
	{
		//参数合法性校验
		if (nullptr == pathFile)
			return -1;
		if (0 >= strlen(pathFile))
			return -1;

		FILE * m_pFile = fopen(pathFile, "wt+");
		if (nullptr == m_pFile)
			return -2;

		//循环读取各个配置参数
		char s[300];

		sprintf(s, "HEADER VERSION: %d\n"        , m_iHeadVersion    );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DATA VERSION: %d\n"          , m_iDataVersion    );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DATE: %s\n"                  , m_szDate          );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "START TIME: %s\n"            , m_szStartTime     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "STOP TIME: %s\n"             , m_szStopTime      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA: %s\n"               , m_szANTENNA       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA SEPARATION: %f\n"    , m_dSeparation     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SAMPLES: %d\n"               , m_iSamples        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SIGNAL POSITION: 12\n"                           );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CLIPPED SAMPLES: 0\n"                            );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "RUNS: %d\n"                  , m_iRuns           );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "MAX STACKS: %d\n"            , m_iMaxStacks      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "AUTOSTACKS: 1\n"                                 );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "FREQUENCY: %d\n"             , (int)m_dFrequency );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TIMEWINDOW: %d\n"            , (int)m_dTimeWindow);  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "LAST TRACE: %d\n"            , m_iTraceCount     );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TRIG SOURCE: %s\n"           , m_szTrigSource    );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "TIME INTERVAL: %f\n"         , m_dIntervalTime   );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "DISTANCE INTERVAL: %f\n"     , m_dIntervalDist   );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "USER DISTANCE INTERVAL: %f\n", m_dIntervalUser   );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "STOP POSITION: %f\n"         , 1.0               );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "WHEEL NAME: WENDE\n"                             );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "WHEEL CALIBRATION: 1.0\n"                        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ZERO LEVEL: %d\n"            , m_iZeroLevel      );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SOIL VELOCITY: %f\n"         , m_dSoilVel        );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "PREPROCESSING: 0\n"                              );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "OPERATOR COMMENT: _\n"                           );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA F/W: 48001262\n"                         );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA H/W: 0\n"                                );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA FPGA: WENDE\n"                           );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "ANTENNA SERIAL: 001\n"                           );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "SOFTWARE VERSION: 1.0.0\n"                       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "POSITIONING: %d\n"           , m_iPositioning    );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CHANNELS: %d\n"              , m_iChannels       );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CHANNEL CONFIGURATION: T2 - R2\n"                );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CH_X_OFFSET: 0.000000\n"                         );  fwrite(s, 1, strlen(s), m_pFile);
		sprintf(s, "CH_Y_OFFSET: 0.000000\n"                         );  fwrite(s, 1, strlen(s), m_pFile);
		fclose(m_pFile);

		return 0;
	}
private:
	//下面一组提供通道属性定义
	int m_iHeadVersion      = 0;        // header version: 10
	int m_iDataVersion      = 0;        // data version: 16-16b; 32-32b 
	char m_szDate[16]       = { 0 };    //测线操作日期
	char m_szStartTime[9]   = { 0 };    //测线开始时间
	char m_szStopTime[9]    = { 0 };    //测线结束时间
	char m_szANTENNA[32]    = { 0 };    //频率？
	double m_dSeparation    = 0;
	int m_iSamples          = 0;        //一个trace中，采样数量
	int m_iRuns             = 0;
	int m_iMaxStacks        = 0;
	double m_dFrequency     = 0;        // Sampling Frequency (MHz)又是一个频率？
	double m_dTimeWindow    = 0;        //时间窗，波從發出返回的时间
	int m_iTraceCount       = 0;        //一个测线中trace的数量
	char m_szTrigSource[16] = { 0 };
	double m_dIntervalTime  = 0;        // time interval (seconds)
	double m_dIntervalDist  = 0;        // real distance interval (meters)
	double m_dIntervalUser  = 0;        // user distance interval (meters)
	int    m_iZeroLevel     = 0;
	double m_dSoilVel       = 0;        // soil velocity (m/us)
	int    m_iPositioning   = 0;        // it is used for combined files only
	int    m_iChannels      = 0;
	char   m_szChanName[16] = { 0 };    // channel name (like "T1-R1")

	int    m_iTrigSource    = 0;        //trig source

	double m_tgCoef[MAX_SAMPLES];

	unsigned int m_GainEx = 0;
	unsigned int m_Deep   = 1;
public:

	//计算图像矫正参数
	void setCoef( int iContrast , int iTimeGain )
	{
		double tgVal = 1.0;
		double linTG = iTimeGain / 1000.0;

		//实际操作过程中，在去除背景噪声后，地表缺陷的显示不够明显，此处的目的是加强地表的对比度
		for (int j = 0; j < m_iSamples; j++)
		{
			m_tgCoef[j] = tgVal * iContrast / 10;
			//if (j > m_iZeroLevel)    //之所以去掉这一行，是因为所有的处理，都要在切除直达波之后进行
				tgVal += linTG;
		}
	}
};

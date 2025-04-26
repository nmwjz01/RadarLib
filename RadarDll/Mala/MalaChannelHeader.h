#pragma once

class MalaChannelHeader
{
public:
	MalaChannelHeader() {};
	~MalaChannelHeader() {};

	//样本数量
	int GetSample()
	{
		return m_iSamples;
	};
	void SetSample(int sample)
	{
		m_iSamples = sample;
	};

	//采样频率
	double GetFrequency()
	{
		return m_dFrequency;
	}
	void SetFrequency(double freq)
	{
		m_dFrequency = freq;
	}

	//天线频率
	char * GetAntennas()
	{
		return m_szAntennas;
	}
	void SetAntennas(const char* szAntennas)
	{
		strncpy( m_szAntennas, szAntennas, 10 );
	}

	//天线间隔
	double GetAntennaSeparation()
	{
		return m_dAntennas;
	}
	void SetAntennaSeparation(double separation)
	{
		m_dAntennas = separation;
	}

	//时间窗
	double GetTimeWindow()
	{
		return m_dTimeWindow;
	}
	void SetTimeWindow( double timeWindow )
	{
		m_dTimeWindow = timeWindow;
	}

	//Trace数量
	int GetTraceCount()
	{
		return m_iTraceCount;
	};
	void SetTraceCount(int count)
	{
		m_iTraceCount = count;
	};

	int GetChannelCount()
	{
		return m_iChannels;
	};
	void SetChannelCount( int count )
	{
		m_iChannels = count;
	}

	double GetTimeInterval()
	{
		return m_dTimeInterval;
	}
	void SetTimeInterval(double dTimeInterval)
	{
		m_dTimeInterval = dTimeInterval;
	}

	double GetDistanceInterval()
	{
		return m_dDistanceInterval;
	}
	void SetDistanceInterval(double distanceInterval)
	{
		m_dDistanceInterval = distanceInterval;
	}

	//通过通道头文件信息，初始化通道头对象
	int init( char * pathFile )
	{
		//参数合法性校验
		if (nullptr == pathFile)
			return -1;
		if (0 >= strlen(pathFile))
			return -1;

		//open 通道配置文件
		std::ifstream ifs;
		ifs.open(pathFile);
		if (ifs.fail())
			return -2;

		//循环读取各个配置参数
		char s[300];
		while (!ifs.eof())
		{
			ifs.getline(s, 300);
			Utils::ReadParameterInt(s,    (char *)"SAMPLES:"              , m_iSamples);
			Utils::ReadParameterDouble(s, (char *)"FREQUENCY:"            , m_dFrequency);
			Utils::ReadParameterInt(s,    (char *)"FREQUENCY STEPS:"      , m_iFrequencySteps);
			Utils::ReadParameterInt(s,    (char *)"SIGNAL POSITION:"      , m_iSignalPosition);
			Utils::ReadParameterInt(s,    (char *)"RAW SIGNAL POSITION:"  , m_iRawSignalPosition);
			Utils::ReadParameterInt(s,    (char *)"DISTANCE FLAG:"        , m_iDistanceFlag);
			Utils::ReadParameterInt(s,    (char *)"TIME FLAG:"            , m_iTimeFlag);
			Utils::ReadParameterInt(s,    (char *)"PROGRAM FLAG:"         , m_iProgramFlag);
			Utils::ReadParameterInt(s,    (char *)"EXTERNAL FLAG:"        , m_iExternalFlag);
			Utils::ReadParameterDouble(s, (char *)"TIME INTERVAL:"        , m_dTimeInterval);
			Utils::ReadParameterDouble(s, (char *)"DISTANCE INTERVAL:"    , m_dDistanceInterval);
			Utils::ReadParameterStr(s,    (char *)"OPERATOR:"             , m_szOperator);
			Utils::ReadParameterStr(s,    (char *)"CUSTOMER:"             , m_szCustomer);
			Utils::ReadParameterStr(s,    (char *)"SITE:"                 , m_szSite);
			Utils::ReadParameterStr(s,    (char *)"ANTENNAS"              , m_szAntennas);
			Utils::ReadParameterStr(s,    (char *)"ANTENNA ORIENTATION:"  , m_szAntennas2);
			Utils::ReadParameterDouble(s, (char *)"ANTENNA SEPARATION:"   , m_dAntennas  );
			Utils::ReadParameterStr(s,    (char *)"COMMENT:"              , m_szComment);
			Utils::ReadParameterDouble(s, (char *)"TIMEWINDOW:"           , m_dTimeWindow);
			Utils::ReadParameterInt(s,    (char *)"STACKS:"               , m_iStacks);
			Utils::ReadParameterInt(s,    (char *)"STACK EXPONENT:"       , m_iStackExponent);
			Utils::ReadParameterDouble(s, (char *)"STACKING TIME:"        , m_dStackTime);
			Utils::ReadParameterInt(s,    (char *)"LAST TRACE:"           , m_iTraceCount);
			Utils::ReadParameterDouble(s, (char *)"STOP POSITION:"        , m_dStopPosition );
			Utils::ReadParameterDouble(s, (char *)"SYSTEM CALIBRATION:"   , m_dSysCalibration);
			Utils::ReadParameterDouble(s, (char *)"START POSITION:"       , m_dStartPosition);
			Utils::ReadParameterInt(s,    (char *)"SHORT FLAG:"           , m_iShartFlag);
			Utils::ReadParameterInt(s,    (char *)"INTERMEDIATE FLAG:"    , m_iInterMediateFag);
			Utils::ReadParameterInt(s,    (char *)"LONG FLAG:"            , m_iLongFlag);
			Utils::ReadParameterInt(s,    (char *)"PREPROCESSING:"        , m_iPrepocessing);
			Utils::ReadParameterInt(s,    (char *)"HIGH:"                 , m_iHigh);
			Utils::ReadParameterInt(s,    (char *)"LOW:"                  , m_iLow);
			Utils::ReadParameterDouble(s, (char *)"FIXED INCREMENT:"      , m_dFixedIncrement );
			Utils::ReadParameterInt(s,    (char *)"FIXED MOVES UP:"       , m_iFixMoveUp);
			Utils::ReadParameterInt(s,    (char *)"FIXED MOVES DOWN:"     , m_iFixMoveDown);
			Utils::ReadParameterDouble(s, (char *)"FIXED POSITION:"       , m_dFixedPosition);
			Utils::ReadParameterDouble(s, (char *)"WHEEL CALIBRATION:"    , m_WheelCalibration);
			Utils::ReadParameterInt(s,    (char *)"POSITIVE DIRECTION:"   , m_iPositiveDirection);
			Utils::ReadParameterInt(s,   (char *)"CHANNELS:"              , m_iChannels);
			Utils::ReadParameterStr(s,   (char *)"CHANNEL CONFIGURATION:" , m_szChanConf);
		}

		ifs.close();

		return 0; //OK
	}
private:

	//下面一组提供通道属性定义
	int m_iSamples            = 0;      //一个trace中，采样数量
	double m_dFrequency       = 0;      //Sampling Frequency (MHz)又是一个频率？
	int m_iFrequencySteps     = 0;      //FREQUENCY STEPS: 59
	int m_iSignalPosition     = 0;      //SIGNAL POSITION:52981
	int m_iRawSignalPosition  = 0;      //RAW SIGNAL POSITION : 52981
	int m_iDistanceFlag       = 0;      //DISTANCE FLAG:1
	int m_iTimeFlag           = 0;      //TIME FLAG:0
	int m_iProgramFlag        = 0;      //PROGRAM FLAG:0
	int m_iExternalFlag       = 0;      //EXTERNAL FLAG:0
	double m_dTimeInterval    = 0;      //TIME INTERVAL: 0.000000
	double m_dDistanceInterval= 0;      //DISTANCE INTERVAL: 0.014347
	char m_szOperator[32]     = {0};    //OPERATOR: 
	char m_szCustomer[32]     = {0};    //CUSTOMER: 
	char m_szSite[32]         = {0};    //SITE: 
	char m_szAntennas[32]     = {0};    //ANTENNAS:400 MHz shielded
	char m_szAntennas2[32]    = {0};    //ANTENNA ORIENTATION:NOT VALID FIELD
	double m_dAntennas        = 0;      //ANTENNA SEPARATION : 0.180000
	char m_szComment[32]      = {0};    //COMMENT :
	double m_dTimeWindow      = 0;      //时间窗，波從發出返回的时间
	int m_iStacks             = 0;      //STACKS:2
	int m_iStackExponent      = 0;      //STACK EXPONENT:1
	double m_dStackTime       = 0;      //STACKING TIME:0.048000
	int m_iTraceCount         = 0;      //一个测线中trace的数量
	double m_dStopPosition    = 0;      //STOP POSITION:280.387374
	double m_dSysCalibration  = 0;      //SYSTEM CALIBRATION:0.0000022007
	double m_dStartPosition   = 0;      //START POSITION:0.000000
	int m_iShartFlag          = 0;      //SHORT FLAG:1
	int m_iInterMediateFag    = 0;      //INTERMEDIATE FLAG:0
	int m_iLongFlag           = 0;      //LONG FLAG:0
	int m_iPrepocessing       = 0;      //PREPROCESSING:0
	int m_iHigh               = 0;      //HIGH:0
	int m_iLow                = 0;      //LOW:0
	double m_dFixedIncrement  = 0;      //FIXED INCREMENT:0.300000
	int m_iFixMoveUp          = 0;      //FIXED MOVES UP:0
	int m_iFixMoveDown        = 1;      //FIXED MOVES DOWN:1
	double m_dFixedPosition   = 0;      //FIXED POSITION:0.000000
	double m_WheelCalibration = 0;      //WHEEL CALIBRATION:139.4000000000
	int m_iPositiveDirection  = 1;      //POSITIVE DIRECTION:1
	int m_iChannels           = 0;      //CHANNELS:16
	char m_szChanConf[32]     = {0};    //CHANNEL CONFIGURATION:T1 - R1
};

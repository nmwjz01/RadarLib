#pragma once
#include <cstring>

//240 byte + length of data
//TraceHeader + TraceData
class SegyTrace
{
public:
	SegyTrace() { traceData = NULL; };
	~SegyTrace()
	{
		if (traceData)
		{
			free(traceData);
			traceData = NULL;
		}
	};

	void setHeaderBuff(unsigned char *pHeaderBuff)
	{
		memcpy(traceHeader, pHeaderBuff, 240);

		iChannelNum = ((pHeaderBuff[4] * 265 + pHeaderBuff[5]) * 256 + pHeaderBuff[6]) * 256 + pHeaderBuff[7];

		iTraceNum   = ((pHeaderBuff[20] * 265 + pHeaderBuff[21]) * 256 + pHeaderBuff[22]) * 256 + pHeaderBuff[23];
	}
	unsigned char *getHeaderBuff()
	{
		return traceHeader;
	}

	void setData(void *traceData)
	{
		if (this->traceData)
			free(this->traceData);
		this->traceData = traceData;
	}
	void *getData()
	{
		return traceData;
	}

	void setTraceNum(int iNum)
	{
		iTraceNum = iNum;
	}
	int getTraceNum()
	{
		return iTraceNum;
	}

	void setChannelNum(int iNum)
	{
		iChannelNum = iNum;
	}
	int getChannelNum()
	{
		return iChannelNum;
	}
private:
	int iSwathNum;
	int iTraceNum;
	int iChannelNum;

	unsigned char traceHeader[240];
	void *traceData;
};


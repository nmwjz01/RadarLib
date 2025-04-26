#pragma once

class MalaTrace32
{
public:
	MalaTrace32()
	{
		m_pData = NULL;
	}
	~MalaTrace32()
	{
		if (NULL != m_pData)
		{
			free(m_pData);
			m_pData = NULL;
		}
	}

	void setTrace(long* pData, int iSample)
	{
		m_iSamples = iSample;
		m_pData    = pData;
	}

	void setTraceNum(int num)
	{
		m_iTraceNum = num;
	};
	int getTraceNum()
	{
		return m_iTraceNum;
	};

	long* getTrace()
	{
		return m_pData;
	}
	int getSamples()
	{
		return m_iSamples;
	}
	void setSamples(int sample)
	{
		m_iSamples = sample;
	};
	//切去开头的直达波
	void cutSamples(int iZero)
	{
		memcpy(m_pData, m_pData + iZero, m_iSamples - iZero);
		m_iSamples = m_iSamples - iZero;
	}
private:
	long* m_pData;
	int   m_iSamples;
	int   m_iTraceNum;
};

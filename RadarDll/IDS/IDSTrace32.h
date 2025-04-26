#pragma once

class IDSTrace32
{
public:
	IDSTrace32() {}
	~IDSTrace32() {}

	void setTrace32(short* pData, int iSample)
	{
		m_iSamples = iSample;
		m_pData = pData;
	}

	short* getTrace32()
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
	short* m_pData;
	int    m_iSamples;
};

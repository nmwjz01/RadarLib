#pragma once

class Trace16
{
public:
	Trace16()
	{
		m_pData = NULL;
		m_iSamples = 0;
	}
	~Trace16()
	{
		if (m_pData)
		{
			free(m_pData);
			m_pData = NULL;
		}
		m_iSamples = 0;
	}

	void setTrace(short* pData , int iSample)
	{
		if (m_pData)
		{
			free(m_pData);
			m_pData = NULL;
		}

		m_iSamples = iSample;
		m_pData = pData;
	}

	short* getTrace()
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
		memcpy(m_pData, m_pData + iZero, 2*(m_iSamples - iZero));
		m_iSamples = m_iSamples - iZero;
	}

	//叠加逆振幅操作--叠加偏移
	void GainInvDecayConst(int *coef)
	{
		for (int i = 0; i < m_iSamples; i++)
		{
			m_pData[i] = m_pData[i] + coef[i];
		}
	}
	//叠加逆振幅操作--乘以系数
	void GainInvDecayCoef(int *coef)
	{
		for (int i = 0; i < m_iSamples; i++)
		{
			m_pData[i] = (short)(m_pData[i] * (1 + (float)coef[i] / (float)100));
		}
	}
	//叠加逆振幅操作--叠加正线性放大部分
	void GainInvDecayCurve(int k, int n)
	{
		for (int i = 0; i < m_iSamples; i++)
		{
			//m_pData[i] = (short)(m_pData[i] + (m_pData[i] * (float)(k) / (float)100) * ( pow(i,n/9) /m_iSamples ) );
			m_pData[i] = (short)(m_pData[i] + (m_pData[i] * (float)(k) / (float)100) * (pow(i, n / 10) / m_iSamples));
		}
	}

private:
	short* m_pData;
	int    m_iSamples;
	int    m_iTraceNum;
};


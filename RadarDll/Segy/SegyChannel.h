#pragma once
#include <map>

#include "SegyTrace.h"

class SegyChannel
{
public:
	SegyChannel() {};

	void addTrace(SegyTrace* pTrace)
	{
		if (!pTrace)
			return;

		m_lstData.insert(std::map<int, SegyTrace*>::value_type(pTrace->getTraceNum(), pTrace));
	}
	int getTraceCount()
	{
		return (int)m_lstData.size();
	}
	std::map<int, SegyTrace*>*getTraceList()
	{
		return &m_lstData;
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
	std::map<int, SegyTrace*> m_lstData;

	int iChannelNum;
};


#pragma once
#include <cstring>

//400 byte
class SegyFileHeaderBinary
{
public:
	SegyFileHeaderBinary()	{};

	void setBuff( unsigned char *pBuff )
	{
		memcpy(fileHeader, pBuff, 400);

		iSamples = pBuff[20] * 256 + pBuff[21];
		if( 3 == pBuff[25] )
			iBitType = 16;
		else
			iBitType = 32;
	}
	unsigned char *getBuff()
	{
		return fileHeader;
	}

	void setSamples( int iSamples)
	{
		this->iSamples = iSamples;
	}
	int getSamples()
	{
		return iSamples;
	}

	void setBitType(int iBitType)
	{
		this->iBitType = iBitType;
	}
	int getBitType()
	{
		return iBitType;
	}
private:
	unsigned char fileHeader[400];

	int iSamples;    //采样点数
	int iBitType;    //每个点的位数
};

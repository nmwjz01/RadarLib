/*
* Fun:��Segy����ת��Ϊiprb��iprh��ʽ
*/
#include "TransformBase.h"

#pragma once
class Transform3DRadar:public TransformBase
{
public:
	int transform3DRadar(const char * swathPath3DRadar, const char * pathDst);

private:
	int transform3DRadarSwath(const char * swathPath3DRadar, const char * pathDst, const char * swathName);
	int getLastTraceByTxt(char * lineStart);
	int getInLinesByTxt(char * lineStart);
	int getSamplesByTxt(char * lineStart);
};


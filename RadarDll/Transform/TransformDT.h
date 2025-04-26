/*
* Fun:将Segy数据转化为iprb、iprh格式
*/
#include "TransformBase.h"

#pragma once
class TransformDT:public TransformBase
{
public:
	int dt2iprb(char *szDT, char *szIPRB, int height, int width);
	int dt2iprh(char *szDTH, int freq, int ns, float distance, float separation, char *szIPRH, int &tracecount);

private:
	;
};


/*
* Fun:��Segy����ת��Ϊiprb��iprh��ʽ
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


/*
* Fun:��Segy����ת��Ϊiprb��iprh��ʽ
*/
#include "TransformBase.h"

#pragma once
class TransformSegy:public TransformBase
{
public:
	int transformSegy(const char * swathPathSegy, const char * pathDst);

private:
	int transformSegySwath(const char *szPathSegy, const char *swathName, const char *pathDst, int swathIndex);
};


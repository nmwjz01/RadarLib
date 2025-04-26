/*
* Fun:将Segy数据转化为iprb、iprh格式
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


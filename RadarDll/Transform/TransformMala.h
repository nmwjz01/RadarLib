/*
* Fun:将Segy数据转化为iprb、iprh格式
*/
#include "TransformBase.h"

#pragma once
class TransformMala:public TransformBase
{
public:
	int transformMala32(const char * pathMala, const char * pathDst);
	int transformMala32Ex(const char * swathPathSegy, const char * pathDst);

	int transformMala16(const char * pathMala, const char * pathDst);
	int transformMala16Ex(const char * pathMala, const char * pathDst);

private:
	int ChannelMala2Implus(std::map<long, MalaChannel*> * lstDataMala, std::map<long, SwathChannel*> * lstDataImplus);
	int TimeMala2Implus(SwathTime *timeInfoImpluse, MalaTime *timeInfoMala);

};


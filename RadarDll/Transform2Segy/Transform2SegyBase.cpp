
#include <io.h>
#include <atlimage.h>

#include "..\\Utils\\PalSET.h"
#include "..\\Utils\\Utils.h"
#include "..\\Utils\\RadarConst.h"

#include "..\\Impluse\\ImpluseTrace16.h"
#include "..\\Impluse\\ImpluseTrace32.h"
#include "..\\Impluse\\ImpluseCor.h"
#include "..\\Impluse\\ImpluseTime.h"
#include "..\\Impluse\\ImpluseChannelHeader.h"
#include "..\\Impluse\\ImpluseChannelBlob.h"
#include "..\\Impluse\\ImpluseChannel.h"
#include "..\\Impluse\\ImpluseSwath.h"

#include "..\\Segy\\SegySwathFile.h"

#include "Transform2SegyBase.h"

//构造函数
Transform2SegyBase::Transform2SegyBase()
{
}
//析构函数
Transform2SegyBase::~Transform2SegyBase()
{
}

/*
 * Fun:将Segy文件分解为单通道格式，及一个通道一个Segy文件
 * Param:
 *    dstPath,     目标文件存储目录
 *    srcPathFile, 原始文件名（带有路径）
 */
int Transform2SegyBase::SplitChannel(const char *dstPath, const char *srcPathFile)
{
	SegySwathFile *segySwathFileSrc = new SegySwathFile();

	//通过文件加载Segy数据
	int iResult = segySwathFileSrc->loadFile2Data(srcPathFile);
	if (iResult)
		return iResult;

	int iChannelCount = segySwathFileSrc->getChannelCount();
	for (int i = 1; i <= iChannelCount; i++)
	{
		SegySwathFile *segySwathFileDst = new SegySwathFile();
		SegyFileHeaderText *segyFileHeaderTextDst     = new SegyFileHeaderText();
		SegyFileHeaderBinary *segyFileHeaderBinaryDst = new SegyFileHeaderBinary();

		//取得文本文件头和二进制文件头
		SegyFileHeaderText *segyFileHeaderTextSrc     = segySwathFileSrc->getSegyFileHeaderText();
		SegyFileHeaderBinary *segyFileHeaderBinarySrc = segySwathFileSrc->getSegyFileHeaderBinary();
		segyFileHeaderTextDst->setBuff(segyFileHeaderTextSrc->getBuff());
		segyFileHeaderBinaryDst->setBuff(segyFileHeaderBinarySrc->getBuff());

		segySwathFileDst->setHeaderText(segyFileHeaderTextDst);
		segySwathFileDst->setHeaderBinary(segyFileHeaderBinaryDst);

		//将Channel数据复制过去
		SegyChannel *segyChannelSrc = segySwathFileSrc->getChannel(i);
		SegyChannel *segyChannelDst = new SegyChannel();
		std::map<int, SegyTrace*> *m_lst = segyChannelSrc->getTraceList();
		//循环读取每个Trace，然后写过去
		for (std::map<int, SegyTrace*>::iterator iter = m_lst->begin(); iter != m_lst->end(); iter++)
		{
			SegyTrace* p = iter->second;
			segyChannelDst->addTrace(p);
		}

		//设置通道号
		segyChannelDst->setChannelNum(i);
		segySwathFileDst->addChannel(segyChannelDst);

		//设置存储文件名
		char szPathFileDst[512] = { 0 };
		sprintf(szPathFileDst, "%s\\Channel_%03d.sgy", dstPath, i);
		segySwathFileDst->setPathFile(szPathFileDst);
		//将数据存入文件
		segySwathFileDst->saveData2File();
	}
	return 0;
}

#pragma once


class Transform2SegyBase
{
public:
	Transform2SegyBase();
	~Transform2SegyBase();

	//将Segy文件拆分为单通道格式
	int SplitChannel(const char *pathDst, const char *srcPathFileName);

private:

protected:


};

#pragma once


class Transform2SegyBase
{
public:
	Transform2SegyBase();
	~Transform2SegyBase();

	//��Segy�ļ����Ϊ��ͨ����ʽ
	int SplitChannel(const char *pathDst, const char *srcPathFileName);

private:

protected:


};

#pragma once

#include <map>
#include "IDSChannel.h"

class IDSSwathFragment
{
public:
	IDSSwathFragment()
	{};
	~IDSSwathFragment() { /*clear();*/ };

	/*
	* Fun:ʹ��IDS��RAW�ļ���ʼ������󣬳�ʼ����ɺ󣬿���ȡ��Raw�ļ��ĸ����ṹ������
	*     ʹ��һ��RAW_00000X.scan��ʼ��һ��IDS����Ƭ��
	* Para:����·����Raw�ļ�
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	int init(const char *file)
	{
		//Ϊ����Ƭ�Σ���ʼ��IDS��15��ͨ��
		for (int i = 1; i <= 15; i++)
		{
			IDSChannel *channel = new IDSChannel();
			m_lstData.insert(std::pair<long, IDSChannel*>(i, channel));
		}

		//���ļ�
		FILE *fpIDS = fopen(file, "rb");
		fseek(fpIDS, 6, SEEK_SET);

		char buff[128] = { 0 };
		//��ȡ���ߺ�
		fread(buff, 2, 1, fpIDS);
		iID = buff[0] + buff[1] * 256;

		//�ļ��̶�λ��
		int start = 0x01ec;
		//��λ������λ��
		//fseek(fpIDS, start, SEEK_CUR);
		fseek(fpIDS, start, SEEK_SET);

		//ѭ����ȡÿ��Trace��������ӵ���Ӧ��ͨ����
		while (!feof(fpIDS))
		{
			//����Ҫѭ������ȡ����ͷ�ı�־λ��
			fread(buff, 1, 2, fpIDS);
			if ((0x44 != buff[0]) || (0 != buff[1]))
			{
				//�Ѿ���ȡ���
				break;
			}
			//��ȡͨ������Ϣ
			fread(buff, 1, 2, fpIDS);
			int channelNum = buff[0];

			//���ݺϷ��Լ��
			if ((channelNum <= 0) || (channelNum >= 16))
				continue;
			//��¼trace����
			if (1 == channelNum)
				traceCount++;

			//ȡ��ͨ������
			IDSChannel *channel = m_lstData[channelNum];

			//��ʱ����8�ֽ�
			fread(buff, 1, 8, fpIDS);

			//��ȡʵ������
			int length = 1024;
			unsigned char *pData = (unsigned char *)malloc(length);
			int len = (int)fread(pData, 1, length, fpIDS);

			//while ((!feof(fpIDS)) && (length>0) )
			//{
			//	int len = (int)fread(pData, 1, length, fpIDS);
			//	pData  = pData + len;
			//	length = length - len;
			//}

			//����һ��trace
			IDSTrace16 *trace = new IDSTrace16();
			trace->setTrace16((short *)pData, 512);
			trace->setTraceNum(traceCount);

			//���µ�������ӵ�ͨ����
			channel->appendData(trace);
			channel->setID(channelNum);
		}

		return 0;
	};

	/*
	//�������Ƭ���е�����
	int clear()
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, IDSChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			IDSChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	};
	*/

	//���ز���Ƭ���е������б�
	std::map<long, IDSChannel*> *getDataList()
	{
		return &m_lstData;
	};

	//���ز���ID
	int getID()
	{
		return iID;
	};

	//���ز���Ƭ����trace����
	int getTraceCount()
	{
		return traceCount;
	};
private:
	std::map<long, IDSChannel*> m_lstData;

	//����Ƭ���еĲ���ID
	int iID = 0;

	//����Ƭ���е�Trace����
	int traceCount = 0;
};


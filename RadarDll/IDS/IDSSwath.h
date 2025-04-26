#pragma once

#include "IDSChannel.h"

class IDSSwath
{
public:
	IDSSwath()
	{
		//Ϊ����Ƭ�Σ���ʼ��IDS��15��ͨ��
		for (int i = 1; i <= 15; i++)
		{
			IDSChannel *channel = new IDSChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, IDSChannel*>(i, channel));
		}
	};
	IDSSwath(int size)
	{
		//Ϊ����Ƭ�Σ���ʼ��IDS��15��ͨ��
		for (int i = 1; i <= size; i++)
		{
			IDSChannel *channel = new IDSChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, IDSChannel*>(i, channel));
		}
	};
	~IDSSwath()
	{
		int size = ( int )m_lstData.size();

		//���ߣ������ͷ�
		for (int i = 1; i <= size; i++)
		{
			IDSChannel* channel = m_lstData[i];
			delete channel;
			m_lstData.clear();
		}
	};

	//��ͨ���б����ݣ���ӵ�������
	void swathDataAdd(std::map<long, IDSChannel*> *lstData)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, IDSChannel*>::iterator iter = lstData->begin(); iter != lstData->end(); iter++)
		{
			IDSChannel* p = iter->second;

			//���ݺϷ��Ա���
			if (!p)
				continue;
			if ((p->getID() <= 0) || (p->getID() >= 16))
				continue;

			//ȡ��ͨ������
			IDSChannel* channel = m_lstData[p->getID()];
			if (!channel)
				continue;

			//��ȡͨ����Ӧ������ͷ�����ݿ�
			IDSChannelHeader * header = channel->getHeader();
			IDSChannelBlob   * blob   = channel->getBlob();

			if ((!header) || (!blob))
				continue;

			//����ͨ����trace������
			//int traceCount = header->GetTraceCount() + p->getHeader()->GetTraceCount();
			//header->SetTraceCount(traceCount);

			//���ͨ���е�trace����
			IDSChannelBlob * blobNew = p->getBlob();
			std::map<long, IDSTrace16*> * blobList = blobNew->getTraceList();
			for (std::map<long, IDSTrace16*>::iterator itBlob = blobList->begin(); itBlob != blobList->end(); itBlob++)
			{
				//���ͨ���е�trace����
				channel->appendData(itBlob->second);
			}
		}
	};

	//��������е����ݣ������ͷſռ�
	void swathDataClear()
	{
		//�ͷ�ԭ�з���Ŀռ�
		for (std::map<long, IDSChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			IDSChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//���·���ռ�
		for (int i = 1; i <= 15; i++)
		{
			IDSChannel *channel = new IDSChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, IDSChannel*>(i, channel));
		}

		swathID = 0;
	};

	//���ز���ID
	int getID()
	{
		return swathID;
	};
	//���ò���ID
	void setID( int iID )
	{
		swathID = iID;
	};

	//��ȡ����ͨ���б�
	std::map<long, IDSChannel*> *getData()
	{
		return &m_lstData;
	};

private:
	std::map<long, IDSChannel*> m_lstData;
	int swathID;
};

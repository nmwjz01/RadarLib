#pragma once

#include "MalaChannel.h"

class MalaSwath
{
public:
	MalaSwath()
	{
		//Ϊ����Ƭ�Σ���ʼ��Mala��15��ͨ��
		for (int i = 1; i <= 15; i++)
		{
			MalaChannel *channel = new MalaChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, MalaChannel*>(i, channel));
		}
	};
	~MalaSwath()
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();
	};

	//��������е����ݣ������ͷſռ�
	void swathDataClear()
	{
		//�ͷ�ԭ�з���Ŀռ�
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//���·���ռ�
		for (int i = 1; i <= 15; i++)
		{
			MalaChannel *channel = new MalaChannel();
			channel->setID(i);

			m_lstData.insert(std::pair<long, MalaChannel*>(i, channel));
		}
	};

	/*
	* Fun:   ��ʼ������
	* Param: char* pPath   �����ļ�·��
	*        char *pSwathID����ID��������
	*        int bitFlag   ��־������32λ����16λ
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	int init(char* pPath, char *pSwathID, int bitFlag)
	{
		//�����Ϸ����ж�
		if ((!pPath) || (!pSwathID))
		{
			return 1;
		}
		if ((0 == strlen(pPath)) || (0 == strlen(pSwathID)))
		{
			return 2;
		}

		//��¼�������ͣ�16λ or 32λ
		bitType = bitFlag;
		//��¼��������
		strncpy(swathName, pSwathID, 128);

		//�ж��ǲ���32����
		if (32 == bitFlag)
			return init32(pPath, pSwathID);
		else if (33 == bitFlag)
			return init32Ex(pPath, pSwathID);
		else if (16 == bitFlag)
			return init16(pPath, pSwathID);
		else
			return 3;
	}
	/*
	* Fun:   ��ʼ������,Mala16��ά�״�
	* Param: char* pPath   �����ļ�·��
	*        char *pSwathID����ID��������
	*        int bitFlag   ��־������32λ����16λ
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	int init16Ex(char* pPath, char *pSwathID, int bitFlag)
	{
		//�����Ϸ����ж�
		if ((!pPath) || (!pSwathID))
		{
			return 1;
		}
		if ((0 == strlen(pPath)) || (0 == strlen(pSwathID)))
		{
			return 2;
		}

		//��¼�������ͣ�16λ or 32λ
		bitType = bitFlag;
		//��¼��������
		strncpy(swathName, pSwathID, 128);

		//�ж��ǲ���32����
		return init16Ex(pPath, pSwathID);
	}

	/*
	* Fun:   ��ʼ������--32λ
	* Param: char* pPath   �����ļ�·��
	*        char *pSwathID����ID��������
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	int init32(char* pPath, char *pSwathID)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//ͨ���������
		channelCount = 0;

		//������������ͨ����Ϣ
		while (true)
		{
			channelCount++;

			//�����״�Ŀ¼�£�ָ���ļ�,rd7����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRd7[512] = { 0 };
			sprintf(szPathRd7, "%s\\%s_%03d.rd7", pPath, pSwathID, channelCount);
			intptr_t hFileRd7;
			struct _finddata_t oFileInfoRd7;

			//���û���ҵ�rd7������ֹ
			if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) == -1L)
			{
				_findclose(hFileRd7);
				printf("û�з���rd7�Ĳ�������\n");
				break;
			}
			//�ر�rd7�ļ�������
			_findclose(hFileRd7);

			//�����״�Ŀ¼�£�ָ���ļ�,rad����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRad[512] = { 0 };
			sprintf(szPathRad, "%s\\%s_%03d.RAD", pPath, pSwathID, channelCount);
			intptr_t hFileRad;
			struct _finddata_t oFileInfoRad;

			//���û���ҵ�rad������ֹ
			if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
			{
				_findclose(hFileRad);
				printf("û�з���rad�Ĳ�������\n");
				break;
			}
			//�ر�rd7�ļ�������
			_findclose(hFileRad);

			//����ͷ�ļ����������ļ���
			char szPathFileHeader[512] = { 0 };
			char szPathFileBlob[512]   = { 0 };
			sprintf(szPathFileHeader,"%s\\%s", pPath, oFileInfoRad.name);
			sprintf(szPathFileBlob,  "%s\\%s", pPath, oFileInfoRd7.name);

			//��ʼ��һ��Channel
			MalaChannel *channel = new MalaChannel();
			channel->init(szPathFileHeader, szPathFileBlob, 32 );
			channel->setID(channelCount );

			m_lstData.insert(std::pair<int, MalaChannel*>(channelCount, channel));
		}

		channelCount--;

		return 0;
	}

	/*
	* Fun:   ��ʼ������--32λ
	* Param: char* pPath   �����ļ�·��
	*        char *pSwathID����ID��������
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	int init32Ex(char* pPath, char *pSwathID)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//�����״�Ŀ¼�£�ָ���ļ�,rd7����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRd7[512] = { 0 };
		sprintf(szPathRd7, "%s\\%s.rd7", pPath, pSwathID);
		intptr_t hFileRd7;
		struct _finddata_t oFileInfoRd7;

		//���û���ҵ�rd7������ֹ
		if ((hFileRd7 = (intptr_t)_findfirst(szPathRd7, &oFileInfoRd7)) == -1L)
		{
			_findclose(hFileRd7);
			printf("û�з���rd7�Ĳ�������\n");
			return 1;
		}
		//�ر�rd7�ļ�������
		_findclose(hFileRd7);

		//�����״�Ŀ¼�£�ָ���ļ�,rad����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
		char szPathRad[512] = { 0 };
		sprintf(szPathRad, "%s\\%s.RAD", pPath, pSwathID);
		intptr_t hFileRad;
		struct _finddata_t oFileInfoRad;

		//���û���ҵ�rad������ֹ
		if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
		{
			_findclose(hFileRad);
			printf("û�з���rad�Ĳ�������\n");
			return 2;
		}
		//�ر�rd7�ļ�������
		_findclose(hFileRad);

		//����ͷ�ļ����������ļ���
		char szPathFileHeader[512] = { 0 };
		char szPathFileBlob[512] = { 0 };
		sprintf(szPathFileHeader, "%s\\%s", pPath, oFileInfoRad.name);
		sprintf(szPathFileBlob, "%s\\%s", pPath, oFileInfoRd7.name);

		//��ʼ��һ��Channel
		MalaChannel *channel = new MalaChannel();
		channel->init(szPathFileHeader, szPathFileBlob, 32);
		channel->setID(1);   //ͨ������ֻ��1�������Ե�һ��ͨ��IDΪ1

		m_lstData.insert(std::pair<int, MalaChannel*>(1, channel));

		return 0;
	}

	/*
	* Fun:   ��ʼ������--16λ
	* Param: char* pPath   �����ļ�·��
	*        char *pSwathID����ID��������
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	int init16(char* pPath, char *pSwathID)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//ͨ���������
		channelCount = 0;

		//������������ͨ����Ϣ
		while (true)
		{
			//�����״�Ŀ¼�£�ָ���ļ�,rd3����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRd3[512] = { 0 };
			sprintf(szPathRd3, "%s\\%s_A%03d.rd3", pPath, pSwathID, channelCount);
			intptr_t hFileRd3;
			struct _finddata_t oFileInfoRd3;

			//���û���ҵ�rd7������ֹ
			if ((hFileRd3 = (intptr_t)_findfirst(szPathRd3, &oFileInfoRd3)) == -1L)
			{
				_findclose(hFileRd3);
				printf("û�з���rd3�Ĳ�������\n");
				break;
			}
			//�ر�rd7�ļ�������
			_findclose(hFileRd3);

			//�����״�Ŀ¼�£�ָ���ļ�,rad����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRad[512] = { 0 };
			sprintf(szPathRad, "%s\\%s_A%03d.RAD", pPath, pSwathID, channelCount);
			intptr_t hFileRad;
			struct _finddata_t oFileInfoRad;

			//���û���ҵ�rad������ֹ
			if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
			{
				_findclose(hFileRad);
				printf("û�з���rad�Ĳ�������\n");
				break;
			}
			//�ر�rd7�ļ�������
			_findclose(hFileRad);

			//����ͷ�ļ����������ļ���
			char szPathFileHeader[512] = { 0 };
			char szPathFileBlob[512] = { 0 };
			sprintf(szPathFileHeader, "%s\\%s", pPath, oFileInfoRad.name);
			sprintf(szPathFileBlob, "%s\\%s", pPath, oFileInfoRd3.name);

			//��ʼ��һ��Channel
			MalaChannel *channel = new MalaChannel();
			channel->init(szPathFileHeader, szPathFileBlob, 16);
			channel->setID(channelCount);

			m_lstData.insert(std::pair<int, MalaChannel*>(channelCount, channel));

			channelCount++;
		}

		return 0;
	}

	/*
	* Fun:   ��ʼ������--16λ
	* Param: char* pPath   �����ļ�·��
	*        char *pSwathID����ID��������
	* Return:�ɹ�����0,ʧ�ܷ��ش�����
	*/
	int init16Ex(char* pPath, char *pSwathID)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<long, MalaChannel*>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); iter++)
		{
			MalaChannel* p = iter->second;
			delete p;
		}
		m_lstData.clear();

		//������������ͨ����Ϣ
		{
			//�����״�Ŀ¼�£�ָ���ļ�,rd3����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRd3[512] = { 0 };
			sprintf(szPathRd3, "%s\\%s*.rd3", pPath, pSwathID);
			intptr_t hFileRd3;
			struct _finddata_t oFileInfoRd3;

			//���û���ҵ�rd7������ֹ
			if ((hFileRd3 = (intptr_t)_findfirst(szPathRd3, &oFileInfoRd3)) == -1L)
			{
				printf("û�з���rd3�Ĳ�������\n");
				_findclose(hFileRd3);
				return 0;
			}
			//�ر�rd7�ļ�������
			_findclose(hFileRd3);

			//�����״�Ŀ¼�£�ָ���ļ�,rad����           //64λ�ο���  https://blog.csdn.net/u012925946/article/details/47830701
			char szPathRad[512] = { 0 };
			sprintf(szPathRad, "%s\\%s*.rad", pPath, pSwathID);
			intptr_t hFileRad;
			struct _finddata_t oFileInfoRad;

			//���û���ҵ�rad������ֹ
			if ((hFileRad = (intptr_t)_findfirst(szPathRad, &oFileInfoRad)) == -1L)
			{
				printf("û�з���rad�Ĳ�������\n");
				_findclose(hFileRad);
				return 0;
			}
			//�ر�rd7�ļ�������
			_findclose(hFileRad);

			//����ͷ�ļ����������ļ���
			char szPathFileHeader[512] = { 0 };
			char szPathFileBlob[512] = { 0 };
			sprintf(szPathFileHeader, "%s\\%s", pPath, oFileInfoRad.name);
			sprintf(szPathFileBlob, "%s\\%s", pPath, oFileInfoRd3.name);

			//��ʼ��һ��Channel
			MalaChannel *channel = new MalaChannel();
			channel->init(szPathFileHeader, szPathFileBlob, 16);
			channel->setID(0);

			m_lstData.insert(std::pair<int, MalaChannel*>(0, channel));
		}

		return 0;
	}

	//���ز���Name
	char * getName()
	{
		return swathName;
	};
	//���ò���ID
	void setID( char * name )
	{
		strncpy(swathName, name, 128);
	};
	//��ȡ����ͨ���б�
	std::map<long, MalaChannel*> *getData()
	{
		return &m_lstData;
	};

private:
	std::map<long, MalaChannel*> m_lstData;
	char swathName[128] = { 0 };
	int channelCount = 0;
	int bitType = 16;
};

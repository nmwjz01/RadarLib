#pragma once
#include <cstring>
#include <map>

#include "SegyFileHeaderText.h"
#include "SegyFileHeaderBinary.h"
#include "SegyChannel.h"

class SegySwathFile
{
public:
	SegySwathFile()
	{
		segyFileHeaderText   = NULL;
		segyFileHeaderBinary = NULL;

		memset(szPathFile, 0 , 512);
		iSmaple  = 0;
		iBitType = 0;
	};
	~SegySwathFile()
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, SegyChannel*>::iterator iter = m_lstChannel.begin(); iter != m_lstChannel.end(); iter++)
		{
			SegyChannel* p = iter->second;
			delete p;
		}
		m_lstChannel.clear();
	}

	void setHeaderText(SegyFileHeaderText *fileHeaderText)
	{
		segyFileHeaderText = fileHeaderText;
	}
	void setHeaderBinary(SegyFileHeaderBinary *fileHeaderBinary)
	{
		segyFileHeaderBinary = fileHeaderBinary;

		iSmaple  = segyFileHeaderBinary->getSamples();
		iBitType = segyFileHeaderBinary->getBitType();
	}
	SegyFileHeaderText *getSegyFileHeaderText()
	{
		return segyFileHeaderText;
	}
	SegyFileHeaderBinary *getSegyFileHeaderBinary()
	{
		return segyFileHeaderBinary;
	}

	int getChannelCount()
	{
		return (int)m_lstChannel.size();
	}

	void addChannel(SegyChannel *segyChannel)
	{
		m_lstChannel.insert(std::map<int, SegyChannel*>::value_type(segyChannel->getChannelNum(), segyChannel));
	}
	SegyChannel *getChannel( int iChannelNum )
	{
		return m_lstChannel.at(iChannelNum);
	}

	void setPathFile( char *pathFile )
	{
		strncpy(szPathFile, pathFile, 512);
	}
	char *getPathFile()
	{
		return szPathFile;
	}

	//�������е�����ת�浽�ļ�
	int saveData2File()
	{
		if (strlen(szPathFile) == 0)
			return -1;

		//��һ���ļ�������д��segy�ļ�
		FILE *fp = fopen(szPathFile, "wb");

		if (!segyFileHeaderText)
		{
			//�˴�ǰ�ر��ļ�
			fclose(fp);
			return -2;
		}
		//��Segy�ı��ļ�ͷд���ļ�
		unsigned char *headerText = (unsigned char *)segyFileHeaderText->getBuff();
		int iIndex = 0;
		while (iIndex < 3200)
		{
			iIndex = iIndex + (int)fwrite(headerText + iIndex, 1, 3200 - iIndex, fp);
		}

		if (!segyFileHeaderBinary)
		{
			//�˴�ǰ�ر��ļ�
			fclose(fp);
			return -3;
		}
		//��Segy������ͷд���ļ�
		unsigned char *headerBinary = segyFileHeaderBinary->getBuff();
		iIndex = 0;
		while (iIndex < 400)
		{
			iIndex = iIndex + (int)fwrite(headerBinary + iIndex, 1, 400 - iIndex, fp);
		}

		//��ÿ��ͨ��˳�����
		for (std::map<int, SegyChannel*>::iterator iter = m_lstChannel.begin(); iter != m_lstChannel.end(); iter++)
		{
			SegyChannel* p = iter->second;
			std::map<int, SegyTrace*>* lstTrace = p->getTraceList();

			//��ÿ��ͨ����ÿ��Trace˳�����
			for (std::map<int, SegyTrace*>::iterator iter = lstTrace->begin(); iter != lstTrace->end(); iter++)
			{
				SegyTrace* pTrace = iter->second;
				unsigned char *pDataHeader = pTrace->getHeaderBuff();
				unsigned char *pData = (unsigned char *)pTrace->getData();

				//��Traceͷд���ļ�
				iIndex = 0;
				while (iIndex < 240)
				{
					iIndex = iIndex + (int)fwrite(pDataHeader + iIndex, 1, 240 - iIndex, fp);
				}

				//��Trace����
				int iLength = iBitType / 8 * iSmaple;
				iIndex = 0;
				while (iIndex < iLength)
				{
					iIndex = iIndex + (int)fwrite(pData + iIndex, 1, iLength - iIndex, fp);
				}
			}
		}

		//д����ɾ͹ر��ļ�
		fclose(fp);

		return 0;
	}
	//���ļ��ж�ȡ���ݵ�����
	int loadFile2Data( const char *pathFile )
	{
		if (strlen(pathFile) == 0)
			return -1;

		strncpy(szPathFile, pathFile, 512);

		//���ļ�
		FILE *fp = fopen(pathFile, "rb");
		if (!fp)
		{
			fclose(fp);
			return -2;
		}

		//��ȡ�ı��ļ�ͷ�������ݴ����ı��ļ�ͷ����
		char *szBuff = (char *)malloc(3200); memset(szBuff, 0, 3200);
		int iIndex = 0;
		while (iIndex < 3200)
		{
			iIndex = iIndex + (int)fread(szBuff + iIndex, 1, 3200 - iIndex, fp);
		}
		if (segyFileHeaderText)
			delete segyFileHeaderText;
		segyFileHeaderText = new SegyFileHeaderText();
		segyFileHeaderText->setBuff(szBuff);

		//��ȡ�������ļ�ͷ
		szBuff = (char *)malloc(400); memset(szBuff, 0, 400);
		iIndex = 0;
		while (iIndex < 400)
		{
			iIndex = iIndex + (int)fread(szBuff + iIndex, 1, 400 - iIndex, fp);
		}
		if (segyFileHeaderBinary)
			delete segyFileHeaderBinary;
		segyFileHeaderBinary = new SegyFileHeaderBinary();
		segyFileHeaderBinary->setBuff((unsigned char *)szBuff);

		//��ȡ��������
		iSmaple  = segyFileHeaderBinary->getSamples();
		iBitType = segyFileHeaderBinary->getBitType();

		//����Ѿ��������ݣ�����Ҫ���
		if (m_lstChannel.size())
		{
			clearChannelData(m_lstChannel);
		}

		//����һ��Channel
		SegyChannel *segyChannel = new SegyChannel();
		segyChannel->setChannelNum(1);

		int iChannelNum = 1;
		//int iTraceNum   = 1;
		//ѭ����ȡȫ��Trace����
		while (!feof(fp))
		{
			//��ȡTraceͷ
			szBuff = (char *)malloc(240); memset(szBuff, 0, 240);
			iIndex = 0;
			while((iIndex < 240) && (!feof(fp)))
			{
				iIndex = iIndex + (int)fread(szBuff + iIndex, 1, 240 - iIndex, fp);
			}
			if (0 == iIndex)
			{
				//�Ѿ���ȡ����β��
				free(szBuff);
				break;
			}
			SegyTrace *segyTrace = new SegyTrace();
			segyTrace->setHeaderBuff((unsigned char *)szBuff);

			//��ȡTrace����
			int iTraceLenth = iBitType / 8 * iSmaple;
			szBuff = (char *)malloc(iTraceLenth); memset(szBuff, 0, iTraceLenth);
			iIndex = 0;
			while (iIndex < iTraceLenth)
			{
				iIndex = iIndex + (int)fread(szBuff + iIndex, 1, iTraceLenth - iIndex, fp);
			}
			segyTrace->setData((void *)szBuff);
			//segyTrace->setTraceNum(iTraceNum);

			//��ȡ�õ���Trace���ڵ�ǰͨ��
			if (segyTrace->getChannelNum() > iChannelNum)
			{
				//�õ���Trace�����ں���ͨ����������Ҫ��ͨ���ŵ�ͨ���б�
				m_lstChannel.insert(std::map<int, SegyChannel*>::value_type(iChannelNum, segyChannel));

				//������Ҫ����һ��ͨ��
				iChannelNum++;
				segyChannel = new SegyChannel();
				segyChannel->setChannelNum(iChannelNum);
			}

			segyChannel->addTrace(segyTrace);
		}

		//��Ҫ�����һ��ͨ���ŵ�ͨ���б�
		m_lstChannel.insert(std::map<int, SegyChannel*>::value_type(iChannelNum, segyChannel));

		return 0;
	}

private :
	void clearChannelData(std::map<int, SegyChannel*> m_lst)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, SegyChannel*>::iterator iter = m_lst.begin(); iter != m_lst.end(); iter++)
		{
			SegyChannel* p = iter->second;
			delete p;
		}
		m_lst.clear();
	}
private:
	SegyFileHeaderText   *segyFileHeaderText;       //3200�ֽ��ı�ͷ
	SegyFileHeaderBinary *segyFileHeaderBinary;     //400�ֽڶ�����ͷ

	//����Segy�ļ���һ���ļ�����һ��Swath��������߿��ܰ������ͨ����Ҳ���ܰ���һ��ͨ��
	std::map<int, SegyChannel*> m_lstChannel;


	char szPathFile[512];    //�����ļ�
	int iSmaple;             //��������
	int iBitType;            //�������ݵ��λ��
};

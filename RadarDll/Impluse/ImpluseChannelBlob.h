#pragma once

#include <map>

//#include "Trace16.h"
//#include "Trace32.h"

class SwathChannelBlob
{
public:
	SwathChannelBlob(){}
	~SwathChannelBlob()
	{
		//�ͷ����з���Ŀռ�
		clearTraceData();
	}

	/*
	* Fun:����ͨ�����ݲ���
	* Param:
	*     int iVersion     trace���ݵ�λ��,16λ��32λ
	*     int iSample      ÿ��trace����������
	*     int iTraceCount  Trace����
	*/
	void setChannelParam( int iVersion , int iSample, int iTraceCount )
	{
		m_iDataVersion = iVersion;
		m_iSample      = iSample;
		m_iTraceCount  = iTraceCount;
	}
	//��ʼ�����߿�����
	int init(char* pPathFile)
	{
		if (nullptr == pPathFile)
			return -1;
		if (0 >= strlen(pPathFile))
			return -1;

		strcpy(m_FileName, pPathFile);
		return 0;
	}
	/*
	* Fun:��ȡͨ����trace����
	* Param
	*     int iStart ��ʼTrace��
	*     int iCount ��ȡ������
	*     std::map<long, Trace16> &lstData  ���������ݴ�ŵ�map��
	* Return: �ɹ�����0��ʧ�ܷ��ش�����
	*/
	int getTraceData( int iStart , int iCount , std::map<long, Trace16*> &lstData)
	{
		for (int i = 0; i < iCount; i++)
		{
			if ((iStart + i) >= (int)m_lstData16.size())
				break;
			lstData.insert(std::pair<long, Trace16*>(iStart + i, m_lstData16[iStart + i]) );
		}
		return 0;
	}
	std::map<int, Trace16*> * getTraceData()
	{
		return &m_lstData16;
	}
	/*
    * Fun:��ȡͨ����trace����
    * Param:��
    * Return: �ɹ�����std::map<long, Trace16>��ʧ�ܷ���null
    */
	std::map<int, Trace16*> * getTraceData16()
	{
		return &m_lstData16;
	}
	std::map<int, Trace32*> * getTraceData32()
	{
		return &m_lstData32;
	}
	void AppendTraceData(std::map<int, void*> *lst, int bitFlag)
	{
		if (!lst)
			return;

		if (16 == bitFlag)
		{
			std::map<int, Trace16*> *lstTmp = (std::map<int, Trace16*> *)lst;

			//�ͷ����з���Ŀռ�
			for (std::map<int, Trace16*>::iterator iter = lstTmp->begin(); iter != lstTmp->end(); iter++)
			{
				Trace16* p = iter->second;
				m_lstData16.insert(std::pair<int, Trace16*>(m_iTraceCount, p));

				m_iTraceCount++;
			}

			lstTmp->clear();
		}
		else if (32 == bitFlag)
		{
			std::map<int, Trace32*> *lstTmp = (std::map<int, Trace32*> *)lst;

			//�ͷ����з���Ŀռ�
			for (std::map<int, Trace32*>::iterator iter = lstTmp->begin(); iter != lstTmp->end(); iter++)
			{
				Trace32* p = iter->second;
				m_lstData32.insert(std::pair<int, Trace32*>(m_iTraceCount, p));

				m_iTraceCount++;
			}

			lstTmp->clear();
		}
	}
	void AppendTraceData( int traceNum, void *data, int bitFlag)
	{
		if (!data)
			return;

		if (16 == bitFlag)
		{
			Trace16* tmp = (Trace16*)data;
			m_lstData16.insert(std::pair<int, Trace16*>(traceNum, tmp));
		}
		else if (32 == bitFlag)
		{
			Trace32* tmp = (Trace32*)data;
			m_lstData32.insert(std::pair<int, Trace32*>(traceNum, tmp));
		}
	}
	void clearTraceData()
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			delete p;
		}
		m_lstData16.clear();

		//�ͷ����з���Ŀռ�
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			delete p;
		}
		m_lstData32.clear();

		m_iTraceCount = 0;
	}

	//�����г�ֱ�ﲨ���г���ʼ������
	void cutTraceData( int iZero )
	{
		//�����Ϸ����ж�
		if (iZero >= m_iSample)
			return;

		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			if (!p)
				continue;
			p->cutSamples(iZero);
		}

		//�ͷ����з���Ŀռ�
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			if (!p)
				continue;
			p->cutSamples(iZero);
		}
		m_iSample = m_iSample - iZero;
	}

	/*
	* Fun:ʵ�����������
	* Param:coef����ϵ��
	*/
	int GainInvDecayConst(int *coef)
	{
		if (16 == m_iDataVersion)
			return GainInvDecayConst16(coef);
		else if (32 == m_iDataVersion)
			return GainInvDecayConst32(coef);
		else
			return -2;
	}
	/*
	* Fun:ʵ�����������
	* Param:coef����ϵ��
	*/
	int GainInvDecayCoef(int *coef)
	{
		if (16 == m_iDataVersion)
			return GainInvDecayCoef16(coef);
		else if (32 == m_iDataVersion)
			return GainInvDecayCoef32(coef);
		else
			return -2;
	}
	/*
	* Fun:ʵ�����������--��������
	* Param:k����ϵ��
	*    k : ȡֵ��ΧΪ 1 ... ... 100
	*    n : ȡֵ��ΧΪ 1 ... ... 10
	*/
	int GainInvDecayCurve(int k, int n)
	{
		if (16 == m_iDataVersion)
			return GainInvDecayCurve16(k, n);
		else if (32 == m_iDataVersion)
			return GainInvDecayCurve32(k, n);
		else
			return -2;
	}

	/*
	* Fun:ɾ����������
	* Param:
	*/
	int RemoveBackgr()
	{
		if (16 == m_iDataVersion)
		{
			float *pAvg = (float *)malloc(m_iSample * 8);
			getLineAvg16(pAvg);
			int iResult = RemoveBackgr16(pAvg);
			free(pAvg);
			return iResult;
		}
		else if (32 == m_iDataVersion)
		{
			double *pAvg = (double *)malloc(m_iSample * 8);
			getLineAvg32(pAvg);
			int iResult = RemoveBackgr32(pAvg);
			free(pAvg);
			return iResult;
		}
		else
			return -2;
	}

	/*
	* Fun:ɾ��ֱ������
	* Param:
	*/
	int RemoveDC()
	{
		if (16 == m_iDataVersion)
		{
			double *pAvg = (double *)malloc(m_iTraceCount * 8);
			getColumnAvg16(pAvg);
			int iResult = RemoveDC16(pAvg);
			free(pAvg);
			return iResult;
		}
		else if (32 == m_iDataVersion)
		{
			double *pAvg = (double *)malloc(m_iTraceCount * 8);
			getColumnAvg32(pAvg);
			int iResult = RemoveDC32(pAvg);
			free(pAvg);
			return iResult;
		}
		else
			return -2;
	}

	/*
	* Fun:��ȡͨ����trace����
	* Param
	*     int iIndex Trace������
	* Return: �ɹ�����Trace16��ʧ�ܷ���null
	*/
	Trace16 *getTraceData(int iIndex)
	{
		return m_lstData16[iIndex];
	}

	/*
	* Fun:    ��һ��ͨ��������trace
	* Param:  ��
	* Return: �ɹ�����0��ʧ�ܷ��ش�����
	*/
	int loadTrace()
	{
		if (m_lstData16.size() > 0)
			return 0;

		if (16 == m_iDataVersion)
			return loadTrace16();
		else if (32 == m_iDataVersion)
			return loadTrace32();
		else
			return -2;
	}

	/*
	* Fun:�ͷ�ͨ���е�����Trace
	* Param:  ��
	* Return: �ɹ�����0��ʧ�ܷ��ش�����
	*/
	int unloadTrace()
	{
		//�ͷ����е�Trace����
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			if (16 == m_iDataVersion)
				unLoadTrace16(iter->second);
			else if (32 == m_iDataVersion)
				unLoadTrace32(iter->second);
			else
				continue;
			//free( iter->second );
		}
		m_lstData16.clear();
		return 0;
	}

	/*
	* Fun:��ȡÿ�е�ƽ�����ݣ�����ڷ��ص�Trace
	*/
	int getLineAvg16(float *pAvg)
	{
		long long iSum[1024] = { 0 };

		for (int i = 0; i < m_iSample; i++)
		{
			int iTraceCount = 0;
			//��ÿ������˳�����
			for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
			{
				Trace16* p = iter->second;
				short * pData = (short *)p->getTrace();

				iSum[i] = iSum[i] + pData[i];

				iTraceCount++;
				//if (iTraceCount >= 1024)
				//	break;
			}
			pAvg[i] = (float)iSum[i] / (float)iTraceCount;

		}

		return 0;
	}
	/*
	* Fun:��ȡÿ�е�ƽ�����ݣ�����ڷ��ص�Trace
	*/
	int getLineAvg32(double *pAvg)
	{
		long long iSum[1024] = { 0 };

		for (int i = 0; i < m_iSample; i++)
		{
			int iTraceCount = 0;
			//��ÿ������˳�����
			for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
			{
				Trace32* p = iter->second;
				long * pData = p->getTrace();

				iSum[i] = iSum[i] + pData[i];

				iTraceCount++;
				//if (iTraceCount >= 1024)
				//	break;
			}
			pAvg[i] = (double)iSum[i] / (double)iTraceCount;
		}

		return 0;
	}


	/*
    * Fun:��ȡÿ�е�ƽ�����ݣ�����ڷ��ص�Trace
    */
	int getColumnAvg16(double *pAvg)
	{
		int iTraceCount = 0;
		//��ÿ������˳�����
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			short * pData = p->getTrace();
			int iSample = p->getSamples();

			long long iSum = 0;
			for (int i = 0; i < iSample; i++)
			{
				iSum = iSum + pData[i];
			}

			pAvg[iTraceCount] = (double)iSum / (double)iSample;
			iTraceCount++;
		}
		return 0;
	}
	/*
	* Fun:��ȡÿ�е�ƽ�����ݣ�����ڷ��ص�Trace
	*/
	int getColumnAvg32(double *pAvg)
	{
		int iTraceCount = 0;
		//��ÿ������˳�����
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			long * pData = p->getTrace();
			int iSample = p->getSamples();

			long long iSum = 0;
			for (int i = 0; i < iSample; i++)
			{
				iSum = iSum + pData[i];
			}

			pAvg[iTraceCount] = (double)iSum / (double)iSample;
			iTraceCount++;
		}
		return 0;
	}


	/*
	* Fun:   ��ȡͨ����Trace������
	* Param: ��
	* Return:�ɹ�����0��ʧ�ܷ��ش�����
	*/
	int getTraceCount()
	{
		return ( int )m_lstData16.size();
	}

	//��Tarce���ݴ洢Ϊ�ļ�
	int saveTrace( const char * pathFile )
	{
		if (16 == m_iDataVersion)
			saveTrace16(pathFile);
		else if (32 == m_iDataVersion)
			saveTrace32(pathFile);
		return 0;
	}

	//ɾ����ЧTrace(��Trace���ݺ�С��ʱ��)
	int deleteTrace16(int threshold)
	{
		int count = 0;

		//��ÿ������˳�����
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); )
		{
			Trace16* p = iter->second;
			short * pData = p->getTrace();
			int iSample = p->getSamples();

			unsigned int sum = 0;
			//����һ��Trace�и���ֵ�ĺ�
			for (int i = 0; i < m_iSample; i++)
			{
				unsigned short tmp = *((unsigned short*)pData);
				sum = sum + tmp;
				pData++;
			}
			//�������С����ֵ����ɾ��
			if (sum <= (unsigned int)threshold)
			{
				iter = m_lstData16.erase(iter);
				count++;
			}
			else
				iter++;
		}

		return count;
	}

	//ɾ����ЧTrace(��Trace���ݺ�С��ʱ��)
	int deleteTrace32(int threshold)
	{
		int count = 0;

		//��ÿ������˳�����
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); )
		{
			Trace32* p = iter->second;
			long * pData = p->getTrace();
			int iSample = p->getSamples();

			unsigned long sum = 0;
			//����һ��Trace�и���ֵ�ĺ�
			for (int i = 0; i < m_iSample; i++)
			{
				unsigned long tmp = *((unsigned long*)pData);
				sum = sum + tmp;
				pData++;
			}
			//�������С����ֵ����ɾ��
			if (sum <= (unsigned int)threshold)
			{
				iter = m_lstData32.erase(iter);
				count++;
			}
			else
				iter++;
		}

		return count;
	}
private:
	//��ȡһ��ͨ�������е�16λTrace
	int loadTrace16()
	{
		FILE * m_pFile = fopen(m_FileName, "rb");
		if (nullptr == m_pFile)
			return -2;

		//��ȡ��ʼλ��Ϊ0
		fseek(m_pFile, 0, SEEK_SET);
		//ѭ����ȡĿ��������
		for (int i = 0; i < m_iTraceCount; i++)
		{
			Trace16 *pTrace = new Trace16();
			char* pData = (char*)malloc(m_iSample * 2);

			fread(pData, 2, m_iSample, m_pFile);
			pTrace->setTrace((short*)pData, m_iSample);

			m_lstData16.insert(std::pair<int, Trace16*>(i, pTrace));
		}

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}
		return 0;
	}
	//��ȡһ��ͨ�������е�32λTrace
	int loadTrace32()
	{
		FILE * m_pFile = fopen(m_FileName, "rb");
		if (nullptr == m_pFile)
			return -2;

		//��ȡ��ʼλ��Ϊ0
		fseek(m_pFile, 0, SEEK_SET);
		//ѭ����ȡĿ��������
		long* pData32 = (long*)malloc(m_iSample * m_iTraceCount * 4);
		long *pos = pData32;
		for (int i = 0; i < m_iTraceCount; i++)
		{
			Trace32 *pTrace32 = new Trace32();
			long *pData32 = (long *)malloc(m_iSample * 4);
			pTrace32->setTrace((long*)pData32, m_iSample);

			Trace16 *pTrace = new Trace16();
			short* pData16 = (short*)malloc(m_iSample * 2);
			pTrace->setTrace((short*)pData16, m_iSample);

			//��ȡһ����������
			fread(pos, 4, m_iSample, m_pFile);

			for (int j = 0; j < m_iSample; j++)
			{
				//�洢32λ����
				*pData32 = (long)(*pos);
				pData32++;

				//��32Ϊ��Ϊ16λ��ȥ����λ
				*pData16 = (short)(*pos / 0x10000);
				pData16++;

				pos++;
			}

			m_lstData32.insert(std::pair<int, Trace32*>(i, pTrace32));
			m_lstData16.insert(std::pair<int, Trace16*>(i, pTrace));
		}
		free(pData32);
		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}
		return 0;
	}
	//�ͷ�һ��ͨ�������е�16λTrace
	int unLoadTrace16(Trace16 *pTrace)
	{
		if (pTrace)
		{
			delete pTrace;
			pTrace = NULL;
		}
		return 0;
	}
	//�ͷ�һ��ͨ�������е�32λTrace
	int unLoadTrace32(Trace16 *pTrace)
	{
		if (pTrace)
		{
			delete pTrace;
			pTrace = NULL;
		}
		return 0;
	}

	//�洢16λTrace����
	void saveTrace16(const char * pathFile)
	{
		FILE * m_pFile = fopen(pathFile, "wb");
		if (nullptr == m_pFile)
			return;

		//��ȡ��ʼλ��Ϊ0
		fseek(m_pFile, 0, SEEK_SET);

		//��ÿ������˳�����
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			short * pData = p->getTrace();
			int iSample   = p->getSamples();

			fwrite(pData, 2, iSample, m_pFile);
		}

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}
	}
	void saveTrace32(const char * pathFile)
	{
		FILE * m_pFile = fopen(pathFile, "wb");
		if (nullptr == m_pFile)
			return;

		//��ȡ��ʼλ��Ϊ0
		fseek(m_pFile, 0, SEEK_SET);

		//��ÿ������˳�����
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			long * pData = p->getTrace();
			int iSample = p->getSamples();

			fwrite(pData, 4, iSample, m_pFile);
		}

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}
	}

	//�����16λ--����ƫ��
	int GainInvDecayConst16(int *coef)
	{
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			if (!p)
				continue;
			//p->cutSamples(iZero);
			p->GainInvDecayConst(coef);
		}

		return 0;
	}
	//�����32λ--����ƫ��
	int GainInvDecayConst32(int *coef)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			if (!p)
				continue;
			//p->cutSamples(iZero);
			p->GainInvDecayConst(coef);
		}

		return 0;
	}

	//�����16λ--����ϵ��
	int GainInvDecayCoef16(int *coef)
	{
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			if (!p)
				continue;
			//p->cutSamples(iZero);
			p->GainInvDecayCoef(coef);
		}

		return 0;
	}
	//�����32λ--����ϵ��
	int GainInvDecayCoef32(int *coef)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			if (!p)
				continue;
			//p->cutSamples(iZero);
			p->GainInvDecayCoef(coef);
		}

		return 0;
	}

	//�����16λ--���������߷Ŵ�ֵ
	int GainInvDecayCurve16(int k, int n)
	{
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			if (!p)
				continue;
			//p->cutSamples(iZero);
			p->GainInvDecayCurve(k, n);
		}

		return 0;
	}
	//�����32λ--���������߷Ŵ�ֵ
	int GainInvDecayCurve32(int k, int n)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			if (!p)
				continue;
			//p->cutSamples(iZero);
			p->GainInvDecayCurve(k, n);
		}

		return 0;
	}

	//ȥ��������16λ
	int RemoveBackgr16(float *pAvg)
	{
		//���з���Ŀռ�
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			if (!p)
				continue;

			//FILE * fpTmp = fopen( "C:\\MyData_C.iprb", "wb" );
			//char szBuff[8] = { 0 };

			short * pData = (short *)p->getTrace();
			//ȥ��������
			for (int i = 0; i < p->getSamples(); i++)
			{
				//float iValue = (float)std::round((float)pData[i] - (float)pAvg[i]);
				//printf("%f", iValue );

				pData[i] = (short)std::round((float)pData[i] - (float)pAvg[i]);

				//sprintf(szBuff, "%02x", pData[i]);
				//fwrite(pData+i, 2, 1, fpTmp);
			}
			//fclose(fpTmp);
			//printf( "OK" );
		}

		return 0;
	}
	//ȥ��������32λ
	int RemoveBackgr32(double *pAvg)
	{
		//�ͷ����з���Ŀռ�
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			if (!p)
				continue;

			int *pData = ( int * )p->getTrace();
			//ȥ��������
			for (int i = 0; i < p->getSamples(); i++)
			{
				pData[i] = (int)std::round((double)pData[i] - (double)pAvg[i]);
			}
		}

		return 0;
	}

	//ȥֱ������16λ
	int RemoveDC16(double *pAvg)
	{
		int iTraCount = 0;
		//����Trace
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			if (!p)
				continue;

			for (int i = 0; i < m_iSample; i++)
			{
				short *pData = p->getTrace();

				pData[i] = (short)std::round((double) pData[i] - pAvg[iTraCount]);
			}

			iTraCount++;
		}

		return 0;
	}
	//ȥֱ������32λ
	int RemoveDC32(double *pAvg)
	{
		int iTraCount = 0;
		//����Trace
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			if (!p)
				continue;

			for (int i = 0; i < m_iSample; i++)
			{
				long *pData = p->getTrace();

				pData[i] = (long)std::round((double)pData[i] - pAvg[iTraCount]);
			}

			iTraCount++;
		}

		return 0;
	}
private:
	int m_iDataVersion = 16;   //trace���ݵ�λ��
	int m_iSample      = 0;    //һ��trace�е���������
	int m_iTraceCount  = 0;    //һ��ͨ����Trace������

	std::map<int, Trace16*> m_lstData16;
	std::map<int, Trace32*> m_lstData32;
	char m_FileName[1024] = { 0 };
};

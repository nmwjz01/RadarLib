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
		//释放所有分配的空间
		clearTraceData();
	}

	/*
	* Fun:设置通道数据参数
	* Param:
	*     int iVersion     trace数据的位数,16位或32位
	*     int iSample      每个trace的样本数量
	*     int iTraceCount  Trace数量
	*/
	void setChannelParam( int iVersion , int iSample, int iTraceCount )
	{
		m_iDataVersion = iVersion;
		m_iSample      = iSample;
		m_iTraceCount  = iTraceCount;
	}
	//初始化测线块数据
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
	* Fun:读取通道中trace数据
	* Param
	*     int iStart 开始Trace的
	*     int iCount 读取的数量
	*     std::map<long, Trace16> &lstData  将返回数据存放到map中
	* Return: 成功返回0，失败返回错误码
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
    * Fun:读取通道中trace数据
    * Param:无
    * Return: 成功返回std::map<long, Trace16>，失败返回null
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

			//释放所有分配的空间
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

			//释放所有分配的空间
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
		//释放所有分配的空间
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			delete p;
		}
		m_lstData16.clear();

		//释放所有分配的空间
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			delete p;
		}
		m_lstData32.clear();

		m_iTraceCount = 0;
	}

	//用于切除直达波，切除开始的数据
	void cutTraceData( int iZero )
	{
		//参数合法性判断
		if (iZero >= m_iSample)
			return;

		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			if (!p)
				continue;
			p->cutSamples(iZero);
		}

		//释放所有分配的空间
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
	* Fun:实现逆振幅增益
	* Param:coef增益系数
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
	* Fun:实现逆振幅增益
	* Param:coef增益系数
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
	* Fun:实现逆振幅增益--线性增益
	* Param:k增益系数
	*    k : 取值范围为 1 ... ... 100
	*    n : 取值范围为 1 ... ... 10
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
	* Fun:删除背景噪声
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
	* Fun:删除直流噪声
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
	* Fun:读取通道中trace数据
	* Param
	*     int iIndex Trace的索引
	* Return: 成功返回Trace16，失败返回null
	*/
	Trace16 *getTraceData(int iIndex)
	{
		return m_lstData16[iIndex];
	}

	/*
	* Fun:    打开一个通道中所有trace
	* Param:  无
	* Return: 成功返回0，失败返回错误码
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
	* Fun:释放通道中的所有Trace
	* Param:  无
	* Return: 成功返回0，失败返回错误码
	*/
	int unloadTrace()
	{
		//释放所有的Trace数据
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
	* Fun:获取每行的平均数据，存放于返回的Trace
	*/
	int getLineAvg16(float *pAvg)
	{
		long long iSum[1024] = { 0 };

		for (int i = 0; i < m_iSample; i++)
		{
			int iTraceCount = 0;
			//将每个测线顺序计算
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
	* Fun:获取每行的平均数据，存放于返回的Trace
	*/
	int getLineAvg32(double *pAvg)
	{
		long long iSum[1024] = { 0 };

		for (int i = 0; i < m_iSample; i++)
		{
			int iTraceCount = 0;
			//将每个测线顺序计算
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
    * Fun:获取每列的平均数据，存放于返回的Trace
    */
	int getColumnAvg16(double *pAvg)
	{
		int iTraceCount = 0;
		//将每个测线顺序计算
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
	* Fun:获取每列的平均数据，存放于返回的Trace
	*/
	int getColumnAvg32(double *pAvg)
	{
		int iTraceCount = 0;
		//将每个测线顺序计算
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
	* Fun:   读取通道中Trace的数量
	* Param: 无
	* Return:成功返回0，失败返回错误码
	*/
	int getTraceCount()
	{
		return ( int )m_lstData16.size();
	}

	//将Tarce数据存储为文件
	int saveTrace( const char * pathFile )
	{
		if (16 == m_iDataVersion)
			saveTrace16(pathFile);
		else if (32 == m_iDataVersion)
			saveTrace32(pathFile);
		return 0;
	}

	//删除无效Trace(当Trace数据很小的时候)
	int deleteTrace16(int threshold)
	{
		int count = 0;

		//将每个测线顺序存入
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); )
		{
			Trace16* p = iter->second;
			short * pData = p->getTrace();
			int iSample = p->getSamples();

			unsigned int sum = 0;
			//计算一个Trace中各个值的和
			for (int i = 0; i < m_iSample; i++)
			{
				unsigned short tmp = *((unsigned short*)pData);
				sum = sum + tmp;
				pData++;
			}
			//如果数据小于阈值，就删除
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

	//删除无效Trace(当Trace数据很小的时候)
	int deleteTrace32(int threshold)
	{
		int count = 0;

		//将每个测线顺序存入
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); )
		{
			Trace32* p = iter->second;
			long * pData = p->getTrace();
			int iSample = p->getSamples();

			unsigned long sum = 0;
			//计算一个Trace中各个值的和
			for (int i = 0; i < m_iSample; i++)
			{
				unsigned long tmp = *((unsigned long*)pData);
				sum = sum + tmp;
				pData++;
			}
			//如果数据小于阈值，就删除
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
	//读取一个通道中所有的16位Trace
	int loadTrace16()
	{
		FILE * m_pFile = fopen(m_FileName, "rb");
		if (nullptr == m_pFile)
			return -2;

		//读取开始位置为0
		fseek(m_pFile, 0, SEEK_SET);
		//循环读取目标数据量
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
	//读取一个通道中所有的32位Trace
	int loadTrace32()
	{
		FILE * m_pFile = fopen(m_FileName, "rb");
		if (nullptr == m_pFile)
			return -2;

		//读取开始位置为0
		fseek(m_pFile, 0, SEEK_SET);
		//循环读取目标数据量
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

			//读取一个道的数据
			fread(pos, 4, m_iSample, m_pFile);

			for (int j = 0; j < m_iSample; j++)
			{
				//存储32位数据
				*pData32 = (long)(*pos);
				pData32++;

				//把32为变为16位，去掉高位
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
	//释放一个通道中所有的16位Trace
	int unLoadTrace16(Trace16 *pTrace)
	{
		if (pTrace)
		{
			delete pTrace;
			pTrace = NULL;
		}
		return 0;
	}
	//释放一个通道中所有的32位Trace
	int unLoadTrace32(Trace16 *pTrace)
	{
		if (pTrace)
		{
			delete pTrace;
			pTrace = NULL;
		}
		return 0;
	}

	//存储16位Trace数据
	void saveTrace16(const char * pathFile)
	{
		FILE * m_pFile = fopen(pathFile, "wb");
		if (nullptr == m_pFile)
			return;

		//读取开始位置为0
		fseek(m_pFile, 0, SEEK_SET);

		//将每个测线顺序存入
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

		//读取开始位置为0
		fseek(m_pFile, 0, SEEK_SET);

		//将每个测线顺序存入
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

	//逆振幅16位--叠加偏移
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
	//逆振幅32位--叠加偏移
	int GainInvDecayConst32(int *coef)
	{
		//释放所有分配的空间
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

	//逆振幅16位--乘以系数
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
	//逆振幅32位--乘以系数
	int GainInvDecayCoef32(int *coef)
	{
		//释放所有分配的空间
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

	//逆振幅16位--增加正曲线放大值
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
	//逆振幅32位--增加正曲线放大值
	int GainInvDecayCurve32(int k, int n)
	{
		//释放所有分配的空间
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

	//去背景噪声16位
	int RemoveBackgr16(float *pAvg)
	{
		//所有分配的空间
		for (std::map<int, Trace16*>::iterator iter = m_lstData16.begin(); iter != m_lstData16.end(); iter++)
		{
			Trace16* p = iter->second;
			if (!p)
				continue;

			//FILE * fpTmp = fopen( "C:\\MyData_C.iprb", "wb" );
			//char szBuff[8] = { 0 };

			short * pData = (short *)p->getTrace();
			//去背景操作
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
	//去背景噪声32位
	int RemoveBackgr32(double *pAvg)
	{
		//释放所有分配的空间
		for (std::map<int, Trace32*>::iterator iter = m_lstData32.begin(); iter != m_lstData32.end(); iter++)
		{
			Trace32* p = iter->second;
			if (!p)
				continue;

			int *pData = ( int * )p->getTrace();
			//去背景操作
			for (int i = 0; i < p->getSamples(); i++)
			{
				pData[i] = (int)std::round((double)pData[i] - (double)pAvg[i]);
			}
		}

		return 0;
	}

	//去直流噪声16位
	int RemoveDC16(double *pAvg)
	{
		int iTraCount = 0;
		//所有Trace
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
	//去直流噪声32位
	int RemoveDC32(double *pAvg)
	{
		int iTraCount = 0;
		//所有Trace
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
	int m_iDataVersion = 16;   //trace数据的位数
	int m_iSample      = 0;    //一个trace中的样本数量
	int m_iTraceCount  = 0;    //一个通道中Trace的数量

	std::map<int, Trace16*> m_lstData16;
	std::map<int, Trace32*> m_lstData32;
	char m_FileName[1024] = { 0 };
};

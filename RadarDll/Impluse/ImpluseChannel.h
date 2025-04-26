#pragma once


//#include "Impluse\\ImpluseChannelHeader.h"
//#include "Impluse\\ImpluseChannelBlob.h"

class SwathChannel
{
public:
	SwathChannel(){}
	~SwathChannel(){}

	/*
	* Fun:   初始化通道对象
	* Param: char *pIPRB 通道块数据
	*        char *pIPRH 通道块头
	* Return:成功返回0，失败返回错误码
	*/
	int init( char *pIPRB , char *pIPRH )
	{
		int iResult = 0;

		//初始化块头
		iResult = m_oHeader.init(pIPRH);
		if (0 != iResult)
			return iResult;

		//初始化块数据
		iResult = m_oBlob.init(pIPRB);
		if (0 != iResult)
			return iResult;

		//设置块参数
		m_oBlob.setChannelParam(m_oHeader.getDataVersion() , m_oHeader.getSample() , m_oHeader.getTraceCount() );

		return 0;
	}

	/*
	* Fun:   设置通道ID
	* Param: int iNo 通道ID
	* Return:无
	*/
	void setNo( int iNo )
	{
		m_iNo = iNo;
	}

	/*
	* Fun:   获取通道ID
	* Param: 无
	* Return:通道ID
	*/
	int getNo()
	{
		return m_iNo;
	}

	/*
	* Fun:获得块对象
	* Return:成功返回块对象，失败返回NULL
	*/
	SwathChannelBlob* getChannelBlob()
	{
		return &m_oBlob;
	}

	/*
	* Fun:获得块头
	* Return:成功返回块头，失败返回NULL
	*/
	SwathChannelHeader* getChannelHeader()
	{
		return &m_oHeader;
	}

private:
	SwathChannelHeader m_oHeader;
	SwathChannelBlob   m_oBlob;
	int m_iNo = 0;
};

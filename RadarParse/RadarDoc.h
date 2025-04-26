
// RadarDoc.h: RadarDoc 类的接口
//


#pragma once


class RadarDoc : public CDocument
{
protected: // 仅从序列化创建
	RadarDoc() noexcept;
	DECLARE_DYNCREATE(RadarDoc)

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~RadarDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

private:
	//BOOL GetCPUID(char *pID);         //获取CPUID
	BOOL GetDiskID(char *pID);        //获取磁盘序列号，多个磁盘，则串联返回(从小到大排序后串联)
	//BOOL GetMAC(char *pID);           //获取MAC地址，多个MAC地址，则串联返回(从小到大排序后串联)
	BOOL CalculateSN(char *pAllID);   //计算注册码

};

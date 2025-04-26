#pragma once

#include "stdafx.h"
#include "resource.h" 

class RadarViewSetting : public CFormView
{
	DECLARE_DYNCREATE(RadarViewSetting)
public:
	RadarViewSetting();
	~RadarViewSetting();

	CSliderCtrl m_oContrast;
	CSliderCtrl m_oTimeGain;
	CSliderCtrl m_oDeep;

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD=IDD_VIEW_SETTING };
#endif

	static int GetContrast()
	{
		return RadarViewSetting::m_iContrast;
	}
	static int GetTimegain()
	{
		return RadarViewSetting::m_iTimegain;
	}
	static int GetDeep()
	{
		return RadarViewSetting::m_iDeep;
	}
	static BOOL GetAuto()
	{
		return m_Auto;
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void OnInitialUpdate();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedSettingOpenLeft();
	afx_msg void OnBnClickedSettingOpenRight();
	afx_msg void OnBnClickedSliderDeepAuto();

private:
	static int m_iContrast;    //�Աȶ�
	static int m_iTimegain;    //ʱ������
	static int m_iDeep;        //��ƽ��ȣ���С�ֱ���Ϊ1cm���ֶ�
	static BOOL m_Auto;        //�Ƿ��Զ���ƽ

};

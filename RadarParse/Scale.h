#pragma once
/////////////////////////////////////////////////////////////////////////////

#include <windef.h>
#include "stdafx.h"
//#include "FSize.h"
#include "Scale.h"

class Scale
{
public:
	Scale()
	{
	}

	~Scale()
	{
		TRACE("Scale destructor\n");
	}

	void sInit(CRect rect, CRect vis, FSize units, int zero)
	{
		m_rect = rect;
		m_vis = vis;
		m_units = units;
		sZeroLevel = zero;
	}

	void SetTitle(BOOL b, CString str)
	{
		if (b)
			HorzTitle = str;
		else
			VertTitle = str;
	}

	//绘制深度标尺上的红色三角形
	void ShowZero(CDC* pDC, BOOL dir, CPoint loc, COLORREF color)
	{
		CBrush* oldbrush, brush;
		CPoint points[3];
		if (dir)
		{
			points[0] = CPoint(loc.x, loc.y + 6);
			points[1] = CPoint(loc.x, loc.y - 6);
			points[2] = CPoint(loc.x + 10, loc.y);
		}
		else
		{
			points[0] = CPoint(loc.x + 10, loc.y + 6);
			points[1] = CPoint(loc.x + 10, loc.y - 6);
			points[2] = CPoint(loc.x, loc.y);
		}


		brush.CreateSolidBrush(color);
		oldbrush = pDC->SelectObject(&brush);
		pDC->Polygon(points, 3);
		pDC->SelectObject(oldbrush);
		brush.DeleteObject();
	}

	void Draw(CDC* pDC, int iStartPosX , int iStartPosY)
	{
		int x, y;
		FSize units;
		CString s, s0, s1;
		CPen *oldpen, pen;
		CFont *oldfont, hfont, vfont;
		LOGFONT logfont;
		//
		if ((m_units.getCX() == 0) || (m_units.getCY() == 0) || (m_vis.Width() == 0) || (m_vis.Height() == 0))
			return;

		//绘制文字
		memset(&logfont, 0, sizeof(LOGFONT));
		_tcscpy(logfont.lfFaceName, _T("Arial"));
		logfont.lfHeight = 14;
		logfont.lfEscapement = 0;
		//
		if (hfont.CreateFontIndirect(&logfont) == 0)
		{
			s0 = "设置字体错误";
			s1="错误提示";
			MessageBox(NULL, s0, s1, MB_OK | MB_TASKMODAL);
		}
		logfont.lfHeight = 12;
		logfont.lfEscapement = 900;
		if (vfont.CreateFontIndirect(&logfont) == 0)
		{
			s0 = "设置字体错误";
			s1 = "错误提示";
			MessageBox(NULL, s0, s1, MB_OK | MB_TASKMODAL);
		}

		pDC->SetTextColor(RGB(0, 0, 0));
		pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		oldpen = pDC->SelectObject(&pen);
		SetBkMode(*pDC, TRANSPARENT);		    //设置界面上写字时，背景透明

		//DRAW SCALES
		CRect rc = m_rect;
		rc.InflateRect(5, 5);
		POINT oPoint = rc.TopLeft(); oPoint.y = oPoint.y - iStartPosY;
		pDC->MoveTo(oPoint);
		pDC->LineTo(rc.right, rc.top - iStartPosY );          //绘制水平比例尺的直线
		pDC->MoveTo(oPoint);
		pDC->LineTo(rc.left, rc.bottom - iStartPosY);         //绘制垂直比例尺的直线

		pDC->SetTextAlign(TA_CENTER);
		oldfont = pDC->SelectObject(&vfont);
		pDC->TextOut(rc.left - 40, (rc.top + rc.bottom) / 2 - iStartPosY , VertTitle);    //绘制垂直的比例单位

		pDC->SetTextAlign(TA_LEFT);
		pDC->SelectObject(&hfont);
		pDC->TextOut(rc.left , rc.top - 30 - iStartPosY, HorzTitle);     //绘制水平的比例文本单位

		//////////////////////////// DRAW ZERO LEVEL ///////////////////////////////
		if (sZeroLevel > m_vis.top)
		{
			ShowZero(pDC, TRUE , CPoint(rc.left  - 30, m_rect.top + sZeroLevel - m_vis.top - iStartPosY), RGB(255, 0, 0));
			//ShowZero(pDC, FALSE, CPoint(rc.right + 30, m_rect.top + sZeroLevel - m_vis.top - iStartPosY), RGB(255, 0, 0));
		}

		/////////////////////////// DRAW VERTICAL SCALE UNDER ZERO ////////////////
		pDC->SetTextAlign(TA_RIGHT);
		CString res;
		double st = 0;
		bool ln;
		//绘制深度坐标
		for (y = m_rect.top + sZeroLevel; y < m_rect.bottom; y++, st += m_units.getCY())
		{
			res = "";
			ln = false;

			if (m_units.getCY() <= 0.005)
			{
				if (int(st * 10000 + 0.001) % 5000 < int(m_units.getCY() * 10010)) res.Format(_T("%3.1f"), st);
				if (int(st * 10000 + 0.001) % 1000 < int(m_units.getCY() * 10010)) ln = true;
			}
			else if (m_units.getCY() <= 0.01)
			{
				//TRACE("scale: %3.3f %d %3.3f\n", st, int(st * 10000) % 100000, m_units.getCY() * 10000);
				if (int(st * 10000 + 0.001) % 10000 < int(m_units.getCY() * 10010)) res.Format(_T("%3.0f"), st);
				if (int(st * 10000 + 0.001) % 1000 < int(m_units.getCY() * 10010)) ln = true;
			}
			else if (m_units.getCY() <= 0.05)
			{
				if (int(st * 10000 + 0.001) % 10000 < int(m_units.getCY() * 10010)) res.Format(_T("%3.0f"), st);
				if (int(st * 10000 + 0.001) % 5000 < int(m_units.getCY() * 10010)) ln = true;
			}
			else if (m_units.getCY() <= 0.1)
			{
				if (int(st * 1000 + 0.001) % 10000 < int(m_units.getCY() * 1001)) res.Format(_T("%3.0f"), st);
				if (int(st * 1000 + 0.001) % 5000 < int(m_units.getCY() * 1001)) ln = true;
			}
			else if (m_units.getCY() <= 0.5)
			{
				if (int(st * 1000 + 0.001) % 50000 < int(m_units.getCY() * 1001)) res.Format(_T("%3.0f"), st);
				if (int(st * 1000 + 0.001) % 10000 < int(m_units.getCY() * 1001)) ln = true;
			}
			else if (m_units.getCY() <= 1.0)
			{
				if (int(st * 1000 + 0.001) % 100000 < int(m_units.getCY() * 1001)) res.Format(_T("%d"), ((int)(st / 10.0)) * 10);
				if (int(st * 1000 + 0.001) % 50000 < int(m_units.getCY() * 1001)) ln = true;
			}
			else if (m_units.getCY() <= 5.0)
			{
				if (int(st * 100 + 0.001) % 50000 < int(m_units.getCY() * 100)) res.Format(_T("%d"), ((int)(st / 10.0)) * 10);
				if (int(st * 100 + 0.001) % 10000 < int(m_units.getCY() * 100)) ln = true;
			}
			else if (m_units.getCY() <= 10.0)
			{
				if (int(st * 100 + 0.001) % 100000 < int(m_units.getCY() * 100)) res.Format(_T("%d"), ((int)(st / 100.0)) * 100);
				if (int(st * 100 + 0.001) % 50000 < int(m_units.getCY() * 100)) ln = true;
			}
			else
			{
				if (int(st * 10 + 0.001) % 100000 < int(m_units.getCY() * 10)) res.Format(_T("%d"), ((int)(st / 1000.0)) * 1000);
				if (int(st * 10 + 0.001) % 10000 < int(m_units.getCY() * 10)) ln = true;
			}

			if (ln)
			{
				//绘制深度坐标的刻度线
				pDC->MoveTo(rc.left - 5, y - iStartPosY);
				pDC->LineTo(rc.left, y - iStartPosY);
			}
			if (res != "")
			{
				//绘制深度坐标的文本，以及文本行对应的刻度线
				pDC->MoveTo(rc.left - 7, y - iStartPosY);
				pDC->LineTo(rc.left, y - iStartPosY);
				if (y < m_rect.bottom - 10)
				{
					pDC->SetTextAlign(TA_RIGHT);
					pDC->TextOut(rc.left - 10, y - 5 - iStartPosY, res);
				}
			}
		}

		///////////////////////////// DRAW HORIZONTAL SCALE ///////////////////////////
		pDC->SetTextAlign(TA_CENTER);
		st = iStartPosX * m_units.getCX();
		//绘制水平座标
		for (x = m_rect.left; x < m_rect.right; x++, st += m_units.getCX())
		{
			res = "";
			ln = false;

			if (m_units.getCX() <= 0.005)
			{
				if (int(st * 10000 + 0.001) % 5000 < int(m_units.getCX() * 10010)) res.Format(_T("%3.1f"), st);
				if (int(st * 10000 + 0.001) % 1000 < int(m_units.getCX() * 10010)) ln = true;
			}
			else if (m_units.getCX() <= 0.01)
			{
				if (int(st * 10000 + 0.001) % 10000 < int(m_units.getCX() * 10010)) res.Format(_T("%3.0f"), st);
				if (int(st * 10000 + 0.001) % 1000 < int(m_units.getCX() * 10010)) ln = true;
			}
			else if (m_units.getCX() <= 0.05)
			{
				if (int(st * 10000 + 0.001) % 10000 < int(m_units.getCX() * 10010)) res.Format(_T("%3.0f"), st);
				if (int(st * 10000 + 0.001) % 5000 < int(m_units.getCX() * 10010)) ln = true;
			}
			else if (m_units.getCX() <= 0.1)
			{
				if (int(st * 1000 + 0.001) % 10000 < int(m_units.getCX() * 1001)) res.Format(_T("%3.0f"), st);
				if (int(st * 1000 + 0.001) % 5000 < int(m_units.getCX() * 1001)) ln = true;
			}
			else if (m_units.getCX() <= 0.5)
			{
				if (int(st * 1000 + 0.001) % 50000 < int(m_units.getCX() * 1001)) res.Format(_T("%3.0f"), st);
				if (int(st * 1000 + 0.001) % 10000 < int(m_units.getCX() * 1001)) ln = true;
			}
			else if (m_units.getCX() <= 1.0)
			{
				if (int(st * 1000 + 0.001) % 100000 < int(m_units.getCX() * 1001)) res.Format(_T("%d"), ((int)(st / 10.0)) * 10);
				if (int(st * 1000 + 0.001) % 50000 < int(m_units.getCX() * 1001)) ln = true;
			}
			else if (m_units.getCX() <= 5.0)
			{
				if (int(st * 100 + 0.001) % 50000 < int(m_units.getCX() * 100)) res.Format(_T("%d"), ((int)(st / 10.0)) * 10);
				if (int(st * 100 + 0.001) % 10000 < int(m_units.getCX() * 100)) ln = true;
			}
			else if (m_units.getCX() <= 10.0)
			{
				if (int(st * 100 + 0.001) % 100000 < int(m_units.getCX() * 100)) res.Format(_T("%d"), ((int)(st / 100.0)) * 100);
				if (int(st * 100 + 0.001) % 50000 < int(m_units.getCX() * 100)) ln = true;
			}
			else
			{
				if (int(st * 10 + 0.001) % 100000 < int(m_units.getCX() * 10)) res.Format(_T("%d"), ((int)(st / 1000.0)) * 1000);
				if (int(st * 10 + 0.001) % 10000 < int(m_units.getCX() * 10)) ln = true;
			}

			if (ln)
			{
				//绘制水平座标上的刻度
				pDC->MoveTo(x, rc.top - 5 - iStartPosY);
				pDC->LineTo(x, rc.top     - iStartPosY);
			}
			if (res != "")
			{
				//绘制水平座标上的文字，以及对应的刻度
				pDC->MoveTo(x, rc.top - 7 - iStartPosY);
				pDC->LineTo(x, rc.top     - iStartPosY);
				if (x < m_rect.right - 10)
				{
					pDC->SetTextAlign(TA_RIGHT);
					pDC->TextOut(x + 5, rc.top - 17 - iStartPosY, res);
				}
			}
		}

		pDC->SelectObject(oldpen);
		pen.DeleteObject();
		pDC->SelectObject(oldfont);
		hfont.DeleteObject();
		vfont.DeleteObject();
		pDC->SetTextAlign(TA_LEFT);
	}

private:
	CRect m_rect;
	CRect m_vis;
	FSize m_units;
	CString HorzTitle;
	CString VertTitle;

	int sZeroLevel;
	int vertnum;
	int horznum;
};

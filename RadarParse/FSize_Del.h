#pragma once

///////////////////////////////////////////////////////////////////////////////////

#include <windef.h>
#include "stdafx.h"



class FSize
{
public:
    FSize(){}

    FSize(const FSize& src)
    {
        cx = src.cx;
        cy = src.cy;
    }
    
    FSize(const CSize& src)
    {
        cx = src.cx;
        cy = src.cy;
    }
    
    FSize(const CPoint& src)
    {
        cx = src.x;
        cy = src.y;
    }
    
    FSize(int x, int y)
    {
        cx = x;
        cy = y;
    }
    
    FSize(double x, double y)
    {
        cx = x;
        cy = y;
    }
    
    FSize operator =(const FSize& src)
    {
        cx = src.cx;
        cy = src.cy;
        return *this;
    }
    
    FSize operator +(const FSize& op1)
    {
        return FSize(cx + op1.cx, cy + op1.cy);
    }
    
    FSize operator +(const CPoint& op1)
    {
        return FSize(cx + op1.x, cy + op1.y);
    }
    
    void operator +=(const FSize& op1)
    {
        cx += op1.cx; cy += op1.cy;
    }
    
    void operator +=(const CPoint& op1)
    {
        cx += op1.x; cy += op1.y;
    }
    
    FSize operator -(const FSize& op1)
    {
        return FSize(cx - op1.cx, cy - op1.cy);
    }
    
    FSize operator *(const FSize& op1)
    {
        return FSize(cx * op1.cx, cy * op1.cy);
    }
    
    FSize operator *(const double& f)
    {
        return FSize(cx*f, cy*f);
    }
    
    FSize operator /(const double& f)
    {
        return FSize(cx / f, cy / f);
    }
    
    FSize operator /(const FSize& op)
    {
        return FSize(cx / op.cx, cy / op.cy);
    }
    
    void operator *=(const CSize& op)
    {
        cx *= op.cx; cy *= op.cy;
    }
    
    void operator *=(const double& f)
    {
        cx *= f; cy *= f;
    }
    
    void operator /=(const FSize& op)
    {
        cx /= op.cx; cy /= op.cy;
    }
    
    void operator /=(const CSize& op)
    {
        cx /= double(op.cx); cy /= double(op.cy);
    }

	double getCX()
	{
		return cx;
	}
	double getCY()
	{
		return cy;
	}
private:
	double cx;
	double cy;
};

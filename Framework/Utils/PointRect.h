#pragma once


//  2D point.

struct Point
{
	int x, y;

	Point() = default;

	Point(int _x, int _y)
	{
		x = _x,  y = _y;
	}

	inline Point& operator += (const Point& p)
	{
		x += p.x,  y += p.y;
		return *this;
	}

	inline Point& operator -= (const Point& p)
	{
		x -= p.x,  y -= p.y;
		return *this;
	}

	inline Point operator + (const Point& p )  const
	{
		Point s(x + p.x, y + p.y);
		return s;
	}

	inline Point operator - (const Point& p)  const
	{
		Point s(x - p.x, y - p.y );
		return s;
	}
};

typedef const Point& CPoint;  // We dont use the MFC.




//  2D rectangle. It is not the Windows RECT - x2, y2 exactly lie on plain. (For example x1=0, x2=9, so width=10).

struct Rect
{
#pragma warning( disable: 4201 )
	union
	{	struct
		{	int  x1, y1, x2, y2;
        };
		struct
		{	Point p1, p2;
		};
    };
#pragma warning( default: 4201 )

	Rect() = default;

	Rect(int left, int top, int right, int bottom)
	{
		x1 = left,  y1 = top,  x2 = right;  y2 = bottom;
	}

	Rect(CPoint pos1, CPoint pos2)
	{
		p1 = pos1,  p2 = pos2;
	}

	int width()  const
	{
		return (x2 - x1 + 1);  // +1 - It is not the Windows RECT.
	}

	int height()  const
	{
		return (y2 - y1 + 1);  // +1 - It is not the Windows RECT.
	}

	Point getWH()  const
	{
		Point p(width(), height());
		return p;
	}

	inline Rect& operator += (const Point& p)
	{
		p1 += p,  p2 += p;
		return *this;
	}

	inline Rect& operator -= (const Point& p)
	{
		p1 -= p,  p2 -= p;
		return *this;
	}

	bool isPointInside(CPoint p)  const
	{
		return (p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2);
	}

	void  setPos(CPoint p)
	{
		int w = width();
		int h = height();

		p1 = p;

		x2 = x1 + w - 1;
		y2 = y1 + h - 1;
	}
};

typedef const Rect& CRect;  // We dont use the MFC.
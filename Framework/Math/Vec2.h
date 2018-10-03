#pragma once

#include <windows.h>


struct Vec2
{
	float  x, y;

	Vec2() = default;

	Vec2(float _x, float _y)
	{
		x = _x;  y = _y;
	}

	inline Vec2& operator *= ( const Vec2& v )
	{
		x *= v.x,  y *= v.y;
		return *this;
	}

	inline Vec2& operator /= ( const Vec2& v )
	{
		x /= v.x,  y /= v.y;
		return *this;
	}

	inline Vec2& operator += ( const Vec2& v )
	{
		x += v.x,  y += v.y;
		return *this;
	}

	inline Vec2& operator -= ( const Vec2& v )
	{
		x -= v.x,  y -= v.y;
		return *this;
	}
};

typedef const Vec2& CVec2;

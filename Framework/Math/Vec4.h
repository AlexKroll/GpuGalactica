#pragma once

#include <windows.h>

#include "../Defines.h"

#ifdef SSE
	#include "Vec4Sse.h"
#endif

#ifdef NEON
	#include "Vec4Neon.h"
#endif


#ifdef FPU


struct Vec4
{
	float  x, y, z, w;

	Vec4() = default;

	Vec4(float _x, float _y, float _z, float _w)
	{
		x = _x;  y = _y;  z = _z;  w = _w;
	}

	inline Vec4& operator *= (const Vec4& v)
	{
		x *= v.x,  y *= v.y,  z *= v.z,  w *= v.w;
		return *this;
	}

	inline Vec4& operator /= (const Vec4& v)
	{
		x /= v.x,  y /= v.y,  z /= v.z,  w /= v.w;
		return *this;
	}

	inline Vec4& operator += (const Vec4& v)
	{
		x += v.x,  y += v.y,  z += v.z,  w += v.w;
		return *this;
	}

	inline Vec4& operator -= (const Vec4& v)
	{
		x -= v.x,  y -= v.y,  z -= v.z,  w -= v.w;
		return *this;
	}

	inline Vec4 operator + (const Vec4& v) const
	{
		Vec4 v2(x + v.x, y + v.y, z + v.z, w + v.w);
		return v2;
	}

	inline Vec4 operator - (const Vec4& v) const
	{
		Vec4 v2(x - v.x, y - v.y, z - v.z, w - v.w);
		return v2;
	}

	inline Vec4 operator * (const Vec4& v) const
	{
		Vec4 v2(x * v.x, y * v.y, z * v.z, w * v.w);
		return v2;
	}

	inline Vec4 operator / (const Vec4& v) const
	{
		Vec4 v2(x / v.x, y / v.y, z / v.z, w / v.w);
		return v2;
	}

	inline Vec4 operator * (float f) const
	{
		Vec4 v(x * f, y * f, z * f, w);
		return v;
	}

	inline void normalize()
	{
		float f = 1.0f / sqrtf( x * x + y * y + z * z );
		x *= f,  y *= f,  z *= f;
	}

	inline float length()
	{
		float len = sqrtf( x * x + y * y + z * z );
		return len;
	}

	inline float length_normalize()
	{
		float len = sqrtf( x * x + y * y + z * z );
		float f = 1.0f / len;
		x *= f,  y *= f,  z *= f;
		return len;
	}

	inline Vec4 negate()
	{
		Vec4 v(-x, -y, -z, w);
		return v;
	}
};

typedef const Vec4& CVec4;



inline Vec4 Cross(CVec4 v1, CVec4 v2)
{
	Vec4 out;

	out.x = v1.y * v2.z - v1.z * v2.y;
	out.y = v1.z * v2.x - v1.x * v2.z;
	out.z = v1.x * v2.y - v1.y * v2.x;
	out.w = 0.0f;

	return out;
}



inline float Dot3(CVec4 v1, CVec4 v2)
{
	float out;

	out = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;

	return out;
}


#endif
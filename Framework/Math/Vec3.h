#pragma once

#include <windows.h>


struct Vec3
{
	float  x, y, z;

	Vec3() = default;

	Vec3(float _x, float _y, float _z)
	{
		x = _x;  y = _y;  z = _z;
	}

	inline Vec3& operator *= ( const Vec3& v )
	{
		x *= v.x,  y *= v.y,  z *= v.z;
		return *this;
	}

	inline Vec3& operator /= ( const Vec3& v )
	{
		x /= v.x,  y /= v.y,  z /= v.z;
		return *this;
	}

	inline Vec3& operator += ( const Vec3& v )
	{
		x += v.x,  y += v.y,  z += v.z;
		return *this;
	}

	inline Vec3& operator -= ( const Vec3& v )
	{
		x -= v.x,  y -= v.y,  z -= v.z;
		return *this;
	}
};

typedef const Vec3& CVec3;


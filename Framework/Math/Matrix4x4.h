#pragma once

#include "../Defines.h"



struct Matrix4x4
{
#pragma warning( disable: 4201 )
	union
	{	struct
		{	float  _11, _12, _13, _14;
            float  _21, _22, _23, _24;
            float  _31, _32, _33, _34;
            float  _41, _42, _43, _44;

        };
		struct
		{	float m[4][4];
		};
    };
#pragma warning( default: 4201 )

	Matrix4x4() = default;

	Matrix4x4(float f11, float f12, float f13, float f14,
			  float f21, float f22, float f23, float f24,
			  float f31, float f32, float f33, float f34,
			  float f41, float f42, float f43, float f44)
	{
		_11 = f11,  _12 = f12,  _13 = f13,  _14 = f14;
		_21 = f21,  _22 = f22,  _23 = f23,  _24 = f24;
		_31 = f31,  _32 = f32,  _33 = f33,  _34 = f34;
		_41 = f41,  _42 = f42,  _43 = f43,  _44 = f44;
	}

	Matrix4x4 transpose()
	{
		Matrix4x4 out;

		for (int i = 0; i < 4; ++i)
		{	for (int j = 0; j < 4; ++j)
				out.m[i][j] = m[j][i];
		}

		return out;
	}
};

typedef const Matrix4x4& CMatrix4x4;




inline Matrix4x4 MatrixMultiply(CMatrix4x4 m1, CMatrix4x4 m2)
{
	Matrix4x4 out;

	out._11 = m1._11 * m2._11 + m1._12 * m2._21 + m1._13 * m2._31 + m1._14 * m2._41;
	out._12 = m1._11 * m2._12 + m1._12 * m2._22 + m1._13 * m2._32 + m1._14 * m2._42;
	out._13 = m1._11 * m2._13 + m1._12 * m2._23 + m1._13 * m2._33 + m1._14 * m2._43;
	out._14 = m1._11 * m2._14 + m1._12 * m2._24 + m1._13 * m2._34 + m1._14 * m2._44;

	out._21 = m1._21 * m2._11 + m1._22 * m2._21 + m1._23 * m2._31 + m1._24 * m2._41;
	out._22 = m1._21 * m2._12 + m1._22 * m2._22 + m1._23 * m2._32 + m1._24 * m2._42;
	out._23 = m1._21 * m2._13 + m1._22 * m2._23 + m1._23 * m2._33 + m1._24 * m2._43;
	out._24 = m1._21 * m2._14 + m1._22 * m2._24 + m1._23 * m2._34 + m1._24 * m2._44;

	out._31 = m1._31 * m2._11 + m1._32 * m2._21 + m1._33 * m2._31 + m1._34 * m2._41;
	out._32 = m1._31 * m2._12 + m1._32 * m2._22 + m1._33 * m2._32 + m1._34 * m2._42;
	out._33 = m1._31 * m2._13 + m1._32 * m2._23 + m1._33 * m2._33 + m1._34 * m2._43;
	out._34 = m1._31 * m2._14 + m1._32 * m2._24 + m1._33 * m2._34 + m1._34 * m2._44;

	out._41 = m1._41 * m2._11 + m1._42 * m2._21 + m1._43 * m2._31 + m1._44 * m2._41;
	out._42 = m1._41 * m2._12 + m1._42 * m2._22 + m1._43 * m2._32 + m1._44 * m2._42;
	out._43 = m1._41 * m2._13 + m1._42 * m2._23 + m1._43 * m2._33 + m1._44 * m2._43;
	out._44 = m1._41 * m2._14 + m1._42 * m2._24 + m1._43 * m2._34 + m1._44 * m2._44;

	return out;
}
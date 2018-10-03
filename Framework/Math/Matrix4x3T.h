#pragma once

#include "Vec4.h"

#ifdef SSE
	#include "Matrix4x3TSse.h"
#endif

#ifdef NEON
	#include "Matrix4x3TNeon.h"
#endif


#ifdef FPU


// Transposed matrix of the world transformation.
//
//   Orientation 		Position		(No scaling, it is in the vector of the object)
//   _11, _12, _13		_14
//   _21, _22, _23		_24
//   _31, _32, _33		_34

struct __declspec(align(16)) Matrix4x3T
{
#pragma warning( disable: 4201 )
	union
	{	struct
		{	float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
		};
		struct
		{	Vec4 v[3];
		};
	};
#pragma warning( default: 4201 )


	inline void setPos(CVec4 pos)
	{
		_14 = pos.x,  _24 = pos.y,  _34 = pos.z;
	}


	inline void setOrient(CVec4 orient)
	{
		float sinz = sinf(orient.z),  cosz = cosf(orient.z);	// Roll.
		float sinx = sinf(-orient.x),  cosx = cosf(-orient.x);	// Pitch.
		float siny = sinf(orient.y),  cosy = cosf(orient.y);	// Yaw.

		_11 =  cosy * cosz + siny * sinx * sinz;
		_12 = -sinz * cosy + siny * sinx * cosz;
		_13 =  siny * cosx;

		_21 =  cosx * sinz;
		_22 =  cosx * cosz;
		_23 = -sinx;

		_31 = -siny * cosz + cosy * sinx * sinz;
		_32 =  siny * sinz + cosy * sinx * cosz;
		_33 =  cosy * cosx;
	}


	inline Vec4 transformCoord(CVec4 vec)
	{
		Vec4 out;

		Vec4 m0 = vec * v[0];
		Vec4 m1 = vec * v[1];
		Vec4 m2 = vec * v[2];

		out.x = m0.x + m0.y + m0.z + m0.w;
		out.y = m1.x + m1.y + m1.z + m1.w;
		out.z = m2.x + m2.y + m2.z + m2.w;

		return out;
	}
};


#endif
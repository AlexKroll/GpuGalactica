#pragma once

#include "Matrix4x3T.h"
#include "Color.h"



inline void OrientFromDir(Vec4& orient, CVec4 dir)   // Calculation of the pitch & yaw angles.
{
    orient.x = asinf(dir.y);
	orient.y = atan2f(dir.x, dir.z);
}



////////////////  Random numbers  ///////////////////

const float kInvRandMax = 1.0f / ( (float)RAND_MAX+1.0f );

inline void InitRandomGenerator()
{
	LARGE_INTEGER cnt;
	QueryPerformanceCounter(&cnt);
	DWORD low = cnt.LowPart;
	low &= RAND_MAX;
	srand(low);
}


inline WORD Random(WORD num1, WORD num2)
{
	int ival = rand();
	float fval = static_cast<float>(ival) * kInvRandMax;
	int dist = (num2+1) - num1;
	fval *= static_cast<float>(dist);
	ival = static_cast<int>(fval);
	ival += num1;
	
	return static_cast<WORD>(ival);
}


inline float Random01()  // number range is [0..1]
{
	int ival = rand();
	float fval = static_cast<float>(ival) * kInvRandMax;
	return fval;
}


inline Vec4 V4Random01()
{
	Vec4 v;

	v.x = Random01();
	v.y = Random01();
	v.z = Random01();
	v.w = Random01();

	return v;
}

/////////////////////////////////////////////////////
#include "Timer.h"



Timer* Timer::getInstance()
{
	static Timer* pTimer = nullptr;
	if (nullptr == pTimer)
		pTimer = new Timer;

	return pTimer;
}


Timer::Timer()
{
	// Do it for windows platforms only.
	BOOL bRes = QueryPerformanceFrequency( &freqPerSec_ );
	if (!bRes) // QPC is not supported.
	{
		freqPerSec_.QuadPart = 0;

		int res = ::GetLastError();
		if (res)
		{
		}
	}
}


void Timer::startMeasure()
{
	if (freqPerSec_.QuadPart)
	{
		::QueryPerformanceCounter( &startTiks_ );
	}
}


float Timer::getMeasuredTime()
{
	if (freqPerSec_.QuadPart)
	{
		LARGE_INTEGER tiks;
		::QueryPerformanceCounter( &tiks );

		tiks.QuadPart = tiks.QuadPart - startTiks_.QuadPart;

		double time = static_cast<double>(tiks.QuadPart);
		time = time / static_cast<double>(freqPerSec_.QuadPart);

		float ftime = static_cast<float>(time);
		if (ftime > 0.1f)  // Limit the FPS to 10.
			ftime = 0.1f;

		return ftime;
	}
	else
	{
		return 1.0f / 60.0f;  // Return the fixed time for 60 FPS.
	}
}
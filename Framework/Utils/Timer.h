#pragma once

#include <windows.h>


class Timer
{
public:
	static Timer* getInstance();

	void startMeasure();

	float getMeasuredTime();

private:
	Timer();

	LARGE_INTEGER freqPerSec_;

	LARGE_INTEGER startTiks_;

	float lowLimitTime_ = 0.1f;  // Lowest FPS is 10.
};

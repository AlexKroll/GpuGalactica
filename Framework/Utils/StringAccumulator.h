#pragma once

#include <windows.h>
#include <string>

#include "../Defines.h"


//  Static class to accumulate the string from different data: text, int, float etc.
//  The class uses input data as template variadic parameters.

class StringAccumulator
{
public:
	template <typename T, typename... Ts>
	static void AddErrorStr(std::string& text, T x, Ts... xs)
	{
		AddErrorStr(text, x);
		AddErrorStr(text, xs...);
	}

private:
	template <typename... Args>
	static void AddErrorStr(std::string& text, Args... args)
	{
		AddErrorStr(text, args...);
	}


	static void AddErrorStr(std::string& text, int Value)
	{
		char c_dig[64];
		sprintf_s(c_dig, "%d", Value);

		text += c_dig;
	}


	static void AddErrorStr(std::string& text, float Value)
	{
		char c_dig[64];
		sprintf_s(c_dig, "%.3f", Value);

		text += c_dig;
	}


	static void AddErrorStr(std::string& text, const char* pText)
	{
		text += pText;
	}


	template <typename T>
	static void AddErrorStr(T Value)
	{
		Value;
	}
};


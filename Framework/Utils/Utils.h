#pragma once

#include <windows.h>

#include "StringAccumulator.h"




template <typename... Args>
void ShowError(HWND hWindow, Args... args)				// Accumulates the string from variadic parameters
{														// and shows into message dialog.
	std::string text;

	StringAccumulator::AddErrorStr(text, args...);

	MessageBox(hWindow, text.c_str(), "Error", MB_OK);
}



class FileAutoClose
{
public:
	FileAutoClose() = default;

	FileAutoClose(FILE* hf)
	{
		mFile = hf;
	}

	~FileAutoClose()
	{
		if (mFile)
			fclose(mFile);
	}

	operator FILE* ()
	{
		return mFile;
	}

	FILE** getAddrOf()
	{
		return &mFile;
	}

	FILE* mFile = nullptr;
};
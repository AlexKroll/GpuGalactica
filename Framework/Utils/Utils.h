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
		pFile_ = hf;
	}

	~FileAutoClose()
	{
		if (pFile_)
			fclose(pFile_);
	}

	operator FILE* ()
	{
		return pFile_;
	}

	FILE** getAddrOf()
	{
		return &pFile_;
	}

	FILE* pFile_ = nullptr;
};



__int64 MyGetFileSize(PCCHAR filename);

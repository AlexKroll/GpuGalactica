#include "Utils.h"



__int64 MyGetFileSize(PCCHAR filename)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;

    if (!::GetFileAttributesEx(filename, GetFileExInfoStandard, &fad))
	{
		DWORD error = ::GetLastError();
		if (error)
		{	error = 0;
		}
		return -1;
	}

    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}
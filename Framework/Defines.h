#pragma once

#include <windows.h>
#include <string>
#include <memory>
#include <vector>


typedef const TCHAR *PCCHAR;

typedef const WCHAR *PCWCHAR;

typedef const std::string& cstring;


#define SAFE_RELEASE(p)	    { if (p)  { (p)->release();  (p) = nullptr; }  }

#define SAFE_RELEASE_DX(p)  { if (p)  { (p)->Release();  (p) = nullptr; }  }


#define SAFE_DELETE(p)		  {	if (p)  { delete (p);  (p) = nullptr; }  }

#define SAFE_DELETE_ARRAY(p)  { if (p)  { delete[] (p);  (p) = nullptr; }  }


const int32_t kUndefInteger = 0xCCCCCCCC;


#ifdef SIMD
	#define SPEC_ALIGN16 __declspec(align(16))
#else
	#define SPEC_ALIGN16
#endif


#define ERROR_UNINITIALIZED 1

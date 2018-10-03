#pragma once

#include <windows.h>


struct Color
{
#pragma warning( disable: 4201 )
	union
	{
		struct
		{	float r, g, b, a;
		};
		struct
		{	float f[4];
		};
	};
#pragma warning( default: 4201 )

	Color() = default;

	Color (float _r, float _g, float _b, float _a)
	{
        r = _r,  g = _g,  b = _b,  a = _a;
	}

	Color (uint32_t color)
	{
		BYTE* pbytes = reinterpret_cast<BYTE*>(&color);
		for (int i = 0; i < 4; ++i)
			f[i] = static_cast<float>(pbytes[i]) / 255.0f;
	}

	operator float* ()
	{
		return &r;
	}

	operator const float* ()  const
	{
		return &r;
	}


	inline Color operator + (const Color& color)  const
	{
        Color out;
		for (int i = 0; i < 4; ++i)
			out.f[i] = f[i] + color.f[i];
		return out;
	}

	inline Color operator - (const Color& color)  const
	{
        Color out;
		for (int i = 0; i < 4; ++i)
			out.f[i] = f[i] - color.f[i];
		return out;
	}


	inline Color operator * (const float value)  const
	{
		Color out;
		for (int i = 0; i < 4; ++i)
			out.f[i] = f[i] * value;
		return out;
	}


	inline Color operator * (const uint32_t value)  const
	{
		Color clr_value = value;
		Color out;
		for (int i = 0; i < 4; ++i)
			out.f[i] = f[i] * clr_value.f[i];
		return out;
	}


	inline void store(uint32_t& color)  const
	{
		BYTE* pbytes = reinterpret_cast<BYTE*>(&color);
		Color temp;

		for (int i = 0; i < 4; ++i)
		{	temp.f[i] = f[i] * 255.0f;
			if (temp.f[i] > 255.0f)
				temp.f[i] = 255.0f;

			pbytes[i] = static_cast<BYTE>(temp.f[i]);
		}
	}
};

typedef const Color& CColor;
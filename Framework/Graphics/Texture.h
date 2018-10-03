#pragma once

#include "../Defines.h"


class ITexture abstract
{
public:
	enum PixelFormat
	{
		UnknownFormat = 0,		// Unknown or unsupported format.
		ColorRGBA8 = 1,			// 32-bit, 8-bit per channel.
		ColorRGB8 = 2,			// 24-bit,8-bit per channel. 
		ColorR5G6B5 = 3,		// 16-bit, 5Red-, 6Green-, 5Blue- bits.
		ColorR8 = 4,			// 8-bit red channel.
		ColorR16F = 5,			// 16-bit float red channel.
		ColorR32F = 6,			// 32-bit float red channel.
		ColorA8 = 7,			// 8-bit alpha channel.
		ColorBGRA8 = 8,			// 32-bit, 8-bit per channel (Blue, Green, Red, Alpha order).
		Depth16 = 9,			// 16-bit depth.
		Depth32 = 10,			// 32-bit depth.
		Float16_4 = 11,			// 4x 16-bits floats [0..1]
		LastFormat = 12,		// Unusable format to iterate device formats.
		MaxFormats = 16
	};

	enum Type
	{
		StaticTexture = 1,		// Texture is in GPU memory. GPU uses it as source.
		RenderTarget = 2,		// Texture is in GPU memory. GPU uses it as destination.
		DynamicTexture = 3,		// Texture is in CPU-GPU memory. CPU writes / GPU reads the texture.
		GdiCompatible = 4		// Texture is used for GDI drawing (text, figures, etc).
	};

	HRESULT getDimentions(int& width, int& height) const;

	PixelFormat getFormat() const;

	static int getBytesPerPixel(PixelFormat format);

	virtual void release() = 0;

protected:
	ITexture() = default;
	virtual ~ITexture() = default;

	int width_ = 0;
	int height_ = 0;

	PixelFormat pixelFormat_ = UnknownFormat;

	UINT multisampleCount_ = 1;
	UINT multisampleQuality_ = 0;
};

//typedef ITexture* Texture;
typedef std::shared_ptr<ITexture> Texture;
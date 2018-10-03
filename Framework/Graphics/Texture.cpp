#include "Texture.h"



HRESULT ITexture::getDimentions(int& width, int& height) const
{
	width = width_;
	height = height_;

	return S_OK;
}


ITexture::PixelFormat ITexture::getFormat() const
{
	return pixelFormat_;
}



int ITexture::getBytesPerPixel(PixelFormat format)
{
	switch(format)
	{
		case ColorRGBA8:
		case ColorBGRA8:
		case Depth32:
		case ColorR32F:
			return 4;

		case ColorRGB8:
			return 3;

		case ColorR5G6B5:
		case ColorR16F:
		case Depth16:
			return 2;

		case ColorR8:
		case ColorA8:
			return 1;

		case Float16_4:
			return 8;

		default:
			return 0;
	}
}
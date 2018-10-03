#include "../Utils/Utils.h"
#include "TextureDX11.h"


DXGI_FORMAT TextureDX11impl::PixFormats[ITexture::MaxFormats];



TextureDX11impl::~TextureDX11impl()
{
	release();
}


void TextureDX11impl::fillDesc()
{
	if (pDxTexture_)
	{
		D3D11_RESOURCE_DIMENSION dim;
		pDxTexture_->GetType(&dim);

		DXGI_FORMAT dx_format = DXGI_FORMAT_UNKNOWN;

		if (D3D11_RESOURCE_DIMENSION_TEXTURE1D == dim)
		{
			ID3D11Texture1D* pTexture1D = reinterpret_cast<ID3D11Texture1D*>(pDxTexture_);
			D3D11_TEXTURE1D_DESC desc;
			pTexture1D->GetDesc(&desc);

			dx_format = desc.Format;
			width_ = desc.Width;
			height_ = 1;
		}
		else
		{
			D3D11_TEXTURE2D_DESC desc;
			pDxTexture_->GetDesc(&desc);

			dx_format = desc.Format;
			width_ = desc.Width;
			height_ = desc.Height;

			multisampleCount_ = desc.SampleDesc.Count;
			multisampleQuality_ = desc.SampleDesc.Quality;
		}

		// Find the format.
		int format = ColorRGBA8;
		for (; format < LastFormat; ++format)
		{
			if (dx_format == PixFormats[format])
				break;
		}

		if (format == LastFormat)
		{
			ShowError(nullptr, __FUNCTION__, " error: unknown pixel format: ", static_cast<int>(dx_format), ".");
			return;
		}

		pixelFormat_ = static_cast<PixelFormat>(format);
	}
}


ID3D11Texture2D* TextureDX11impl::getDxTexture()
{
	return pDxTexture_;
}


void TextureDX11impl::release()
{
	SAFE_RELEASE_DX( pDxTexture_ );
	SAFE_RELEASE_DX( pRndrTrgtView_ );
	SAFE_RELEASE_DX( pDepthView_ );
	SAFE_RELEASE_DX( pShaderView_ );
}


void TextureDX11impl::initNativeFormats()
{
	PixFormats[UnknownFormat] = DXGI_FORMAT_UNKNOWN;
	PixFormats[ColorRGBA8] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PixFormats[ColorRGB8] = DXGI_FORMAT_UNKNOWN;  // RGB8 is not supported in DX11.
	PixFormats[ColorR5G6B5] = DXGI_FORMAT_B5G6R5_UNORM;
	PixFormats[ColorR8] = DXGI_FORMAT_R8_UNORM;
	PixFormats[ColorR16F] = DXGI_FORMAT_R16_FLOAT;
	PixFormats[ColorR32F] = DXGI_FORMAT_R32_FLOAT;
	PixFormats[ColorA8] = DXGI_FORMAT_A8_UNORM;
	PixFormats[ColorBGRA8] = DXGI_FORMAT_B8G8R8A8_UNORM;
	PixFormats[Depth16] = DXGI_FORMAT_D16_UNORM;
	PixFormats[Depth32] = DXGI_FORMAT_D32_FLOAT;
	PixFormats[Float16_4] = DXGI_FORMAT_R16G16B16A16_UNORM;
}

#pragma once

#include "Texture.h"

#include <d3d11.h>


class TextureDX11impl : public ITexture
{
public:
	void fillDesc();

	ID3D11Texture2D* getDxTexture();

	virtual ~TextureDX11impl();

protected:
	friend class RenderDX11;

	virtual void release() final;

private:
	static void initNativeFormats();

	static DXGI_FORMAT PixFormats[ITexture::MaxFormats];

	ID3D11Texture2D* pDxTexture_ = nullptr;

	ID3D11RenderTargetView* pRndrTrgtView_ = nullptr;
	ID3D11DepthStencilView* pDepthView_ = nullptr;
	ID3D11ShaderResourceView* pShaderView_ = nullptr;

//friend std::shared_ptr<TextureDX11impl>;
};

typedef std::shared_ptr<TextureDX11impl> TextureDX11;
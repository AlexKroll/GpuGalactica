#include "../Utils/Utils.h"

#include "FontDX11.h"
#include "TextureDX11.h"

ShaderProgram FontDX11::prFont = nullptr;
//ShaderProgram FontDX11::prFontGdi = nullptr;


/*
FontDX11* FontDX11::createFont(Render* pRender, Texture pCommonFontTexture, PCCHAR fontName, int width, int height, int weight)
{
	if (pRender == nullptr)
		return nullptr;

	// Shader for drawing font symbols from GDI texture to alpha-channel texture.
	if (prFontGdi == nullptr)
	{
		TCHAR* vsText = "float4 Size		: register(c0);	\
						 float4 ScreenPos	: register(c1);	\
						 float4 ScreenProj	: register(c2);	\
						 float4 UvMulAdd	: register(c3);	\
						 void Main( in float4 VertexPos: POSITION, in float2 VertexTexCrd: TEXCOORD0,	\
									out float4 OutPos: SV_Position, out float2 OutTexCrd: TEXCOORD0 )	\
						{ \
							OutPos = VertexPos * Size;		\
							OutPos.xy += ScreenPos.xy;		\
							OutPos *= ScreenProj;			\
							OutTexCrd = VertexTexCrd.xy * UvMulAdd.xy + UvMulAdd.zw;  \
						}";

		TCHAR* psText = "Texture2D Tex: register(t0);  \
						 sampler Samp: register(s0);   \
						 void Main( in float4 Pos: SV_POSITION, in float2 TexCoord: TEXCOORD0, out float4 ColorOut : SV_Target ) \
						 { \
							ColorOut.rgba = Tex.Sample(Samp,TexCoord).rrrr; \
						 }";

		prFontGdi = pRender->createShaderProgram(vsText, psText);
		if (prFontGdi_ == nullptr)
			return nullptr;
	}

	TextureDX11* pCommonTexture = dynamic_cast<TextureDX11*>(pCommonFontTexture);
	if (pCommonTexture == nullptr)
		return nullptr;

	ID3D11Texture2D* pDxTexture = pCommonTexture->getDxTexture();
	if (pDxTexture == nullptr)
		return nullptr;

	IDXGISurface1* pDxgiSurface = nullptr;
	HRESULT hr = pDxTexture->QueryInterface(&pDxgiSurface);
	if (hr != S_OK)
	{
		ShowError(nullptr, __FUNCTION__, " error: ID3D11Texture2D::QueryInterface(IDXGISurface1) failed.");
		return nullptr;
	}

	HDC hdc = nullptr;
	hr = pDxgiSurface->GetDC(false, &hdc);
	if (hr != S_OK)
	{
		ShowError(nullptr, __FUNCTION__, " error: IDXGISurface1::GetDC failed.");
		//int refs = pDxgiSurface->Release();
		return nullptr;
	}

	HFONT hFont = CreateFontA(height, width, 0, 0, weight, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0,
							  NONANTIALIASED_QUALITY, 0, fontName);
	if (hFont == nullptr)
	{
		ShowError(nullptr, __FUNCTION__, " error: GDI::CreateFontA failed.");
		//int refs = pDxgiSurface->Release();
		return nullptr;
	}

	FontDX11* pFont = new FontDX11;
	if (pFont == nullptr)
		return nullptr;

	pFont->pRender_ = pRender;

	// Calculate the max width and height of the one symbol...
	int max_w = 0, max_h = 0;
	char symbol = 0;

	SelectObject(hdc, hFont);

	// ... iterate the symbol from 0 to 255 and get the width and height of each char.
	for (int y = 0; y < 16; y++)
	{	for (int x = 0; x < 16; x++)
		{
			int w = 0, h = 0;

			int index = static_cast<int>(symbol);
			pFont->symbols_[index].w = 0;

			if (symbol != 0 && symbol != 9 && symbol != 10 && symbol != 13 )
			{
				RECT Rect = {0, 0, 0, 0};

				DrawTextA(hdc, &symbol, 1, &Rect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_CALCRECT);

				w = Rect.right - Rect.left + 1;
				h = Rect.bottom - Rect.top + 1;

				pFont->symbols_[index].w = w;
			}
			if (w > max_w) max_w = w;
			if (h > max_h) max_h = h;

			symbol++;
		}
	}

	int frame_w = max_w + 2;	// +2 - we need the interval between symbols if the font will be scaled (texture filtering).
	int frame_h = max_h + 2;

	// we need bigger common texture.
	int cmn_w, cmn_h;
	pCommonTexture->getDimentions(cmn_w, cmn_h);
	if (((frame_w*16) > cmn_w) || ((frame_h*16) > cmn_h))
		return nullptr;

	pFont->symbHeight_ = max_h;

	// Create the bitmap where there is a table of 16x16 symbols.
	// Draw the symbols on the GDI surface.
	SetTextColor(hdc, RGB(255,255,255));
	SetBkColor(hdc, RGB(0,0,0));

	symbol = 0;

	for (int y = 0; y < 16; y++)
	{	for (int x = 0; x < 16; x++)
		{
			RECT Rect = {0, 0, 0, 0};

			DrawTextA(hdc, &symbol, 1, &Rect, DT_LEFT | DT_TOP | DT_NOCLIP);
		}

		symbol++;
	}

	DeleteObject(hFont);
	pDxgiSurface->ReleaseDC(0);
	SAFE_RELEASE_DX(pDxgiSurface);

	//  Create alpha-channel texture in GPU memory and copy the GDI surface there (only red-channel to alpha-channel).
	//pFont->pTexture_ = pRender->createTexture(frame_w*16, frame_h*16, ITexture::ColorA8, ITexture::RenderTarget);
	//if (pFont->pTexture_ == nullptr)
	//	return nullptr;

	//pRender->keepRenderTargetContext();
	//pRender->setRenderTargetContext(pFont->pTexture_, nullptr);

	//pRender->setShaderProgram(prFontGdi_);

	//RECT rect = {0,0, frame_w*16-1, frame_h*16-1};
	//pRender->drawSprite(0,0, pCommonTexture, &rect, IRender::kCustomShaderProgram);

	//pRender->restoreRenderTargetContext();

	//return nullptr;
	//pTexture;

	//pBMFont->CellW = (WORD)FrameW;    pBMFont->CellH = (WORD)FrameH;
}
*/

FontDX11* FontDX11::createFont(Render pRender, cstring filePath)
{
	if (nullptr == pRender)
		return nullptr;

	// Shader for drawing font symbols from alpha texture.
	if (nullptr == prFont)
	{
		TCHAR* psText = "Texture2D Tex: register(t0);  \
						 sampler Samp: register(s0);   \
						 void Main( in float4 Pos: SV_POSITION, in float2 TexCoord: TEXCOORD0, in float4 Color: COLOR0, out float4 ColorOut : SV_Target ) \
						 { \
							ColorOut.rgb = Color.rgb;  \
							ColorOut.a = Tex.Sample(Samp,TexCoord).a * Color.a; \
						 }";

		ShaderProgram program = pRender->getSpriteShaderProgram();
		prFont = pRender->createShaderProgram(program->getVertexShader(), psText);
		if (prFont == nullptr)
			return nullptr;
	}

	FontDX11* pFont = new FontDX11;
	if (pFont == nullptr)
		return nullptr;

	pFont->mSymbolsTexture = pRender->LoadTextureFromFile(filePath);

	// Replace path of the texture with path for txt font describer.
	size_t found = filePath.rfind('/', filePath.length());
	if (found != std::string::npos)
	{
		std::string name = filePath.substr(found+1, filePath.length() - found);
		name = name.replace(name.length()-3, 3, "txt");

		std::string txt_path = filePath;
		txt_path.replace(found+1, name.length(), name);

		pFont->loadDescriber(txt_path);
	}

	pFont->pRender_ = pRender;

	return pFont;
}


void FontDX11::loadDescriber(cstring filePath)
{
	FileAutoClose hf;
	errno_t res = fopen_s(hf.getAddrOf(), filePath.c_str(), "r");
	if (res == 0 )
	{
		char str[512];
		fscanf_s(hf, "%s%d", str, 512, &version_);

		char ch[3];
		fscanf_s(hf, "%c%c", &ch[0], 1, &ch[1], 1);

		fscanf_s(hf, "%s%d", str, 512, &uppercaseHeight_);

		fscanf_s(hf, "%c%c", &ch[0], 1, &ch[1], 1);

		for (int smb = 0; smb < 256; ++smb)
		{
			fscanf_s(hf, "%c%c%c", &ch[0], 1, &ch[1], 1, &ch[2], 1);

			Rect& rect = symbols_[smb].rect;
			fscanf_s(hf, "%d%d%d%d", &rect.x1, &rect.y1, &rect.x2, &rect.y2);

			fscanf_s(hf, "%d", &symbols_[smb].dx);
			fscanf_s(hf, "%d", &symbols_[smb].w);

			fscanf_s(hf, "%c", &ch[0], 1);
		}

		symbHeight_ = symbols_['A'].rect.height();

		if (version_ >= 3)  // scan kerning
		{
		}
	}
}


void FontDX11::drawText(int x, int y, PCCHAR pText, uint32_t color)
{
	if (nullptr == pRender_ || nullptr == mSymbolsTexture || nullptr == prFont)
		return;

	points_.clear();
	rects_.clear();

	pRender_->setShaderProgram(prFont);

	int nChars = 0;
	Point pnt(x, y);
	for (int i = 0; pText[i]; ++i)
	{
		BYTE symbol = static_cast<BYTE>(pText[i]);

		if (symbol == '\n')
		{
			pRender_->drawSprites(nChars, &points_.front(), &rects_.front(), mSymbolsTexture, color, IRender::kAlphaBlendNormal | IRender::kCustomShaderProgram);

			pnt.x = x;
			pnt.y += symbHeight_;

			points_.clear();
			rects_.clear();
			nChars = 0;
			continue;
		}

		points_.push_back(pnt);
		rects_.push_back(symbols_[symbol].rect);

		pnt.x += symbols_[symbol].w;
		nChars++;
	}

	pRender_->drawSprites(nChars, &points_.front(), &rects_.front(), mSymbolsTexture, color, IRender::kAlphaBlendNormal | IRender::kCustomShaderProgram);
}


void FontDX11::drawText(CPoint pos, PCCHAR pText, uint32_t color)
{
	drawText(pos.x, pos.y, pText, color);
}


void FontDX11::getTextSize(PCCHAR pText, int& width, int& height)
{
	width = 0;
	int max_width = 0;
	height = uppercaseHeight_;

	for (int i = 0; pText[i]; ++i)
	{
		BYTE symbol = static_cast<BYTE>(pText[i]);

		if (symbol == '\n')
		{
			width = 0;
			height += uppercaseHeight_;
			continue;
		}

		width += symbols_[symbol].w;
		if (width >= max_width)
			max_width = width;
	}

	width = max_width;
}


int FontDX11::getTextWidth(PCCHAR pText)
{
	int w, h;
	getTextSize(pText, w, h);
	return w;
}


int FontDX11::getTextHeight(PCCHAR pText)
{
	int w, h;
	getTextSize(pText, w, h);
	return h;
}


void FontDX11::release()
{
	if (pRender_)
		pRender_->releaseTexture(mSymbolsTexture);
}


void FontDX11::finalize()
{
	if (prFont)
		prFont->release();
	//if (prFontGdi_)
	//	prFontGdi_->release();
}
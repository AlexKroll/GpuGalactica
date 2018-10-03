#pragma once

#include <string>
#include <dxgi.h>

#include "Font.h"
#include "Render.h"
#include "ShaderProgram.h"


// The font is a table of 16x16 symbols.
// To create the font we draw 256 symbols by the GDI into texture (bitmap).

class FontDX11 : public IMyFont
{
public:
	virtual void drawText(int x, int y, PCCHAR pText, uint32_t color) final;

	virtual void drawText(CPoint pos, PCCHAR pText, uint32_t color) final;

	virtual void getTextSize(PCCHAR pText, int& width, int& height) final;

	virtual int getTextWidth(PCCHAR pText) final;

	virtual int getTextHeight(PCCHAR pText) final;

	virtual void release() final;

private:
	friend class RenderDX11;

	FontDX11() {}
	//virtual ~FontDX11() {}

	//static FontDX11* createFont(Render pRender, Texture pCommonFontTexture, PCCHAR fontName, int width, int height, int weight);
	static FontDX11* createFont(Render pRender, cstring filePath);

	void loadDescriber(cstring filePath);

	static void finalize();

	Render pRender_ = nullptr;

	Texture mSymbolsTexture = nullptr;

	static ShaderProgram prFont;
	//static ShaderProgram prFontGdi;

	struct OneSymbol
	{
		Rect rect;	// frame of one symbol
		int dx;		// displacment of the symbol
		int w;		// width of one symbol
	};
	OneSymbol symbols_[256];

	std::vector<Point> points_;
	std::vector<Rect> rects_;

	int uppercaseHeight_ = 0;   // The height of the uppercase letter.
};

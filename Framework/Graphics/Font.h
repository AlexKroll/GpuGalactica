#pragma once

#include "../Utils/PointRect.h"
#include "Texture.h"


class IMyFont abstract
{
public:
	virtual void drawText(int x, int y, PCCHAR pText, uint32_t color) = 0;

	virtual void drawText(CPoint pos, PCCHAR pText, uint32_t color) = 0;

	virtual void getTextSize(PCCHAR pText, int& width, int& height) = 0;

	virtual int getTextWidth(PCCHAR pText) = 0;

	virtual int getTextHeight(PCCHAR pText) = 0;

	virtual void release() = 0;

protected:
	IMyFont() {}
	virtual ~IMyFont() {}

	int version_ = 0;

	int symbHeight_ = 0;  // The height in pixels of the one symbol.
};

//typedef IMyFont* Font;
typedef std::shared_ptr<IMyFont> Font;
typedef const Font CFont;  // We don`t use the MFC.

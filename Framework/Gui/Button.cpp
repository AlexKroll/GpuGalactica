#include "Gui.h"


void Button::nativeMouseDown(WidgGui* pGui, byte mouseButtonType)
{
	if (pGui->getFocusWidget() == this && Input::mouseButtonLeft == mouseButtonType)
		shift_ = 1;
}


void Button::draw(Render pRender, WidgGui* pGui)
{
	pos_ += Point(shift_, shift_);

	Widget::draw(pRender, pGui);

	// Caption.
	PCCHAR pText = text_.c_str();
	CFont pFont = pRender->getDefaultFont();

	if (pText && pFont)
	{
		int tw, th;
		pFont->getTextSize(pText, tw, th);

		int tx = ( texRect_.width() - tw ) / 2 + 1;
		int ty = ( texRect_.height() - th ) / 2 - 2;

		Color color = textColor_;
		color = color * color_;
		uint32_t int_color;
		color.store(int_color);

		pFont->drawText(pos_ + Point(tx,ty), pText, int_color);
	}

	pos_ -= Point(shift_, shift_);
	shift_ = 0;
}
//#include "Panel.h"
#include "Gui.h"



void Panel::draw(Render pRender, WidgGui* pGui)
{
	if (nullptr == pRender)
		return;

	Widget::draw(pRender, pGui);

	// Title.
	PCCHAR pText = text_.c_str();
	CFont pFont = pRender->getDefaultFont();

	if (pText && pFont)
	{
		pFont->drawText(pos_ + Point(10,2), pText, textColor_);
	}
}
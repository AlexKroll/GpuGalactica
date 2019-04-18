#include "TextLabel.h"



void TextLabel::draw(Render pRender, WidgGui*)
{
	if (nullptr == pRender || nullptr == pFont_)
		return;

	PCCHAR pText = text_.c_str();
	if (pText)
	{	pFont_->drawText(pos_ + Point(0,0), pText, textColor_);
	}
}
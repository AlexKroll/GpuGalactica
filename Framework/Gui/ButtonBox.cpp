#include "Gui.h"



void ButtonBox::initBox(Widget::Type type, uint32_t checks, CFont pFont, const std::vector<std::string> itemsText)
{
	type_ = type;

	checks_ = checks;

	enables_ = 0xFFFFFFFF;

	itemsTexts_ = itemsText;

	// Calculate the size of the box.
	int h = 0, w = textDx_;
	int max_w = 0;

	if (pFont && text_.c_str())
	{	h += titleY_;

		int w2 = pFont->getTextWidth(text_.c_str());
		if (w2 > max_w)
			max_w = w2;
	}

	h += static_cast<int>(itemsTexts_.size()) * step_;

	pFont_ = pFont;
	if (pFont_)
	{
		for (size_t i = 0; i < itemsTexts_.size(); ++i)
		{
			int w2 = pFont->getTextWidth(itemsTexts_[i].c_str());
			if (w2 > max_w)
				max_w = w2;
		}
	}

	w = max_w;

	texRect_ = Rect(0, 0, w-1, h-1);
}


void ButtonBox::disableItem(uint32_t check)
{
	enables_ &= ~check;
}


void ButtonBox::nativeMouseClick(byte mouseButtonType)
{
	uint32_t flag = 1 << focusItem_;

	if ((enables_ & flag) == 0)
		return;

	if (RadioButtons == type_)
	{	if (focusItem_ >= 0 && Input::mouseButtonLeft == mouseButtonType)
			checks_ = flag;
	}
	else
	{	if (checks_ & flag)
			checks_ &= ~flag;
		else
			checks_ |= flag;
	}
}


void ButtonBox::process(WidgGui* pGui, float elapsedTime)
{
	Widget::process(pGui, elapsedTime);

	// Box item under mouse.
	focusItem_ = -1;

	Point local_mouse_pos = pGui->getLocalMousePoint();

	if (pGui->getFocusWidget() == this)
	{
		if (local_mouse_pos.y > titleY_)
		{	focusItem_ = (local_mouse_pos.y - titleY_) / step_;
			if (focusItem_ >= itemsTexts_.size())
				focusItem_ = static_cast<int>(itemsTexts_.size()) - 1;
		}
	}
}


void ButtonBox::draw(Render pRender, WidgGui* pGui)
{
	if (nullptr == pRender || nullptr == pFont_)
		return;

	int y = 0;

	// Title.
	PCCHAR pText = text_.c_str();
	if (pText)
	{	pFont_->drawText(pos_ + Point(0,0), pText, textColor_);
		y += titleY_;
	}

	// The texts of the buttons and checked images.
	uint32_t check = 1;
	for (size_t i = 0; i < itemsTexts_.size(); ++i, y += step_, check <<= 1)
	{
		Rect* pRect;
		if (RadioButtons == type_)
			pRect = (check & checks_) ? &pGui->checkedRadioRect_ : &pGui->uncheckedRadioRect_;
		else
			pRect = (check & checks_) ? &pGui->checkedCheckRect_ : &pGui->uncheckedCheckRect_;

		uint32_t color = (i == focusItem_) ? pGui->highColor_ : pGui->fadeColor_;
		if ((enables_ & check) == 0)
			color = (pGui->fadeColor_ & 0x00FFFFFF) | 0x77000000;

		pRender->drawSprite(pos_+ Point(0,y), pGui->buttonBoxTexture_, *pRect, color, IRender::kAlphaBlendNormal);
			
		pFont_->drawText(pos_ + Point(textDx_,y-2), itemsTexts_[i].c_str(), color);
	}
}
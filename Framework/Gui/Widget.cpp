#include "Gui.h"



void Widget::addChild(Widget* pwidget)
{
	std::vector<Widget*>::iterator iter = std::find(childs_.begin(), childs_.end(), pwidget);

	if (iter == childs_.end())
	{
		pwidget->pParent_ = this;

		childs_.push_back(pwidget);
	}
}


void Widget::removeChild(Widget* pwidget)
{
	std::vector<Widget*>::iterator iter = std::find(childs_.begin(), childs_.end(), pwidget);

	if (iter != childs_.end())
	{
		pwidget->pParent_ = nullptr;

		childs_.erase(iter);
	}
}


void Widget::init(Type type, CPoint pos, Texture texture, CRect texRect, cstring text, uint32_t textColor)
{
	type_ = type;

	order_ = 1;

	pos_ = pos;

	texture_ = texture;
	texRect_ = texRect;

	text_ = text;
	textColor_ = textColor;
}


void Widget::initHighlighting(uint32_t highColor, float highTime, uint32_t fadeColor, float fadeTime)
{
	highColor_ = highColor,  highTime_ = highTime;
	fadeColor_ = fadeColor, fadeTime_ = fadeTime;
}


void Widget::processHighlighting(Gui* pGui, float elapsedTime)
{
	Color Color1, Color2;
	float time;

	if (pGui->getFocusWidget() == this)
	{	if (color_ == highColor_)
			return;

		Color1 = fadeColor_;
		Color2 = highColor_;
		time = highTime_;
	}
	else
	{	if (color_ == fadeColor_)
			return;

		Color1 = highColor_;
		Color2 = fadeColor_;

		time = fadeTime_;
	}

	currLightTime_ += elapsedTime;
	float factor = currLightTime_ / time;
	if( factor >= 0.95f )
	{	factor = 1.0f;
		currLightTime_ = 0.0f;
	}

	Color color = Color1 + (Color2 - Color1) * factor;

	color.store(color_);
}


void Widget::process(Gui*, float)
{
}


void Widget::draw(Render pRender, Gui*)
{
	if (nullptr == pRender)
		return;

	pRender->drawSprite(pos_, texture_, texRect_, color_, IRender::kAlphaBlendNormal);
}
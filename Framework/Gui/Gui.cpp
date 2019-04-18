#include "Gui.h"



WidgGui* WidgGui::getInstance(Render render)
{
	static WidgGui* pGui = nullptr;
	if (nullptr == pGui)
		pGui = new WidgGui;

	if (pGui)
		pGui->pRender_ = render;

	return pGui;
}


void WidgGui::setColors(uint32_t panelTextColor, uint32_t buttonTextColor)
{
	panelTextColor_ = panelTextColor;
	buttonTextColor_ = buttonTextColor;
}


void WidgGui::setWidgetHighlighting(uint32_t highColor, float highTime, uint32_t fadeColor, float fadeTime)
{
	highColor_ = highColor,  highTime_ = highTime;
	fadeColor_ = fadeColor, fadeTime_ = fadeTime;
}


void WidgGui::setButtonBoxData(Texture texture, CRect uncheckedRadioRect, CRect checkedRadioRect,
							CRect uncheckedCheckRect, CRect checkedCheckRect)
{
	buttonBoxTexture_ = texture;
	uncheckedRadioRect_ = uncheckedRadioRect;
	checkedRadioRect_ = checkedRadioRect;
	uncheckedCheckRect_ = uncheckedCheckRect;
	checkedCheckRect_ = checkedCheckRect;
}


Panel* WidgGui::createPanel(CPoint pos, Texture texture, CRect texRect, cstring title)
{
	Panel* pPanel = new Panel;
	if (nullptr == pPanel)
		return nullptr;

	pPanel->init(Widget::Panel, pos, texture, texRect, title, panelTextColor_);

	//pPanel->initHighlighting(highColor_, highTime_, fadeColor_, fadeTime_);

	pCurrMaster_ = pPanel;

	widgets_.push_back(pPanel);
	return pPanel;
}


Button* WidgGui::createButton(CPoint pos, Texture texture, CRect texRect, cstring caption)
{
	Button* pButton = new Button;
	if (nullptr == pButton)
		return nullptr;

	Point pos2 = (pCurrMaster_) ? pCurrMaster_->pos_ + pos : pos;

	pButton->init(Widget::Button, pos2, texture, texRect, caption, buttonTextColor_);

	pButton->initHighlighting(highColor_, highTime_, fadeColor_, fadeTime_);

	widgets_.push_back(pButton);
	return pButton;
}


ButtonBox* WidgGui::createButtonBox(CPoint pos, Texture texture, uint32_t checks, const std::vector<std::string> itemsText,cstring title, bool bRadioButtons)
{
	ButtonBox* pBox = new ButtonBox;
	if (nullptr == pBox || nullptr == pRender_)
		return nullptr;

	Point pos2 = (pCurrMaster_) ? pCurrMaster_->pos_ + pos : pos;

	pBox->init(Widget::Empty, pos2, texture, Rect(-1,-1,-1,-1), title, panelTextColor_);

	Widget::Type type = (bRadioButtons) ? Widget::RadioButtons : Widget::CheckButtons;

	pBox->initBox(type, checks, pRender_->getDefaultFont(), itemsText);

	widgets_.push_back(pBox);
	return pBox;
}


/*ButtonBox* WidgGui::createCheckButtonBox(CPoint pos, Texture texture, CRect texRect, std::vector<cstring> itemsText)
{
}*/


TextLabel* WidgGui::createTextLabel(CPoint pos, cstring text, uint32_t color)
{
	TextLabel* pLabel = new TextLabel;
	if (nullptr == pLabel || nullptr == pRender_)
		return nullptr;

	Point pos2 = (pCurrMaster_) ? pCurrMaster_->pos_ + pos : pos;

	pLabel->init(Widget::Empty, pos2, 0, Rect(-1,-1,-1,-1), text, color);

	pLabel->pFont_ = pRender_->getDefaultFont();

	widgets_.push_back(pLabel);
	return pLabel;
}


int32_t WidgGui::getButtonBoxCheckIndex()
{
	if (pCurrBox_)
	{
		return pCurrBox_->focusItem_;
	}
	return -1;
}


uint32_t WidgGui::getkButtonBoxCheckFlags()
{
	if (pCurrBox_)
	{
		return pCurrBox_->checks_;
	}
	return 0;
}


void WidgGui::activateWidget(Widget* pwidget)
{
	if (pwidget)
	{	pwidget->order_ = widgetOrderTop_++;
		pwidget->states_ |= WidgGui::Active | WidgGui::Visible;

		for (Widget* pchild : pwidget->childs_)
			activateWidget(pchild);
	}
}


void WidgGui::hideWidget(Widget* pwidget)
{
	if (pwidget)
	{	pwidget->order_ = 1;
		pwidget->states_ &= ~(WidgGui::Active | WidgGui::Visible);

		for (Widget* pchild : pwidget->childs_)
			hideWidget(pchild);

		bNeedResort_ = true;
	}
}


void WidgGui::destroyWidget(Widget* pwidget)
{
	if (pwidget)
	{
		if (pwidget->pParent_)
			pwidget->pParent_->removeChild(pwidget);

		pwidget->order_ = 0;
		
		for (Widget* pchild : pwidget->childs_)
		{
			destroyWidget(pchild);
		}
	}
}


void WidgGui::process(std::shared_ptr<Input> pInput, float elapsedTime)
{
	pCurrMaster_ = nullptr;
	pCurrBox_ = nullptr;

	if (nullptr == pInput)
		return;

	// Find focus widget by mouse cursor.
	pFocusWidget_ = nullptr;
	localMousePoint_ = Point(-1, -1);
	int order = -1;
	Point mouse_pos = pInput->getMousePoint();
	for (Widget* pwidget : widgets_)
	{
		if (0 == pwidget->order_)
			continue;

		Rect rect = Rect(pwidget->pos_, pwidget->pos_ + pwidget->texRect_.getWH() - Point(1,1));
		if (rect.isPointInside(mouse_pos))
		{
			if (pwidget->order_ >= order)
			{	order = pwidget->order_;
				pFocusWidget_ = pwidget;
				localMousePoint_ = mouse_pos - pwidget->pos_;
			}
		}
	}

	if (pFocusWidget_)
	{
		if (pInput->isMouseButtonDown(Input::mouseButtonLeft))
		{
			pFocusWidget_->nativeMouseDown(this, Input::mouseButtonLeft);

			if (Widget::RadioButtons == pFocusWidget_->type_ || Widget::CheckButtons == pFocusWidget_->type_)
				pCurrBox_ = dynamic_cast<ButtonBox*>(pFocusWidget_);
		}

		if (pInput->isMouseButtonClick(Input::mouseButtonLeft))
		{
			pFocusWidget_->nativeMouseClick(Input::mouseButtonLeft);

			if (Widget::RadioButtons == pFocusWidget_->type_ || Widget::CheckButtons == pFocusWidget_->type_)
				pCurrBox_ = dynamic_cast<ButtonBox*>(pFocusWidget_);

			if (pFocusWidget_->pLeftClickFunc)
				pFocusWidget_->pLeftClickFunc->invoke();
		}
	}

	pCurrBox_ = nullptr;

	for (Widget* pwidget : widgets_)
	{
		if (0 == pwidget->order_)
			continue;

		if (pwidget->states_ & WidgGui::Active && pwidget->highColor_ != pwidget->fadeColor_)
			pwidget->processHighlighting(this, elapsedTime);

		if (pwidget->states_ & WidgGui::Active)
			pwidget->process(this, elapsedTime);
	}

	if (bNeedDelete_)
	{	bNeedDelete_ = false;

		widgets_.erase(std::remove_if(widgets_.begin(), widgets_.end(),
			[] (Widget* pwidget) -> bool
			{
				if (pwidget->order_ == 0)
				{	delete pwidget;
					return true;
				}
				else return false;
			}));
	}

	if (bNeedResort_)
	{	bNeedResort_ = false;

		std::sort(widgets_.begin(), widgets_.end(),
			[] (Widget* pwidget1, Widget* pwidget2) -> bool
			{
				return pwidget1->order_ > pwidget2->order_;
			});
	}
}


void WidgGui::render()
{
	if (nullptr == pRender_)
		return;

	for (Widget* pwidget : widgets_)
	{
		if (pwidget->states_ & WidgGui::Visible)
			pwidget->draw(pRender_, this);
	}

	pCurrMaster_ = nullptr;
}
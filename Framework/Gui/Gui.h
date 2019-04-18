#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "Panel.h"
#include "Button.h"
#include "ButtonBox.h"
#include "TextLabel.h"
#include "../Graphics/Render.h"
#include "../Core/Input.h"



class WidgGui
{
public:
	static WidgGui* getInstance(Render render);

	void setColors(uint32_t panelTextColor, uint32_t buttonTextColor);

	void setWidgetHighlighting(uint32_t highColor, float highTime, uint32_t fadeColor, float fadeTime);

	void setButtonBoxData(Texture texture, CRect uncheckedRadioRect, CRect checkedRadioRect,
							CRect uncheckedCheckRect, CRect checkedCheckRect);

	Panel* createPanel(CPoint pos, Texture texture, CRect texRect, cstring title);


	template <class T>
	Button* createButton(CPoint pos, Texture texture, CRect texRect, cstring caption, T* pObject, int (T::*pMouseLeftClick)())
	{
		Button* pButton = createButton(pos, texture, texRect, caption);
		if (pButton && nullptr == pButton->pLeftClickFunc)
			pButton->pLeftClickFunc = new Delegate<T>(pObject,pMouseLeftClick);
		return pButton;
	}


	template <class T>
	ButtonBox* createRadioButtonBox(CPoint pos, Texture texture, uint32_t checks, const std::vector<std::string> itemsText, cstring title,
									T* pObject, int (T::*pMouseLeftClick)())
	{
		ButtonBox* pBox = createButtonBox(pos, texture, checks, itemsText, title, true);
		if (pBox && nullptr == pBox->pLeftClickFunc)
			pBox->pLeftClickFunc = new Delegate<T>(pObject,pMouseLeftClick);
		return pBox;
	}


	template <class T>
	ButtonBox* createCheckButtonBox(CPoint pos, Texture texture, uint32_t checks, const std::vector<std::string> itemsText, cstring title,
									T* pObject, int (T::*pMouseLeftClick)())
	{
		ButtonBox* pBox = createButtonBox(pos, texture, checks, itemsText, title, false);
		if (pBox && nullptr == pBox->pLeftClickFunc)
			pBox->pLeftClickFunc = new Delegate<T>(pObject,pMouseLeftClick);
		return pBox;
	}


	TextLabel* createTextLabel(CPoint pos, cstring text, uint32_t color);


	int32_t getButtonBoxCheckIndex();

	uint32_t getkButtonBoxCheckFlags();

	void activateWidget(Widget* pwidget);
	void hideWidget(Widget* pwidget);

	void destroyWidget(Widget* pwidget);

	Widget* getFocusWidget() const {  return pFocusWidget_;  }

	Point getLocalMousePoint() const {  return localMousePoint_;  }

	void process(std::shared_ptr<Input> pInput, float elapsedTime);

	void render();

	static const uint32_t Active = 0x00000001;
	static const uint32_t Visible = 0x00000002;

	uint32_t panelTextColor_ = 0xFFFFFFFF;
	uint32_t buttonTextColor_ = 0xFFFFFFFF;

	uint32_t highColor_ = 0xFFFFFFFF;
	float highTime_ = 0.0f;
	uint32_t fadeColor_ = 0xFFFFFFFF;
	float fadeTime_ = 0.0f;

	Rect uncheckedRadioRect_;
	Rect checkedRadioRect_;
	Rect uncheckedCheckRect_;
	Rect checkedCheckRect_;
	Texture buttonBoxTexture_ = nullptr;

private:
	WidgGui() {}

	Button* createButton(CPoint pos, Texture texture, CRect texRect, cstring caption);

	ButtonBox* createButtonBox(CPoint pos, Texture texture, uint32_t checks, const std::vector<std::string> itemsText, cstring title, bool bRadioButtons);

	Render pRender_ = nullptr;

	std::vector<Widget*> widgets_;

	Widget* pCurrMaster_ = nullptr;

	Widget* pFocusWidget_ = nullptr;

	Point localMousePoint_ = Point(-1, -1);

	int widgetOrderTop_ = 1;

	bool bNeedDelete_ = false;
	bool bNeedResort_ = false;

	ButtonBox* pCurrBox_ = nullptr;
};

typedef std::shared_ptr<WidgGui> Gui;
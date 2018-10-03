#pragma once

#include "Widget.h"


class ButtonBox : public Widget
{
protected:
	virtual void nativeMouseClick(byte mouseButtonType) override;

	virtual void process(Gui* pGui, float elapsedTime) override;

	virtual void draw(Render pRender, Gui* pGui) override;

private:
	std::vector<std::string> itemsTexts_;

	uint32_t checks_;

	int focusItem_ = -1;

	int step_ = 15;
	int titleY_ = 18;
	int textDx_ = 14;

	void initBox(Widget::Type type, uint32_t checks, CFont pFont, const std::vector<std::string> itemsText);

friend class Gui;
};


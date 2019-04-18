#pragma once

#include "Widget.h"


class ButtonBox : public Widget
{
public:
	void disableItem(uint32_t check);

protected:
	virtual void nativeMouseClick(byte mouseButtonType) override;

	virtual void process(WidgGui* pGui, float elapsedTime) override;

	virtual void draw(Render pRender, WidgGui* pGui) override;

private:
	std::vector<std::string> itemsTexts_;

	uint32_t checks_ = 0;	// every item is a bit: true - item is checked.
	uint32_t enables_ = 0;	// if bit is false then item is unaccessed.

	int focusItem_ = -1;

	int step_ = 15;
	int titleY_ = 18;
	int textDx_ = 14;

	void initBox(Widget::Type type, uint32_t checks, CFont pFont, const std::vector<std::string> itemsText);

friend class WidgGui;
};


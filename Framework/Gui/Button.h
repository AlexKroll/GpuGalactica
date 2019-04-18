#pragma once

#include "Widget.h"


class Button : public Widget
{
protected:
	virtual void nativeMouseDown(WidgGui* pGui, byte mouseButtonType) override;

	virtual void draw(Render pRender, WidgGui* pGui) override;

private:
	int8_t shift_ = 0;		// Shift the button when mouse pressed.

friend class WidgGui;
};


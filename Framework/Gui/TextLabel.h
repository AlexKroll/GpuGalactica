#pragma once

#include "Widget.h"


class TextLabel : public Widget
{
protected:
	virtual void draw(Render pRender, Gui* pGui) override;

private:

friend class Gui;
};

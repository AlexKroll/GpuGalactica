#pragma once

#include "Widget.h"


class Panel : public Widget
{
protected:
	virtual void draw(Render pRender, Gui* pGui) override;

friend class Gui;
};

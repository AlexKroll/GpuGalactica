#pragma once

#include "Widget.h"


class Panel : public Widget
{
protected:
	virtual void draw(Render pRender, WidgGui* pGui) override;

friend class WidgGui;
};

#pragma once

#include "Widget.h"


class TextLabel : public Widget
{
protected:
	virtual void draw(Render pRender, WidgGui* pGui) override;

private:

friend class WidgGui;
};

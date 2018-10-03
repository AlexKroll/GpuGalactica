#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>

#include "../Graphics/Render.h"
#include "../Utils/PointRect.h"
#include "../Utils/Delegate.h"

class Gui;


class Widget
{
public:
	void addChild(Widget* pwidget);
	void removeChild(Widget* pwidget);

	void initHighlighting(uint32_t highColor, float highTime, uint32_t fadeColor, float fadeTime);

	uint32_t color_ = 0xFFFFFFFF;

protected:
	Widget() {}
	~Widget() {}

	virtual void nativeMouseDown(Gui*, byte) {  }

	virtual void nativeMouseClick(byte) {  }

	virtual void process(Gui* pGui, float elapsedTime);

	virtual void draw(Render pRender, Gui* pGui);

	enum Type : BYTE
	{
		Empty,
		Panel,
		Button,
		RadioButtons,
		CheckButtons
	};

	Type type_ = Empty;

	int order_ = 0;

	Point pos_ = Point(0,0);

	Texture texture_ = nullptr;
	Rect texRect_ = Rect(0,0,0,0);

	uint32_t highColor_ = 0xFFFFFFFF;		//  widgets are lighting
	uint32_t fadeColor_ = 0xFFFFFFFF;		//  when mouse is over them.
	float highTime_ = 0.0f;
	float fadeTime_ = 0.0f;
	float currLightTime_ = 0.0f;

	uint32_t states_ = 0;

	Widget* pParent_ = nullptr;
	std::vector<Widget*> childs_;

	std::string text_;
	uint32_t textColor_ = 0xFFFFFFFF;

	Font pFont_ = nullptr;

	IDelegate* pLeftClickFunc = nullptr;

private:
	void init(Type type, CPoint pos, Texture texture, CRect texRect, cstring text, uint32_t textColor);

	void processHighlighting(Gui* pGui, float elapsedTime);

friend class Gui;
};

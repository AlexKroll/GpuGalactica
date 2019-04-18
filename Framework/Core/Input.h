#pragma once

#include <windows.h>

#include "../Utils/PointRect.h"


class Input
{
public:
	static Input* getInstance();

	int init(HWND hWindow);

	Point getMousePoint() const;

	bool isMouseButtonDown(byte button) const;

	bool isMouseButtonClick(byte button) const;

	int getMouseMediumWheelRolling();

	void setMouseMediumWheelScrolling(signed short scrolling);

	void process();

	void clear();

	static const byte mouseButtonLeft = 0;
	static const byte mouseButtonRight = 1;
	static const byte mouseButtonMedium = 2;

private:
	Input() {}

	HWND hWindow_ = nullptr;

	Point mousePoint_;

	byte mouseButtons_[3] = {0};

	const byte mouseButtDown = 1;
	const byte mouseButtClick = 2;

	signed short mouseMediumWheel_ = 0;

	int mouseDx_ = 0;
	int mouseDy_ = 0;

	byte keybKeys[256] = {0};

	void processMouse();

	void processKeyboard();
};
#include "Input.h"



Input* Input::getInstance()
{
	static Input* pInput = nullptr;
	if (nullptr == pInput)
		pInput = new Input;

	return pInput;
}


int Input::init(HWND hWindow)
{
	hWindow_ = hWindow;

	mousePoint_.x = mousePoint_.y = -1;

	HCURSOR hCursor = LoadCursor(0, IDC_ARROW);
	hCursor = SetCursor(hCursor);
	ShowCursor(true);

	TITLEBARINFO info;
	info.cbSize = sizeof(TITLEBARINFO);
	GetTitleBarInfo(hWindow_, &info);

	RECT window_rect;
	GetWindowRect(hWindow_, &window_rect);

	RECT client_rect;
	GetClientRect(hWindow_, &client_rect);

	mouseDx_ = ((window_rect.right - window_rect.left) - (client_rect.right - client_rect.left)) / 2;
	mouseDy_ = info.rcTitleBar.bottom - window_rect.top;

	return S_OK;
}


Point Input::getMousePoint()  const
{
	return mousePoint_;
}


bool Input::isMouseButtonDown(byte button) const
{
	return (mouseButtons_[button] == mouseButtDown);
}


bool Input::isMouseButtonClick(byte button) const
{
	return (mouseButtons_[button] == mouseButtClick);
}


int Input::getMouseMediumWheelRolling()
{
	return mouseMediumWheel_;
}


void Input::setMouseMediumWheelScrolling(signed short scrolling)
{
	mouseMediumWheel_ = scrolling;
}


void Input::processMouse()
{
	POINT WinApiPoint;
	::GetCursorPos(&WinApiPoint);

	HWND hCurrWnd = ::WindowFromPoint(WinApiPoint);
	if (hCurrWnd != hWindow_)
		return;

	mousePoint_.x = mousePoint_.y = -1;  // reset the point (outside of our window)

	RECT client_rect;
	::GetClientRect(hWindow_, &client_rect);

	RECT window_rect;
	::GetWindowRect(hWindow_, &window_rect);

	int mouse_x = WinApiPoint.x - window_rect.left - mouseDx_;
	if (mouse_x < 0 || mouse_x > (client_rect.right - client_rect.left))
		return;

	int mouse_y = WinApiPoint.y - window_rect.top - mouseDy_;
	if (mouse_y < 0 || mouse_y > (client_rect.bottom - client_rect.top))
		return;

	mousePoint_ = Point(mouse_x, mouse_y);

	const byte mouse_keys[3] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON };

	for (byte i = 0; i < 3; ++i)
	{
		SHORT state = ::GetKeyState(mouse_keys[i]);
		if (state & 0x8000)
		{	mouseButtons_[i] = mouseButtDown;
		}
		else
		if (mouseButtons_[i] == mouseButtDown)
		{	mouseButtons_[i] = mouseButtClick;
		}
	}
}


void Input::processKeyboard()
{
	::ZeroMemory(keybKeys, sizeof(keybKeys));

	POINT Point;
	::GetCursorPos(&Point);

	HWND hCurrWnd = ::WindowFromPoint(Point);
	if (hCurrWnd != hWindow_)
		return;

	::GetKeyboardState(keybKeys);
}


void Input::process()
{
	processMouse();

	processKeyboard();
}


void Input::clear()
{
	for (byte& button : mouseButtons_)
		if (mouseButtClick == button)
			button = 0;

	mouseMediumWheel_ = 0;
}
#pragma once

#include <windows.h>
#include <memory>

#include "../Utils/Timer.h"
#include "../Graphics/Render.h"
#include "../Gui/Gui.h"
#include "../Gui/Widget.h"



class Application
{
public:
	virtual int init(HINSTANCE hInstance);

	virtual void run();

	virtual void processScene(float elapsedTime) = 0;

	virtual void renderScene(float elapsedTime) = 0;

	float getElapsedTime();

	LRESULT MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    Application() = default;
	virtual ~Application() = default;

private:
	Application(const Application&) = delete;
	Application& operator= (const Application&) = delete;

protected:
	//  Members.
	HINSTANCE hInstance_ = nullptr;
	HWND hWindow_ = nullptr;

	int windowX_ = 0;
	int windowY_ = 0;

	int windowW_ = 0;
	int windowH_ = 0;

	int windowIndent_ = 40;  // The indentation of the window from the edges of the screen.

	bool bActive_ = false;

	std::unique_ptr<Timer> pTimer_ = nullptr;

	float elapsedTime_ = 0.0f;

	//const float kMouseWheelPeriod_ = 0.1f;
	//float mouseWheelTime_ = kMouseWheelPeriod_;

	std::shared_ptr<Input> pInput_ = nullptr;

	Render pRender_ = nullptr;

	Gui pGui_ = nullptr;
};



LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
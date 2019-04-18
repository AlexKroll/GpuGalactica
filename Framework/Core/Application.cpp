#include "Application.h"

#include "../Graphics/Render.h"



int Application::init(HINSTANCE hInstance)
{
	hInstance_ = hInstance;

	// Window class.
	WNDCLASSEX window_class;
	window_class.cbSize			= sizeof(WNDCLASSEX);
	window_class.style			= CS_CLASSDC;
	window_class.lpfnWndProc	= WindowProcedure;
	window_class.cbClsExtra		= 0;
	window_class.cbWndExtra		= 0;
	window_class.hInstance		= hInstance_;
	window_class.hIcon			= nullptr;
	window_class.hCursor		= nullptr;
	window_class.hbrBackground	= nullptr;
	window_class.lpszMenuName	= nullptr;
	window_class.lpszClassName	= "GpuComputing";
	window_class.hIconSm		= nullptr;

	::RegisterClassEx( &window_class );

	// Get display resolution.
	windowW_ = GetSystemMetrics(SM_CXSCREEN) - windowIndent_ * 2;
	windowH_ = GetSystemMetrics(SM_CYSCREEN) - windowIndent_ * 2;

	// Window.
	windowX_ = windowIndent_;
	windowY_ = windowIndent_;

	long win_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	hWindow_ = CreateWindow("GpuComputing", "GpuGalaxy", win_style, windowX_, windowY_, windowW_, windowH_,
									GetDesktopWindow(), nullptr, window_class.hInstance, nullptr);
	if (nullptr == hWindow_)
	{
		int res = ::GetLastError();
		return res;
	}

	ShowWindow (hWindow_, SW_SHOWDEFAULT);
	UpdateWindow(hWindow_);

	// Timer.
	pTimer_ = std::unique_ptr<Timer>(Timer::getInstance());
	if (nullptr == pTimer_)
		return E_FAIL;

	// Mouse/keyboard input.
	pInput_ = std::shared_ptr<Input>(Input::getInstance());
	if (nullptr == pInput_)
		return E_FAIL;
	int res = pInput_->init(hWindow_);
	if (res != S_OK)
		return res;

	// DX11 render.
	pRender_ = std::shared_ptr<IRender>(IRender::getRender(IRender::DX11));
	if (nullptr == pRender_)
		return E_NOINTERFACE;

	res = pRender_->init(hWindow_, true);
	if (res != S_OK)
		return res;

	pGui_ = std::shared_ptr<WidgGui>(WidgGui::getInstance(pRender_));
	if (nullptr == pGui_)
		return E_FAIL;

	return S_OK;
}


void Application::run()
{
	MSG msg = {0};

	if (pTimer_)
	{	pTimer_->startMeasure();
	}

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (bActive_)
			{
				// Process the application.
				double dbl_time = pTimer_->getMeasuredTime();
				elapsedTime_ = static_cast<float>(dbl_time);
				pTimer_->startMeasure();

				if (pInput_)
					pInput_->process();

				if (pGui_)
					pGui_->process(pInput_, elapsedTime_);

				processScene(elapsedTime_);

/*static double t = 0;
static int fps = 0;
t += dbl_time;
if( t > 1.0 )
{	fps++;
	if( t > 5.0 )
	{	fps = fps/4;
		fps = 0;
		t = 0.0;
	}
}*/

				if (pRender_)
					pRender_->prepareDrawing();

				renderScene(elapsedTime_);

				if (pGui_)
					pGui_->render();

				if (pRender_)
					pRender_->presentDrawing();

				if (pInput_)
					pInput_->clear();
			}
			else
			{	Sleep(50);
			}
		}
	}
}


float Application::getElapsedTime()
{
	return elapsedTime_;
}


LRESULT Application::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{	bActive_ = false;
			}
			else
			{	bActive_ = true;
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_MOUSEWHEEL:
			if (pInput_)
			{	//mouseWheelTime_ += pTimer_->getMeasuredTime();
				//if (mouseWheelTime_ >= kMouseWheelPeriod_)
				{
					//mouseWheelTime_ -= kMouseWheelPeriod_;

					pInput_->setMouseMediumWheelScrolling(static_cast<signed short>(HIWORD(wParam)));
				}
			}
		break;
	}

	return ::DefWindowProc(hWnd, message, wParam, lParam);
}

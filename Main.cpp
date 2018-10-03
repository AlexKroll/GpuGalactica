#include "GpuGalaxy.h"




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (lpCmdLine) {}
	if (nCmdShow) {}
	if (hPrevInstance) {}

	GpuGalaxy* pApp = GpuGalaxy::getInstance();
	if (pApp == nullptr)
		return E_FAIL;

	int res = pApp->init(hInstance);

	if (res == S_OK)
		pApp->run();

	delete pApp;

	return res;
}




LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GpuGalaxy* pApp = GpuGalaxy::getInstance();

	return pApp->MsgProc(hWnd, message, wParam, lParam);
}

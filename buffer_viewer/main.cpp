#include "WinApp.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	std::shared_ptr<winapp::WinApp> winapp = winapp::WinApp::create(L"Буфер-Гляделка", hInstance, lpCmdLine, nCmdShow);
	if (winapp.get() == nullptr)
		return 1;
	winapp->run();
}
#include "WinApp.h"
//#include <iostream>

std::shared_ptr<winapp::WinApp> winapp::WinApp::_instance = nullptr;

winapp::WinApp::WinApp(LPCWSTR window_name, HINSTANCE hinstance, LPWSTR lp_cmd_line, int32_t ncmd_show) :
	_hinstance(hinstance), _lp_cmd_line(lp_cmd_line), _ncmd_show(ncmd_show)
{
	_main_window = Window::create(TEXT("MyWindowAppClass"), window_name, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, config::main_window::main_window_width, config::main_window::main_window_height,
		nullptr, nullptr, hinstance, nullptr);
	_child_window = ChildWindow::create(TEXT("MyCHildWindowAppClass"), TEXT("История"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, config::child_window::child_window_width, config::child_window::child_window_height,
		_main_window->get_hwnd(), nullptr, hinstance, nullptr);

	_main_window->set_child_window(_child_window->get_hwnd());

	/*FILE* conin = stdin;
	FILE* conout = stdout;
	FILE* conerr = stderr;
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen_s(&conin, "CONIN$", "r", stdin);
	freopen_s(&conout, "CONOUT$", "w", stdout);
	freopen_s(&conerr, "CONOUT$", "w", stderr);
	SetConsoleTitle(L"appconsole");*/
}

std::shared_ptr<winapp::WinApp> winapp::WinApp::create(LPCWSTR window_name, HINSTANCE hinstance, LPWSTR lp_cmd_line, int32_t ncmd_show)
{
    if (_instance.get() == nullptr)
    {
        _instance = std::shared_ptr<WinApp>(new WinApp(window_name, hinstance, lp_cmd_line, ncmd_show));
	}
    return _instance;
}

std::shared_ptr<winapp::WinApp> winapp::WinApp::get_instance()
{
    return _instance;
}

WPARAM winapp::WinApp::run()
{
	HWND hWndMain = _main_window->get_hwnd();
	ShowWindow(hWndMain, _ncmd_show);
	UpdateWindow(hWndMain);

	BOOL ret;
	while ((ret = GetMessage(&_msg, NULL, 0, 0)) != 0)
	{
		if (ret == -1) break;
		else
		{
			TranslateMessage(&_msg);
			DispatchMessage(&_msg);
		}
	}
	return _msg.wParam;
}

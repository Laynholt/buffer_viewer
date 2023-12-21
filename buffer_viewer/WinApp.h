#pragma once

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment (lib, "comctl32")

#include <Windows.h>

#include <memory>
#include <string>
#include <cstdint>

#include "config.h"
#include "Window.h"
#include "ChildWindow.h"

namespace winapp
{
	class WinApp
	{
	public:
		static std::shared_ptr<WinApp> create(LPCWSTR window_name, HINSTANCE hinstance, LPWSTR lp_cmd_line, int32_t ncmd_show);
		static std::shared_ptr<WinApp> get_instance();

		WPARAM run();
		
	private:
		WinApp(LPCWSTR window_name, HINSTANCE hinstance, LPWSTR lp_cmd_line, int32_t ncmd_show);

	private:
		static std::shared_ptr<WinApp> _instance;

	private:
		HINSTANCE _hinstance;
		LPWSTR _lp_cmd_line;
		int32_t _ncmd_show;
		MSG _msg;

		std::shared_ptr<Window> _main_window;
		std::shared_ptr<ChildWindow> _child_window;
	};
}

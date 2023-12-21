#pragma once

#include <Windows.h>
#include <shlobj_core.h>

#include <memory>
#include <string>
#include <cstdint>

#include "config.h"
#include "resource.h"
#include "HistoryBuffer.h"

namespace winapp
{
	class Window
	{
	public:
		static std::shared_ptr<Window> create(LPCWSTR class_name, LPCWSTR window_name, DWORD style,
			int32_t x, int32_t y, int32_t width, int32_t height,
			HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, LPVOID param);
		static std::shared_ptr<Window> get_instance();

		~Window();

		HWND get_hwnd() { return _hwnd; }
		HWND get_hwnd_label() { return _hwnd_label; }
		HWND get_hwnd_edit() { return _hwnd_edit; }

		void set_label_text(LPCWSTR text);
		void set_edit_text(LPCWSTR text);

		void set_child_window(HWND child);

	private:
		Window(LPCWSTR class_name, LPCWSTR window_name, int32_t x, int32_t y, int32_t width, int32_t height,
			HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, DWORD style, LPVOID param);

		void _registration(HINSTANCE hinstance);

		static LRESULT CALLBACK MainWindowEventHander(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	private:
		static std::shared_ptr<Window> _win_instance;

	private:
		WNDCLASSEX _wnd_class;
		HINSTANCE _hinstance;

	private:
		std::wstring _class_name, _window_name;

		HWND _hwnd{ nullptr }, _hwnd_parent{ nullptr };
		HMENU _hmenu{ nullptr };
		int32_t _x{}, _y{}, _width{}, _height{};

		HWND _hwnd_label{ nullptr }, _hwnd_edit{ nullptr };
		HFONT _font_label{ nullptr }, _font_edit{ nullptr };
		HBRUSH _brush_bg{ nullptr }, _brush_label_bg{ nullptr }, _brush_edit_bg{ nullptr };

		HWND _child{ nullptr };
	};
}
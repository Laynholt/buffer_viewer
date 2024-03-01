#pragma once
#include <Windows.h>
#include <commctrl.h>

#include <memory>
#include <string>
#include <cstdint>
#include <vector>

#include "config.h"
#include "resource.h"
#include "HistoryBuffer.h"

namespace winapp
{
	class ChildWindow
	{
	public:
		static std::shared_ptr<ChildWindow> create(LPCWSTR class_name, LPCWSTR window_name, DWORD style,
			int32_t x, int32_t y, int32_t width, int32_t height,
			HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, LPVOID param);
		static std::shared_ptr<ChildWindow> get_instance();

		~ChildWindow();

		HWND get_hwnd() { return _hwnd; }

	private:
		ChildWindow(LPCWSTR class_name, LPCWSTR window_name, int32_t x, int32_t y, int32_t width, int32_t height,
			HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, DWORD style, LPVOID param);

		void _registration(HINSTANCE hinstance);
		static LRESULT CALLBACK ChildWindowEventHander(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		static LRESULT CALLBACK SubChildWindowEventHander(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		static LRESULT CALLBACK MultiEditSubClassProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR idsubclass, DWORD_PTR refdata);
		static LRESULT CALLBACK EditSubClassProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR idsubclass, DWORD_PTR refdata);

		void _set_scroll_pos(int32_t pos_delta);
		void _update_scroll_size();
		int32_t _get_widget_pack_position(int32_t index);
		int32_t _get_widget_pack_size();
		int32_t _get_max_scroll_size();
		std::wstring _get_text_from_edit();

		void _delete_current_element(int32_t index);

		void _event_handler_digit_keys(WPARAM symbol);
		void _event_handler_backspace_key();
		void _event_handler_enter_key();
		void _event_handler_ctrlA_key();
		void _event_handler_delete_key();

	private:
		static std::shared_ptr<ChildWindow> _win_instance;

	private:
		HistoryBuffer _hist_buffer;
		std::vector<HWND> _widgets;
		int32_t _scroll_pos{};
		int32_t _old_scroll_pos{};

	private:
		WNDCLASSEX _wnd_class;
		HINSTANCE _hinstance;

	private:
		std::wstring _class_name, _window_name;

		HWND _hwnd{ nullptr }, _hwnd_parent{ nullptr }, _hwnd_visual{ nullptr };
		HMENU _hmenu{ nullptr };
		int32_t _x{}, _y{}, _width{}, _height{};

		HWND _hwnd_label{ nullptr }, _hwnd_edit{ nullptr }, _hwnd_button_copy{ nullptr }, _hwnd_button_delete{ nullptr };
		HWND _hwnd_scroll{ nullptr };

		HFONT _font_edit{ nullptr }, _font_title{ nullptr };
		HBRUSH _brush_bg{ nullptr }, _brush_title{ nullptr }, _brush_label_bg{ nullptr }, _brush_edit_bg{ nullptr };
	};
}
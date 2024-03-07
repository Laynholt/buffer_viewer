#pragma once
#include <cstdint>

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#include <iostream>
#endif // DEBUG_MODE


namespace winapp
{
	enum CustomMessage
	{
		MESSAGE_MENU_CREATE = WM_USER + 0x0001,
		MESSAGE_MENU_HISTORY,
		MESSAGE_MENU_ABOUT,
		
		MESSAGE_TRAY_CALLBACK = WM_USER + 0x0004,
		MESSAGE_TRAY_OPEN_HISTORY,
		MESSAGE_TRAY_CLEAR_HISTORY,
		MESSAGE_TRAY_CLOSE_APP,

		MESSAGE_SEND_TEXT,
		MESSAGE_SEND_FILEPATH,
		MESSAGE_SEND_IMAGE,
		MESSAGE_GET_CUR_DATA_FROM_HISTORY,
		MESSAGE_DEL_CUR_DATA_FROM_HISTORY
	};

	constexpr int16_t history_buffer_max_size = 50;
	constexpr int16_t history_buffer_max_symbols_size = 800;
}

namespace config
{
	constexpr const wchar_t* program_version = L"1.4.2";
	constexpr const wchar_t* program_date = L"02/2024";

	namespace main_window 
	{
		constexpr int16_t label_x = 0;
		constexpr int16_t label_y = 0;
		constexpr int16_t label_w = 420;
		constexpr int16_t label_h = 30;

		constexpr int16_t edit_padding_x = 10;
		constexpr int16_t edit_padding_y = 10;
		constexpr int16_t edit_text_padding = 5;

		constexpr int16_t edit_x = edit_padding_x;
		constexpr int16_t edit_y = label_h + edit_padding_y;

		constexpr int16_t edit_delw = 2 * edit_padding_x;
		constexpr int16_t edit_delh = 2 * edit_padding_y;

		constexpr int16_t edit_w = label_w - edit_delw;
		constexpr int16_t edit_h = 300;

		constexpr int16_t main_window_width = label_w;
		constexpr int16_t main_window_height = label_h + edit_h + edit_delh;

		namespace color
		{
			constexpr unsigned long window_bg = 0x1f1f23;

			constexpr unsigned long label_fg = 0xe6e6e6;
			constexpr unsigned long label_bg = 0x010b75;//0x282828;

			constexpr unsigned long edit_fg = 0xe6e6e6;
			constexpr unsigned long edit_bg = 0x323232;

			constexpr unsigned long scrollbar_bg = 0x404040;
		}
	}

	namespace child_window
	{
		constexpr int16_t child_window_width = 500;
		constexpr int16_t child_window_height = 650;

		constexpr int16_t main_title_rect_x = 0;
		constexpr int16_t main_title_rect_y = 0;
		constexpr int16_t main_title_rect_w = child_window_width;
		constexpr int16_t main_title_rect_h = 30;

		constexpr int16_t main_label_x = 0;
		constexpr int16_t main_label_y = 0;
		constexpr int16_t main_label_w = 190;
		constexpr int16_t main_label_h = 30;

		constexpr int16_t main_edit_x = main_label_w;
		constexpr int16_t main_edit_y = 7;
		constexpr int16_t main_edit_w = 50;
		constexpr int16_t main_edit_h = 20;

		constexpr int16_t main_button1_x = main_label_w + main_edit_w + 10;
		constexpr int16_t main_button1_y = 0;
		constexpr int16_t main_button1_w = 115;
		constexpr int16_t main_button1_h = 30;

		constexpr int16_t main_button2_x = main_button1_x + main_button1_w;
		constexpr int16_t main_button2_y = 0;
		constexpr int16_t main_button2_w = 115;
		constexpr int16_t main_button2_h = 30;

		constexpr int16_t scroll_x = child_window_width - 40;
		constexpr int16_t scroll_y = main_title_rect_h;
		constexpr int16_t scroll_w = 20;
		constexpr int16_t scroll_h = child_window_height - main_title_rect_h - 40;

		constexpr int16_t label_x = 0;
		constexpr int16_t label_y = 0;
		constexpr int16_t label_w = scroll_x;
		constexpr int16_t label_h = 30;

		constexpr int16_t edit_padding_x = 10;
		constexpr int16_t edit_padding_y = 10;
		constexpr int16_t edit_text_padding = 5;

		constexpr int16_t edit_x = edit_padding_x;
		constexpr int16_t edit_y = edit_padding_y;
		constexpr int16_t edit_w = 450;
		constexpr int16_t edit_h = 300;

		constexpr int16_t widget_padding_h = 30;
		constexpr int16_t widget_start_h = 20;

		constexpr int16_t scroll_step = 20;

		namespace color
		{
			constexpr unsigned long title_fg = 0xe6e6e6;
			constexpr unsigned long title_bg = 0x3e3648;//0x282828;
		}
	}
}
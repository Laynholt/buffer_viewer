#include "ChildWindow.h"

std::shared_ptr<winapp::ChildWindow> winapp::ChildWindow::_win_instance = nullptr;

winapp::ChildWindow::ChildWindow(LPCWSTR class_name, LPCWSTR window_name, int32_t x, int32_t y, int32_t width,
	int32_t height, HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, DWORD style, LPVOID param) :
	_class_name(class_name), _window_name(window_name), _x(x), _y(y), _width(width), _height(_height),
	_hwnd_parent(hwnd_parent), _hmenu(_hmenu), _hinstance(hinstance)
{
	_brush_bg = CreateSolidBrush(config::main_window::color::window_bg);
	_brush_title = CreateSolidBrush(config::child_window::color::title_bg);
	_brush_label_bg = CreateSolidBrush(config::main_window::color::label_bg);
	_brush_edit_bg = CreateSolidBrush(config::main_window::color::edit_bg);

	// ������������ ������
	_registration(_hinstance);

	_hwnd = CreateWindow(class_name, window_name, style, x, y, width, height, hwnd_parent, hmenu, hinstance, param);

	// ��������� � ������������� ������ �� �����
	HICON hicon = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_ICON1));
	if (hicon)
	{
		// ������������� ������� ������
		SendMessage(_hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hicon));
		// ������������� ��������� ������
		SendMessage(_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hicon));
	}

	_hwnd_label = CreateWindow(L"STATIC", L"������� ����� ��������:", WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | BS_PUSHBUTTON,
		config::child_window::main_label_x, config::child_window::main_label_y, config::child_window::main_label_w, config::child_window::main_label_h,
		_hwnd, nullptr, nullptr, nullptr);
	_hwnd_edit = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_CENTER | WS_BORDER,
		config::child_window::main_edit_x, config::child_window::main_edit_y, config::child_window::main_edit_w, config::child_window::main_edit_h,
		_hwnd, nullptr, nullptr, nullptr);
	_hwnd_button = CreateWindow(L"BUTTON", L"����������� � ����� ������", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		config::child_window::main_button_x, config::child_window::main_button_y, config::child_window::main_button_w, config::child_window::main_button_h,
		_hwnd, reinterpret_cast<HMENU>(MESSAGE_GET_DATA_FROM_HISTORY), nullptr, nullptr);

	SetWindowSubclass(_hwnd_edit, EditSubClassProc, NULL, NULL);

	_font_title = CreateFont(20, 8, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

	_font_edit = CreateFont(15, 6, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

	SendMessage(_hwnd_label, WM_SETFONT, reinterpret_cast<WPARAM>(_font_title), TRUE);
	SendMessage(_hwnd_edit, WM_SETFONT, reinterpret_cast<WPARAM>(_font_title), TRUE);
	SendMessage(_hwnd_button, WM_SETFONT, reinterpret_cast<WPARAM>(_font_title), TRUE);

	_hwnd_visual = CreateWindow(L"SubVisualWindow", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		0, 0, width, height, _hwnd, nullptr, nullptr, nullptr);
	_update_scroll_size();

	// �������� ����� ����
	LONG_PTR window_styles = GetWindowLongPtr(_hwnd, GWL_STYLE);
	// ������� �����, ��������� � ������� "��������� �� ���� �����" �
	// � ������������ ��������� ���� ����� ����������� � ������� ������ ��� ��� ������� win + <</>>
	window_styles &= ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
	// ��������� ���������� ����� � ����
	SetWindowLongPtr(_hwnd, GWL_STYLE, window_styles);
	// ����������� ���� � ����� ������
	SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// ������� ������ ���� ���������� ����, ���������� �� ��������� �������� � ������������
	HMENU hSystemMenu = GetSystemMenu(_hwnd, FALSE);
	if (hSystemMenu != NULL)
	{
		RemoveMenu(hSystemMenu, SC_SIZE, MF_BYCOMMAND);  // ������� ��������� �������
	}
	
}

std::shared_ptr<winapp::ChildWindow> winapp::ChildWindow::create(LPCWSTR class_name, LPCWSTR window_name, DWORD style, int32_t x, int32_t y,
	int32_t width, int32_t height, HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, LPVOID param)
{
	if (_win_instance.get() == nullptr)
	{
		_win_instance = std::shared_ptr<ChildWindow>(new ChildWindow(class_name, window_name,
			x, y, width, height, hwnd_parent, hmenu, hinstance, WS_OVERLAPPEDWINDOW, param));
	}
	return _win_instance;
}

std::shared_ptr<winapp::ChildWindow> winapp::ChildWindow::get_instance()
{
	return _win_instance;
}

winapp::ChildWindow::~ChildWindow()
{
	if (_font_title != nullptr)
		DeleteObject(_font_title);

	if (_font_edit != nullptr)
		DeleteObject(_font_edit);

	if (_brush_bg != nullptr)
		DeleteObject(_brush_bg);

	if (_brush_title != nullptr)
		DeleteObject(_brush_title);

	if (_brush_label_bg != nullptr)
		DeleteObject(_brush_label_bg);

	if (_brush_edit_bg != nullptr)
		DeleteObject(_brush_edit_bg);
}

void winapp::ChildWindow::_registration(HINSTANCE hinstance)
{
	// ������� ����� ��������� ����
	_wnd_class.cbSize = sizeof(WNDCLASSEX);
	_wnd_class.style = CS_HREDRAW | CS_VREDRAW;
	_wnd_class.lpfnWndProc = static_cast<WNDPROC>(ChildWindowEventHander);
	_wnd_class.cbClsExtra = 0;
	_wnd_class.cbWndExtra = 0;
	_wnd_class.hInstance = hinstance;
	_wnd_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	_wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	_wnd_class.hbrBackground = _brush_title;
	_wnd_class.lpszMenuName = NULL;
	_wnd_class.lpszClassName = _class_name.c_str();
	_wnd_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// ������������ ����� ��������� ����
	RegisterClassEx(&_wnd_class);

	WNDCLASSEX _wnd_class_visual;
	// ������� ����� ����������� ����
	_wnd_class_visual.cbSize = sizeof(WNDCLASSEX);
	_wnd_class_visual.style = CS_HREDRAW | CS_VREDRAW;
	_wnd_class_visual.lpfnWndProc = static_cast<WNDPROC>(SubChildWindowEventHander);
	_wnd_class_visual.cbClsExtra = 0;
	_wnd_class_visual.cbWndExtra = 0;
	_wnd_class_visual.hInstance = hinstance;
	_wnd_class_visual.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	_wnd_class_visual.hCursor = LoadCursor(NULL, IDC_ARROW);
	_wnd_class_visual.hbrBackground = _brush_bg;
	_wnd_class_visual.lpszMenuName = NULL;
	_wnd_class_visual.lpszClassName = L"SubVisualWindow";
	_wnd_class_visual.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// ������������ ����� ����������� ����
	RegisterClassEx(&_wnd_class_visual);
}

LRESULT CALLBACK winapp::ChildWindow::ChildWindowEventHander(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static std::pair<BaseData*, DataType> _temp_history_object_data{};

	switch (message)
	{
	case WM_CREATE:
		return 0;

	case WM_SIZE:
	{
		// �������� ����� ������� ����
		int16_t new_width = LOWORD(lparam);
		int16_t new_height = HIWORD(lparam);

		// �������� ������� title button
		SetWindowPos(_win_instance->_hwnd_button, nullptr,
			config::child_window::main_button_x, config::child_window::main_button_y,
			new_width - config::child_window::main_button_x, config::child_window::main_button_h, SWP_NOZORDER);

		// �������� ������� � ��������� Visual window
		SetWindowPos(_win_instance->_hwnd_visual, nullptr,
			0, config::child_window::main_title_rect_h, new_width, new_height - config::child_window::main_title_rect_h, SWP_NOZORDER);
		break;
	}

	case MESSAGE_SEND_TEXT:
	case MESSAGE_SEND_FILEPATH:
	case MESSAGE_SEND_IMAGE:
	{	
		if (message == MESSAGE_SEND_TEXT)
			_win_instance->_hist_buffer.add_text_data(reinterpret_cast<LPWSTR>(lparam));
		else if (message == MESSAGE_SEND_FILEPATH)
			_win_instance->_hist_buffer.add_filepath_data(reinterpret_cast<LPWSTR>(lparam));
		else if (message == MESSAGE_SEND_IMAGE)
			_win_instance->_hist_buffer.add_image_data(reinterpret_cast<HBITMAP>(lparam));
		
		_win_instance->_widgets.push_back(CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | BS_PUSHBUTTON,
			config::child_window::label_x, config::child_window::label_y, config::child_window::label_w, config::child_window::label_h,
			_win_instance->_hwnd_visual, nullptr, nullptr, nullptr));
		SendMessage(_win_instance->_widgets.back(), WM_SETFONT, reinterpret_cast<WPARAM>(_win_instance->_font_title), TRUE);

		if (message == MESSAGE_SEND_TEXT || message == MESSAGE_SEND_FILEPATH)
		{
			_win_instance->_widgets.push_back(CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
				config::child_window::edit_x, config::child_window::edit_y, config::child_window::edit_w, config::child_window::edit_h,
				_win_instance->_hwnd_visual, nullptr, nullptr, nullptr));

			SendMessage(_win_instance->_widgets.back(), WM_SETFONT, reinterpret_cast<WPARAM>(_win_instance->_font_edit), TRUE);
			SendMessage(_win_instance->_widgets.back(), EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(config::main_window::edit_text_padding,
				config::main_window::edit_text_padding));
		}
		_win_instance->_update_scroll_size();

		InvalidateRect(_win_instance->_hwnd_visual, nullptr, TRUE);
		break;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case MESSAGE_GET_DATA_FROM_HISTORY:
		{
			WCHAR buffer[5]{};
			GetWindowText(_win_instance->_hwnd_edit, buffer, 5);
			std::wstring wbuffer = buffer;

			if (wbuffer.empty())
				break;
			
			int16_t index = std::stoi(wbuffer);
			if (index < 0 || index >= _win_instance->_hist_buffer.get_size())
			{
				MessageBox(hwnd, L"������������ ������!", L"Info", MB_OK | MB_ICONINFORMATION);
				break;
			}

			_temp_history_object_data = _win_instance->_hist_buffer.get_object(index); // _win_instance->_hist_buffer.get_size() - 1 - index
			SendMessage(_win_instance->_hwnd_parent, MESSAGE_GET_DATA_FROM_HISTORY, NULL, reinterpret_cast<LPARAM>(&_temp_history_object_data));
			break;
		}
		default:
			break;
		}
		break;
	}

	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORBTN:
	{
		HDC hdc = reinterpret_cast<HDC>(wparam);

		SetTextColor(hdc, config::child_window::color::title_fg);
		SetBkColor(hdc, config::child_window::color::title_bg);
		return reinterpret_cast<LRESULT>(_win_instance->_brush_title);
	}

	case WM_CLOSE:
		ShowWindow(hwnd, SW_HIDE);
		return 0;

	case WM_DESTROY:
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT CALLBACK winapp::ChildWindow::SubChildWindowEventHander(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CREATE:
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		int32_t _current_widget_pos = config::child_window::widget_start_h;
		int16_t couter{}, rev_counter{};
		int16_t wcouter{};

		int16_t pos = _win_instance->_scroll_pos;
		rev_counter = _win_instance->_hist_buffer.get_size() - 1;

		//for (int16_t i = rev_counter; i >= 0; --i)
		for (int16_t i = 0; i <= rev_counter; ++i)
		{
			auto data = _win_instance->_hist_buffer.get_object(i);

			std::wstring str_num = std::to_wstring(couter);
			std::wstring title = data.second == DataType::TEXT ? L"] �����:" : data.second == DataType::FILE_PATH ? L"] ����:" : L"] �����������";
			title = str_num + title;

			if (data.second == DataType::TEXT || data.second == DataType::FILE_PATH)
			{
				SetWindowText(_win_instance->_widgets[wcouter], title.c_str());
				SetWindowPos(_win_instance->_widgets[wcouter++], nullptr,
					config::child_window::label_x, _current_widget_pos - pos, config::child_window::label_w, config::child_window::label_h, SWP_NOZORDER);

				_current_widget_pos += config::child_window::label_h + config::child_window::edit_padding_y;

				SetWindowText(_win_instance->_widgets[wcouter], static_cast<TextData*>(data.first)->get_data().c_str());
				SetWindowPos(_win_instance->_widgets[wcouter++], nullptr,
					config::child_window::edit_x, _current_widget_pos - pos, config::child_window::edit_w, config::child_window::edit_h, SWP_NOZORDER);

				_current_widget_pos += config::child_window::edit_h + config::child_window::widget_padding_h;
			}

			else if (data.second == DataType::IMAGE)
			{
				SetWindowText(_win_instance->_widgets[wcouter], title.c_str());
				SetWindowPos(_win_instance->_widgets[wcouter++], nullptr,
					config::child_window::label_x, _current_widget_pos - pos, config::child_window::label_w, config::child_window::label_h, SWP_NOZORDER);

				_current_widget_pos += config::child_window::label_h + config::child_window::edit_padding_y;

				HDC mdc = CreateCompatibleDC(hdc);
				HBITMAP hbitmap = static_cast<ImageData*>(data.first)->get_data();

				BITMAP bitmap;
				GetObject(hbitmap, sizeof(BITMAP), &bitmap);

				HBITMAP old_hbitmap = static_cast<HBITMAP>(SelectObject(mdc, hbitmap));

				SetStretchBltMode(hdc, STRETCH_HALFTONE);
				StretchBlt(hdc, config::child_window::edit_x, _current_widget_pos - pos, config::child_window::edit_w, config::child_window::edit_h,
					mdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

				SelectObject(mdc, old_hbitmap);
				DeleteDC(mdc);

				_current_widget_pos += config::child_window::edit_h + config::child_window::widget_padding_h;
			}
			++couter;
		}
		EndPaint(hwnd, &ps);
		UpdateWindow(hwnd);
	}
	break;

	case WM_VSCROLL:
	{
		int32_t pos = -1;

		switch (LOWORD(wparam))
		{
		case SB_LINEDOWN:
			pos = _win_instance->_scroll_pos + config::child_window::scroll_step;
			break;
		case SB_LINEUP:
			pos = _win_instance->_scroll_pos - config::child_window::scroll_step;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			pos = HIWORD(wparam);
			break;
		default:
			break;
		}

		if (pos == -1)
			break;
		
		_win_instance->_set_scroll_pos(pos);
		break;
	}

	case WM_MOUSEWHEEL:
	{
		int32_t delta = GET_WHEEL_DELTA_WPARAM(wparam);
		int32_t step = delta < 0 ? config::child_window::scroll_step : -config::child_window::scroll_step;
		int32_t pos = _win_instance->_scroll_pos + step;

		_win_instance->_set_scroll_pos(pos);
		break;
	}

	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = reinterpret_cast<HDC>(wparam);
		HWND hstatic = reinterpret_cast<HWND>(lparam);

		if (SendMessage(hstatic, WM_GETDLGCODE, 0, 0) & DLGC_STATIC)
		{
			SetTextColor(hdc, config::main_window::color::label_fg);
			SetBkColor(hdc, config::main_window::color::label_bg);
			return reinterpret_cast<LRESULT>(_win_instance->_brush_label_bg);
		}
		else
		{
			SetTextColor(hdc, config::main_window::color::edit_fg);
			SetBkColor(hdc, config::main_window::color::edit_bg);
			return reinterpret_cast<LRESULT>(_win_instance->_brush_edit_bg);
		}
	}

	case WM_DESTROY:
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

void winapp::ChildWindow::_set_scroll_pos(int32_t pos)
{
	RECT window_rect = { 0 };
	GetClientRect(_win_instance->_hwnd_visual, &window_rect);

	int32_t max_pos = _win_instance->_get_max_scroll_size() - (window_rect.bottom - window_rect.top);

	if (pos > max_pos)
		pos = max_pos;
	if (pos < 0)
		pos = 0;

	// https://stackoverflow.com/questions/32094254/how-to-control-scrollbar-in-vc-win32-api
	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	si.nPos = pos;
	si.nTrackPos = 0;
	SetScrollInfo(_win_instance->_hwnd_visual, SB_VERT, &si, true);

	ScrollWindow(_win_instance->_hwnd_visual, 0, -(pos - _win_instance->_scroll_pos), nullptr, nullptr);
	_win_instance->_scroll_pos = pos;

	UpdateWindow(_win_instance->_hwnd_visual);
}

void winapp::ChildWindow::_update_scroll_size()
{
	RECT window_rect = { 0 };
	GetClientRect(_hwnd_visual, &window_rect);
	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = _get_max_scroll_size();
	si.nPage = (window_rect.bottom - window_rect.top);
	si.nPos = _scroll_pos;
	si.nTrackPos = 0;
	SetScrollInfo(_hwnd_visual, SB_VERT, &si, true);
}

int32_t winapp::ChildWindow::_get_widget_pack_size()
{
	return config::child_window::label_h + config::child_window::edit_padding_y 
		+ config::child_window::edit_h + config::child_window::widget_padding_h;
}

int32_t winapp::ChildWindow::_get_max_scroll_size()
{
	return config::child_window::widget_start_h + _get_widget_pack_size() * _hist_buffer.get_size();
}

LRESULT CALLBACK winapp::ChildWindow::EditSubClassProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR idsubclass, DWORD_PTR refdata)
{
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_BACK:
			// ���� ����� Ctrl + backspace, ������� ���� ����� � �������� ���������� Edit
			if (GetKeyState(VK_CONTROL) < 0)
			{
				SetWindowText(hwnd, L"");
				break;
			}

		case 'A':
			// ���� ����� Ctrl + A, �������� ���� ����� � �������� ���������� Edit
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}
		
		break;

	case WM_CHAR:
	{
		// ���������� ����� ������ ��� ����
		if (iswdigit(wparam))
		{
			DWORD start, end;
			SendMessage(hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&end));
			if (start != end)
			{
				SendMessage(hwnd, EM_REPLACESEL, TRUE, reinterpret_cast <LPARAM>(L""));
			}

			WCHAR buffer[5]{};
			GetWindowText(hwnd, buffer, 5);
			std::wstring str = buffer;

			if (str.size() > 3)
				break;

			str += static_cast<wchar_t>(wparam);

			// ������������� ����� ����� � Edit
			SetWindowText(hwnd, str.c_str());
			// ������ ��������� �������
			SendMessage(hwnd, EM_SETSEL, str.size(), str.size());
			return 0;
		}
		else if (wparam == VK_BACK)
		{
			DWORD start, end;
			SendMessage(hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&end));
			if (start != end) 
			{
				SendMessage(hwnd, EM_REPLACESEL, TRUE, reinterpret_cast <LPARAM>(L""));
				return 0;
			}

			// ��������� ������������� ������� Backspace ��� �������� ��������
			WCHAR buffer[5]{};
			GetWindowText(hwnd, buffer, 5);
			std::wstring str = buffer;

			if (str.size() == 0)
				break;

			str.pop_back();
			// ������������� ����� ����� � Edit
			SetWindowText(hwnd, str.c_str());
			// ������ ��������� �������
			SendMessage(hwnd, EM_SETSEL, str.size(), str.size());
			return 0;
		}
		else
		{
			// ��������� ���� ������ ��������
			//MessageBeep(MB_ICONEXCLAMATION);
			return 0;
		}
		break;
	}

	case WM_NCDESTROY:
	{
		RemoveWindowSubclass(hwnd, EditSubClassProc, idsubclass);
		break;
	}
	}

	return DefSubclassProc(hwnd, message, wparam, lparam);
}
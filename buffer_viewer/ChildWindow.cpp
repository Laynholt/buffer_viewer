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

	// Регистрируем классы
	_registration(_hinstance);

	_hwnd = CreateWindow(class_name, window_name, style, x, y, width, height, hwnd_parent, hmenu, hinstance, param);

	// Загружаем и устанавливаем иконку из файла
	HICON hicon = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_ICON1));
	if (hicon)
	{
		// Устанавливаем большую иконку
		SendMessage(_hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hicon));
		// Устанавливаем маленькую иконку
		SendMessage(_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hicon));
	}

	_hwnd_label = CreateWindow(L"STATIC", L"Введите номер элемента:", WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | BS_PUSHBUTTON,
		config::child_window::main_label_x, config::child_window::main_label_y, config::child_window::main_label_w, config::child_window::main_label_h,
		_hwnd, nullptr, nullptr, nullptr);
	_hwnd_edit = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_CENTER | WS_BORDER,
		config::child_window::main_edit_x, config::child_window::main_edit_y, config::child_window::main_edit_w, config::child_window::main_edit_h,
		_hwnd, nullptr, nullptr, nullptr);
	_hwnd_button_copy = CreateWindow(L"BUTTON", L"Копировать", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		config::child_window::main_button1_x, config::child_window::main_button1_y, config::child_window::main_button1_w, config::child_window::main_button1_h,
		_hwnd, reinterpret_cast<HMENU>(MESSAGE_GET_CUR_DATA_FROM_HISTORY), nullptr, nullptr);
	_hwnd_button_delete = CreateWindow(L"BUTTON", L"Удалить", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		config::child_window::main_button2_x, config::child_window::main_button2_y, config::child_window::main_button2_w, config::child_window::main_button2_h,
		_hwnd, reinterpret_cast<HMENU>(MESSAGE_DEL_CUR_DATA_FROM_HISTORY), nullptr, nullptr);

	SetWindowSubclass(_hwnd_edit, EditSubClassProc, NULL, NULL);

	_font_title = CreateFont(20, 8, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

	_font_edit = CreateFont(15, 6, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

	SendMessage(_hwnd_label, WM_SETFONT, reinterpret_cast<WPARAM>(_font_title), TRUE);
	SendMessage(_hwnd_edit, WM_SETFONT, reinterpret_cast<WPARAM>(_font_title), TRUE);
	SendMessage(_hwnd_button_copy, WM_SETFONT, reinterpret_cast<WPARAM>(_font_title), TRUE);
	SendMessage(_hwnd_button_delete, WM_SETFONT, reinterpret_cast<WPARAM>(_font_title), TRUE);

	_hwnd_visual = CreateWindow(L"SubVisualWindow", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		0, 0, width, height, _hwnd, nullptr, nullptr, nullptr);
	_update_scroll_size();

	// Получаем стиль окна
	LONG_PTR window_styles = GetWindowLongPtr(_hwnd, GWL_STYLE);
	// Удаляем стиль, связанный с кнопкой "Увеличить во весь экран" и
	// с возможностью увеличить окно через прислонение к боковым частям или при нажатии win + <</>>
	window_styles &= ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
	// Применяем измененный стиль к окну
	SetWindowLongPtr(_hwnd, GWL_STYLE, window_styles);
	// Пересоздаем окно с новым стилем
	SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// Убираем пункты меню системного меню, отвечающие за изменение размеров и максимизацию
	HMENU hSystemMenu = GetSystemMenu(_hwnd, FALSE);
	if (hSystemMenu != NULL)
	{
		RemoveMenu(hSystemMenu, SC_SIZE, MF_BYCOMMAND);  // Убираем изменение размера
	}
	
}

std::shared_ptr<winapp::ChildWindow> winapp::ChildWindow::create(LPCWSTR class_name, LPCWSTR window_name, DWORD style, int32_t x, int32_t y,
	int32_t width, int32_t height, HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, LPVOID param)
{
	if (_win_instance.get() == nullptr)
	{
		_win_instance = std::shared_ptr<ChildWindow>(new ChildWindow(class_name, window_name,
			x, y, width, height, hwnd_parent, hmenu, hinstance, style, param));
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
	// Создаем класс дочернего окна
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

	// Регистрируем класс дочернего окна
	RegisterClassEx(&_wnd_class);

	WNDCLASSEX _wnd_class_visual;
	// Создаем класс встроенного окна
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

	// Регистрируем класс встроенного окна
	RegisterClassEx(&_wnd_class_visual);
}

LRESULT CALLBACK winapp::ChildWindow::ChildWindowEventHander(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CREATE:
		return 0;

	case WM_SIZE:
	{
		// Получаем новые размеры окна
		int16_t new_width = LOWORD(lparam);
		int16_t new_height = HIWORD(lparam);

		// Изменяем размеры title button
		SetWindowPos(_win_instance->_hwnd_button_copy, nullptr,
			config::child_window::main_button1_x, config::child_window::main_button1_y,
			config::child_window::main_button1_w, config::child_window::main_button1_h, SWP_NOZORDER);

		SetWindowPos(_win_instance->_hwnd_button_delete, nullptr,
			config::child_window::main_button2_x, config::child_window::main_button2_y,
			(new_width - config::child_window::main_button2_x), config::child_window::main_button2_h, SWP_NOZORDER);

		// Изменяем размеры и положение Visual window
		SetWindowPos(_win_instance->_hwnd_visual, nullptr,
			0, config::child_window::main_title_rect_h, new_width, new_height - config::child_window::main_title_rect_h, SWP_NOZORDER);
		break;
	}

	case MESSAGE_SEND_TEXT:
	case MESSAGE_SEND_FILEPATH:
	case MESSAGE_SEND_IMAGE:
	{	
		if (_win_instance->_hist_buffer.get_size() == winapp::history_buffer_max_size)
		{
			_win_instance->_delete_current_element(-1);
		}

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
			_win_instance->_widgets.push_back(CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
				config::child_window::edit_x, config::child_window::edit_y, config::child_window::edit_w, config::child_window::edit_h,
				_win_instance->_hwnd_visual, nullptr, nullptr, nullptr));

			SendMessage(_win_instance->_widgets.back(), WM_SETFONT, reinterpret_cast<WPARAM>(_win_instance->_font_edit), TRUE);
			SendMessage(_win_instance->_widgets.back(), EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(config::main_window::edit_text_padding,
				config::main_window::edit_text_padding));

			SetWindowSubclass(_win_instance->_widgets.back(), MultiEditSubClassProc, NULL, NULL);
		}
		_win_instance->_update_scroll_size();

		InvalidateRect(_win_instance->_hwnd_visual, nullptr, TRUE);
		break;
	}

	case MESSAGE_TRAY_CLEAR_HISTORY:
		_win_instance->_hist_buffer.clear();

		for (auto& widget : _win_instance->_widgets)
			DestroyWindow(widget);

		_win_instance->_widgets.clear();
		_win_instance->_widgets.shrink_to_fit();

		_win_instance->_scroll_pos = 0;
		_win_instance->_old_scroll_pos = 0;

		_win_instance->_update_scroll_size();
		InvalidateRect(_win_instance->_hwnd_visual, nullptr, TRUE);
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case MESSAGE_GET_CUR_DATA_FROM_HISTORY:
		{
			_win_instance->_event_handler_enter_key();
			break;
		}

		case MESSAGE_DEL_CUR_DATA_FROM_HISTORY:
		{
			_win_instance->_event_handler_delete_key();
			break;
		}

		default:
			break;
		}
		break;
	}

	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_RETURN:
		{
			_win_instance->_event_handler_enter_key();
			return 0;
		}
		case VK_DELETE:
		{
			_win_instance->_event_handler_delete_key();
			return 0;
		}
		case VK_BACK:
		{
			_win_instance->_event_handler_backspace_key();
			return 0;
		}
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9': case '0':
		{
			_win_instance->_event_handler_digit_keys(wparam);
			return 0;
		}
		case 'A':
		{
			_win_instance->_event_handler_ctrlA_key();
			return 0;
		}
		case VK_UP:
			_win_instance->_set_scroll_pos(_win_instance->_scroll_pos - _win_instance->_get_widget_pack_size());
			_win_instance->_old_scroll_pos = _win_instance->_scroll_pos;
			break;

		case VK_DOWN:
			_win_instance->_set_scroll_pos(_win_instance->_scroll_pos + _win_instance->_get_widget_pack_size());
			_win_instance->_old_scroll_pos = _win_instance->_scroll_pos;
			break;
		}
		break;

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

		int32_t _current_widget_pos = _win_instance->_get_max_scroll_size() - _win_instance->_get_widget_pack_size();
		int16_t buffer_size{};
		int16_t rev_counter{};
		int16_t wcouter{};

		int16_t pos = _win_instance->_scroll_pos;
		rev_counter = buffer_size = _win_instance->_hist_buffer.get_size() - 1;

		for (int16_t counter = 0; counter <= buffer_size; ++counter, --rev_counter)
		{
			auto data = _win_instance->_hist_buffer.get_object(counter);

			std::wstring str_num = std::to_wstring(rev_counter);
			std::wstring title = data.second == DataType::TEXT ? L"] Текст:" : data.second == DataType::FILE_PATH ? L"] Файл:" : L"] Изображение";
			title = str_num + title;

			if (data.second == DataType::TEXT || data.second == DataType::FILE_PATH)
			{
				SetWindowText(_win_instance->_widgets[wcouter], title.c_str());
				SetWindowPos(_win_instance->_widgets[wcouter++], nullptr,
					config::child_window::label_x, _current_widget_pos - pos, config::child_window::label_w, config::child_window::label_h, SWP_NOZORDER);

				_current_widget_pos += config::child_window::label_h + config::child_window::edit_padding_y;

				auto widget_full_text = static_cast<TextData*>(data.first)->get_data();
				std::wstring widget_view_text;

				if (widget_full_text.size() > winapp::history_buffer_max_symbols_size)
					widget_view_text = widget_full_text.substr(0, winapp::history_buffer_max_symbols_size);
				else
					widget_view_text = widget_full_text;

				SetWindowText(_win_instance->_widgets[wcouter], widget_view_text.c_str());
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
			_current_widget_pos -= 2 * _win_instance->_get_widget_pack_size();
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
		_win_instance->_old_scroll_pos = _win_instance->_scroll_pos;
		break;
	}

	case WM_MOUSEWHEEL:
	{
		int32_t delta = GET_WHEEL_DELTA_WPARAM(wparam);
		int32_t step = delta < 0 ? config::child_window::scroll_step : -config::child_window::scroll_step;
		int32_t pos = _win_instance->_scroll_pos + step;

		_win_instance->_set_scroll_pos(pos);
		_win_instance->_old_scroll_pos = _win_instance->_scroll_pos;
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

int32_t winapp::ChildWindow::_get_widget_pack_position(int32_t index)
{
	return config::child_window::widget_start_h + _get_widget_pack_size() * index;
}

int32_t winapp::ChildWindow::_get_widget_pack_size()
{
	return config::child_window::label_h + config::child_window::edit_padding_y 
		+ config::child_window::edit_h + config::child_window::widget_padding_h;
}

int32_t winapp::ChildWindow::_get_max_scroll_size()
{
	return _get_widget_pack_position(_hist_buffer.get_size());
}

std::wstring winapp::ChildWindow::_get_text_from_edit()
{
	WCHAR buffer[5]{};
	GetWindowText(_hwnd_edit, buffer, 5);
	std::wstring wbuffer = buffer;
	return wbuffer;
}

void winapp::ChildWindow::_delete_current_element(int32_t index)
{
	if (index == -1)
		index = 0;
	else
		index = _hist_buffer.get_size() - index - 1;

	auto first_object_data = _hist_buffer.get_object(index);
	if (first_object_data.second == DataType::TEXT || first_object_data.second == DataType::FILE_PATH)
	{
		DestroyWindow(*(_widgets.begin() + index));
		DestroyWindow(*(_widgets.begin() + index + 1));
		_widgets.erase(_widgets.begin() + index, _widgets.begin() + index + 2);
	}
	else if (first_object_data.second == DataType::IMAGE)
	{
		DestroyWindow(*(_widgets.begin() + index));
		_widgets.erase(_widgets.begin() + index);
	}
	_hist_buffer.erase(index);
}

void winapp::ChildWindow::_event_handler_digit_keys(WPARAM symbol)
{
	std::wstring str = _win_instance->_get_text_from_edit();

	if (str.size() > 3)
		return;

	str += static_cast<wchar_t>(symbol);

	// Устанавливаем новый текст в Edit
	SetWindowText(_win_instance->_hwnd_edit, str.c_str());
	// Меняем положение курсора
	SendMessage(_win_instance->_hwnd_edit, EM_SETSEL, str.size(), str.size());

	int16_t index = std::stoi(str);
	if (index < 0 || index >= _win_instance->_hist_buffer.get_size())
		_win_instance->_set_scroll_pos(_win_instance->_old_scroll_pos);
	else
		_win_instance->_set_scroll_pos(_win_instance->_get_widget_pack_position(index));
}

void winapp::ChildWindow::_event_handler_backspace_key()
{
	// Если нажат Ctrl + backspace, удаляем весь текст в элементе управления Edit
	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		SetWindowText(_win_instance->_hwnd_edit, L"");
		_win_instance->_set_scroll_pos(_win_instance->_old_scroll_pos);
		return;
	}

	bool was_removed = false;
	DWORD start, end;
	SendMessage(_win_instance->_hwnd_edit, EM_GETSEL, reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&end));
	if (start != end)
	{
		SendMessage(_win_instance->_hwnd_edit, EM_REPLACESEL, TRUE, reinterpret_cast <LPARAM>(L""));
		was_removed = true;
	}

	// Разрешаем использование клавиши Backspace для удаления символов
	std::wstring str = _win_instance->_get_text_from_edit();

	if (str.empty() && !was_removed)
		return;
	else if (str.empty())
	{
		_win_instance->_set_scroll_pos(_win_instance->_old_scroll_pos);
	}
	else
	{
		if (!was_removed)
		{
			str.pop_back();
			// Устанавливаем новый текст в Edit
			SetWindowText(_win_instance->_hwnd_edit, str.c_str());
			// Меняем положение курсора
			SendMessage(_win_instance->_hwnd_edit, EM_SETSEL, str.size(), str.size());
		
			if (str.empty())
				_win_instance->_set_scroll_pos(_win_instance->_old_scroll_pos);
			else
			{
				int16_t index = std::stoi(str);
				if (index < _win_instance->_hist_buffer.get_size())
					_win_instance->_set_scroll_pos(_win_instance->_get_widget_pack_position(index));
			}
			return;
		}

		int16_t index = std::stoi(str);
		if (index < _win_instance->_hist_buffer.get_size())
			_win_instance->_set_scroll_pos(_win_instance->_get_widget_pack_position(index));
	}
}

void winapp::ChildWindow::_event_handler_enter_key()
{
	static std::pair<BaseData*, DataType> _temp_history_object_data{};

	SetFocus(_win_instance->_hwnd);

	std::wstring str = _win_instance->_get_text_from_edit();
	if (str.empty())
		return;

	SetWindowText(_win_instance->_hwnd_edit, L"");

	int16_t index = std::stoi(str);
	if (index < 0 || index >= _win_instance->_hist_buffer.get_size())
	{
		MessageBox(_win_instance->_hwnd, L"Некорректный индекс!", L"Info", MB_OK | MB_ICONINFORMATION);
		return;
	}

	_win_instance->_old_scroll_pos = _win_instance->_scroll_pos;

	_temp_history_object_data = _win_instance->_hist_buffer.get_object(_win_instance->_hist_buffer.get_size() - 1 - index);
	SendMessage(_win_instance->_hwnd_parent, MESSAGE_GET_CUR_DATA_FROM_HISTORY, NULL, reinterpret_cast<LPARAM>(&_temp_history_object_data));
}

void winapp::ChildWindow::_event_handler_ctrlA_key()
{
	// Если нажат Ctrl + A, выделяем весь текст в элементе управления Edit
	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		SendMessage(_win_instance->_hwnd_edit, EM_SETSEL, 0, -1);
	}
}

void winapp::ChildWindow::_event_handler_delete_key()
{
	SetFocus(_win_instance->_hwnd);

	std::wstring str = _win_instance->_get_text_from_edit();
	if (str.empty())
		return;

	SetWindowText(_win_instance->_hwnd_edit, L"");

	int16_t index = std::stoi(str);
	if (index < 0 || index >= _win_instance->_hist_buffer.get_size())
	{
		MessageBox(_win_instance->_hwnd, L"Некорректный индекс!", L"Info", MB_OK | MB_ICONINFORMATION);
		return;
	}

	_win_instance->_delete_current_element(index);
	_win_instance->_update_scroll_size();
	_win_instance->_set_scroll_pos(_win_instance->_get_widget_pack_position(index));
	_win_instance->_old_scroll_pos = _win_instance->_scroll_pos;

	InvalidateRect(_win_instance->_hwnd_visual, nullptr, TRUE);
}

LRESULT CALLBACK winapp::ChildWindow::MultiEditSubClassProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR idsubclass, DWORD_PTR refdata)
{
	switch (message)
	{
	case WM_MOUSEWHEEL:
		SendMessage(_win_instance->_hwnd_visual, WM_MOUSEWHEEL, wparam, lparam);
		break;

	case WM_VSCROLL:
		SendMessage(_win_instance->_hwnd_visual, WM_VSCROLL, wparam, lparam);
		break;

	case WM_KEYDOWN:
		SetFocus(_win_instance->_hwnd);
		SendMessage(_win_instance->_hwnd, WM_KEYDOWN, wparam, lparam);
		break;

	case WM_NCDESTROY:
	{
		RemoveWindowSubclass(hwnd, MultiEditSubClassProc, idsubclass);
		break;
	}
	}

	return DefSubclassProc(hwnd, message, wparam, lparam);
}

LRESULT CALLBACK winapp::ChildWindow::EditSubClassProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR idsubclass, DWORD_PTR refdata)
{
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_RETURN:
		{
			_win_instance->_event_handler_enter_key();
			return 0;
		}

		case VK_BACK:
		{
			_win_instance->_event_handler_backspace_key();
			return 0;
		}

		case 'A':
			_win_instance->_event_handler_ctrlA_key();
			return 0;
		}
		
		break;

	case WM_CHAR:
	{
		// Фильтрация ввода только для цифр
		if (iswdigit(wparam))
		{
			_win_instance->_event_handler_digit_keys(wparam);
			return 0;
		}
		else
		{
			// Запрещаем ввод других символов
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
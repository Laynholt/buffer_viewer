#include "Window.h"

std::shared_ptr<winapp::Window> winapp::Window::_win_instance = nullptr;

winapp::Window::Window(LPCWSTR class_name, LPCWSTR window_name, int32_t x, int32_t y, int32_t width,
    int32_t height, HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, DWORD style, LPVOID param) :
    _class_name(class_name), _window_name(window_name), _x(x), _y(y), _width(width), _height(_height),
    _hwnd_parent(hwnd_parent), _hmenu(_hmenu), _hinstance(hinstance)
{
	_brush_bg = CreateSolidBrush(config::main_window::color::window_bg);

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

    _hwnd_label = CreateWindow(L"STATIC", L"Скопируйте что-нибудь в буфер обмена.", WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | BS_PUSHBUTTON,
        config::main_window::label_x, config::main_window::label_y, config::main_window::label_w, config::main_window::label_h,
        _hwnd, nullptr, nullptr, nullptr);
    _hwnd_edit = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        config::main_window::edit_x, config::main_window::edit_y, config::main_window::edit_w, config::main_window::edit_h,
        _hwnd, nullptr, nullptr, nullptr);

    _font_label = CreateFont(25, 10, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
	_font_edit = CreateFont(15, 6, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SendMessage(_hwnd_label, WM_SETFONT, reinterpret_cast<WPARAM>(_font_label), TRUE);
    SendMessage(_hwnd_edit, WM_SETFONT, reinterpret_cast<WPARAM>(_font_edit), TRUE);

	_brush_label_bg = CreateSolidBrush(config::main_window::color::label_bg);
	_brush_edit_bg = CreateSolidBrush(config::main_window::color::edit_bg);

    SendMessage(_hwnd_edit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(config::main_window::edit_text_padding,
        config::main_window::edit_text_padding));

	_hmenu = CreateMenu();
	AppendMenu(_hmenu, MF_STRING, MESSAGE_MENU_HISTORY, L"Открыть историю");
	AppendMenu(_hmenu, MF_STRING, MESSAGE_MENU_ABOUT, L"Справка");

	// Инициализация данных для иконки в трее
	_notify_data.cbSize = sizeof(NOTIFYICONDATA);
	_notify_data.hWnd = _hwnd;
	_notify_data.uID = 1;
	_notify_data.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	_notify_data.uCallbackMessage = MESSAGE_TRAY_CALLBACK;
	_notify_data.hIcon = LoadIcon(_hinstance, MAKEINTRESOURCE(IDI_ICON1));
	lstrcpy(_notify_data.szTip, TEXT("Буфер-Гляделка"));

	// Добавление иконки в трей
	Shell_NotifyIcon(NIM_ADD, &_notify_data);

	// Создание контекстного меню
	_hmenu_tray = CreatePopupMenu();
	AppendMenu(_hmenu_tray, MF_STRING, MESSAGE_TRAY_OPEN_HISTORY, TEXT("Открыть историю"));
	AppendMenu(_hmenu_tray, MF_STRING, MESSAGE_TRAY_CLEAR_HISTORY, TEXT("Очистить историю"));
	AppendMenu(_hmenu_tray, MF_STRING, MESSAGE_TRAY_CLOSE_APP, TEXT("Выход"));

	_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, _hinstance, NULL);
}

LRESULT winapp::Window::KeyboardHookProc(int32_t code, WPARAM wparam, LPARAM lparam)
{
	if (code >= 0) 
	{
		// Обработка клавиш
		KBDLLHOOKSTRUCT* key_info = reinterpret_cast<KBDLLHOOKSTRUCT*>(lparam);
		// Проверка на сочетание клавиш Alt + D
		if (key_info->vkCode == 'D' && (GetAsyncKeyState(VK_MENU) & 0x8000)) // && (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			ShowWindow(_win_instance->_child, SW_SHOWNORMAL);
		}
		else if (key_info->vkCode == 'E' && (GetAsyncKeyState(VK_MENU) & 0x8000))
		{
			ShowWindow(_win_instance->_child, SW_HIDE);
		}

		if (key_info->vkCode == 'C' && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
		{
			_win_instance->_ctrl_c_pressed_flag = true;
		}
		else
		{
			_win_instance->_ctrl_c_pressed_flag = false;
			_win_instance->_was_copied_from_buffer = false;
		}
	}

	// Передать управление следующему хуку в цепочке
	return CallNextHookEx(_win_instance->_hook, code, wparam, lparam);
}


std::shared_ptr<winapp::Window> winapp::Window::create(LPCWSTR class_name, LPCWSTR window_name, DWORD style, int32_t x, int32_t y,
    int32_t width, int32_t height, HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, LPVOID param)
{
    if (_win_instance.get() == nullptr)
    {
        _win_instance = std::shared_ptr<Window>(new Window(class_name, window_name,
            x, y, width, height, hwnd_parent, hmenu, hinstance, style, param));
		
		SendMessage(_win_instance->_hwnd, MESSAGE_MENU_CREATE, NULL, NULL);
    }
    return _win_instance;
}

std::shared_ptr<winapp::Window> winapp::Window::get_instance()
{
    return _win_instance;
}

winapp::Window::~Window()
{
    if (_font_label != nullptr)
        DeleteObject(_font_label);

	if (_font_edit != nullptr)
		DeleteObject(_font_edit);

	if (_brush_bg != nullptr)
		DeleteObject(_brush_bg);

	if (_brush_label_bg != nullptr)
		DeleteObject(_brush_label_bg);

	if (_brush_edit_bg != nullptr)
		DeleteObject(_brush_edit_bg);

	if (_hook != nullptr)
		UnhookWindowsHookEx(_hook);
}

void winapp::Window::set_label_text(LPCWSTR text)
{
	SetWindowText(_hwnd_label, text);
}

void winapp::Window::set_edit_text(LPCWSTR text)
{
	SetWindowText(_hwnd_edit, text);
}

void winapp::Window::set_child_window(HWND child)
{
	_child = child;
}

void winapp::Window::_registration(HINSTANCE hinstance)
{
    // Создаем класс основного окна
    _wnd_class.cbSize = sizeof(WNDCLASSEX);
    _wnd_class.style = CS_HREDRAW | CS_VREDRAW;
    _wnd_class.lpfnWndProc = static_cast<WNDPROC>(MainWindowEventHander);
    _wnd_class.cbClsExtra = 0;
    _wnd_class.cbWndExtra = 0;
    _wnd_class.hInstance = hinstance;
    _wnd_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    _wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	_wnd_class.hbrBackground = _brush_bg;
    _wnd_class.lpszMenuName = NULL;
    _wnd_class.lpszClassName = _class_name.c_str();
    _wnd_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // Регистрируем класс основного окна
    RegisterClassEx(&_wnd_class);
}

LRESULT CALLBACK winapp::Window::MainWindowEventHander(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static LPWSTR text{};
	static HBITMAP hbitmap1{}, hbitmap2{};
	static std::wstring file_paths;

	static bool crutch_against_event_duplication_when_self_recording_to_the_buffer{ false };

	switch (message)
	{
	case WM_CREATE:	// создание основного окна
	{
		AddClipboardFormatListener(hwnd);
		return 0;
	}

	case WM_CLIPBOARDUPDATE:
	{
		if (_win_instance->_ctrl_c_pressed_flag)
		{
			if (_win_instance->_was_copied_from_buffer)
				break;
			else
				_win_instance->_was_copied_from_buffer = true;
		}

		if (wparam == 1)
		{
			if (crutch_against_event_duplication_when_self_recording_to_the_buffer)
			{
				crutch_against_event_duplication_when_self_recording_to_the_buffer = 0;
				break;
			}
			crutch_against_event_duplication_when_self_recording_to_the_buffer = 1;
		}
		else
			if (crutch_against_event_duplication_when_self_recording_to_the_buffer)
			{
				crutch_against_event_duplication_when_self_recording_to_the_buffer = 0;
				break;
			}

		bool is_text{ false };
		bool is_image{ false };
		bool is_file{ false };

		if (OpenClipboard(hwnd))
		{
			// Проверяем наличие текста в буфере обмена
			if (IsClipboardFormatAvailable(CF_TEXT) || IsClipboardFormatAvailable(CF_UNICODETEXT))
				is_text = true;
			else if (IsClipboardFormatAvailable(CF_BITMAP))
				is_image = true;
			else if (IsClipboardFormatAvailable(CF_HDROP))
				is_file = true;

			if (is_text)
			{
				// Получаем текст из буфера обмена
				HANDLE hdata{};

				if (IsClipboardFormatAvailable(CF_UNICODETEXT))
					hdata = GetClipboardData(CF_UNICODETEXT);
				else if (IsClipboardFormatAvailable(CF_TEXT))
					hdata = GetClipboardData(CF_TEXT);

				if (hdata != nullptr)
				{
					text = static_cast<LPWSTR>(GlobalLock(hdata));
					if (text != nullptr)
					{
						_win_instance->set_label_text(L"В буфере обмена сейчас текст:");
						ShowWindow(_win_instance->get_hwnd_edit(), SW_SHOW);
						_win_instance->set_edit_text(text);

						GlobalUnlock(hdata);
						SendMessage(_win_instance->_child, MESSAGE_SEND_TEXT, NULL, reinterpret_cast<LPARAM>(text));
					}
					
				}
			}
			else if (is_file)
			{
				// Получаем список файлов из буфера обмена
				HANDLE hdata = GetClipboardData(CF_HDROP);
				if (hdata != nullptr)
				{
					HDROP hdrop = static_cast<HDROP>(GlobalLock(hdata));
					if (hdrop != nullptr)
					{
						// Получаем количество файлов в списке
						UINT num_files = DragQueryFile(hdrop, 0xFFFFFFFF, nullptr, 0);
						WCHAR file_path[MAX_PATH]{};

						file_paths.clear();
						for (int16_t i = 0; i < num_files; ++i)
						{
							if (i > 0)
								file_paths += L"\r\n";

							// Получаем путь к i-ому файлу в списке
							DragQueryFile(hdrop, i, file_path, MAX_PATH);
							file_paths += file_path;
						}
						_win_instance->set_label_text(L"В буфере обмена сейчас файл(ы):");
						ShowWindow(_win_instance->get_hwnd_edit(), SW_SHOW);
						_win_instance->set_edit_text(file_paths.c_str());

						GlobalUnlock(hdata);
						SendMessage(_win_instance->_child, MESSAGE_SEND_FILEPATH, NULL, reinterpret_cast<LPARAM>(file_paths.c_str()));
					}
				}
			}
			else if (is_image)
			{
				// Получаем изображение из буфера обмена
				hbitmap1 = static_cast<HBITMAP>(GetClipboardData(CF_BITMAP));

				if (hbitmap1 != nullptr)
				{
					_win_instance->set_label_text(L"В буфере обмена сейчас изображение:");
					ShowWindow(_win_instance->get_hwnd_edit(), SW_HIDE);
					SendMessage(_win_instance->_child, MESSAGE_SEND_IMAGE, NULL, reinterpret_cast<LPARAM>(hbitmap1));
				}
			}

			CloseClipboard();
		}
		InvalidateRect(hwnd, nullptr, TRUE);
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		if (hbitmap1)
		{			
			BITMAP bitmap;
			GetObject(hbitmap1, sizeof(BITMAP), &bitmap);
		
			HDC mdc = CreateCompatibleDC(hdc);
			HBITMAP old_bitmap = static_cast<HBITMAP>(SelectObject(mdc, hbitmap1));

			RECT rect;
			GetClientRect(hwnd, &rect);

			SetStretchBltMode(hdc, STRETCH_HALFTONE);
			StretchBlt(hdc, config::main_window::edit_x, config::main_window::edit_y,
				rect.right - config::main_window::edit_delw, rect.bottom - config::main_window::label_h - config::main_window::edit_delh,
				mdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

			SelectObject(mdc, old_bitmap);
			DeleteDC(mdc);
		}
		EndPaint(hwnd, &ps);
	}
	break;

	case MESSAGE_GET_CUR_DATA_FROM_HISTORY:
	{
		auto data = reinterpret_cast<std::pair<BaseData*, DataType>*>(lparam);

		// Открываем буфер обмена
		if (OpenClipboard(nullptr))
		{
			// Очищаем текущие данные в буфере обмена
			EmptyClipboard();

			if (data->second == DataType::TEXT)
			{
				HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, (static_cast<TextData*>(data->first)->get_data().size() + 1) * sizeof(WCHAR));
				if (hglobal != nullptr)
				{
					LPWSTR pglobal = static_cast<LPWSTR>(GlobalLock(hglobal));
					if (pglobal != nullptr)
					{
						wcscpy_s(pglobal, static_cast<TextData*>(data->first)->get_data().size() + 1, static_cast<TextData*>(data->first)->get_data().c_str());
						GlobalUnlock(hglobal);

						SetClipboardData(CF_UNICODETEXT, hglobal);
						// GlobalFree вызывать не нужно, тк буфер обмена сам его вызовет, когда мы сменит данные
						// https://learn.microsoft.com/en-us/answers/questions/607541/(clipboard)-do-i-need-to-call-globalfree-when-usin
					}
					else
						GlobalUnlock(hglobal);
				}
			}

			else if (data->second == DataType::FILE_PATH)
			{
				auto files_data = static_cast<TextData*>(data->first)->get_data();
				std::vector<std::wstring> file_paths;
				std::vector<int16_t> unavaliable_files;
				std::wistringstream file_paths_stream(files_data);
				std::wstring file_path;

				// Разбиваем строку на подстроки по строке L"\r\n"
				while (std::getline(file_paths_stream, file_path, L'\n'))
				{
					if (!file_path.empty() && file_path.back() == L'\r')
					{
						file_path.pop_back(); // Удаляем символ L'\r'
					}
					file_paths.push_back(file_path);
				}

				file_path.clear();
				int16_t count{ 0 };
				for (auto& path : file_paths)
				{
					// Проверяем, не удален ли файл
					DWORD file_avaliable = GetFileAttributes(path.c_str());
					if (file_avaliable == INVALID_FILE_ATTRIBUTES)
					{
						unavaliable_files.push_back(count);
						
						if (unavaliable_files.size() > 1)
							file_path += L"\r\n";
						file_path += path;
					}
					++count;
				}

				if (unavaliable_files.size())
				{
					file_path = L"Список недоступных файлов:\r\n" + file_path;
					// Выводим все недостуные файлы и удаляем их из общего списка
					MessageBox(hwnd, file_path.c_str(), L"Ошибка", MB_OK | MB_ICONERROR);

					// Если все файлы недоступны, то выходим
					if (unavaliable_files.size() == file_paths.size())
						break;
					
					for (int16_t i = 0; i < unavaliable_files.size(); ++i)
						file_paths.erase(file_paths.begin() + unavaliable_files[i] - i);
				}

				// https://stackoverflow.com/questions/25708895/how-to-copy-files-by-win32-api-functions-and-paste-by-ctrlv-in-my-
				// Считаем размер всех файлов для выделения памяти
				int32_t size = sizeof(DROPFILES);
				for (auto& path : file_paths)
					size += (path.size() + 1) * sizeof(WCHAR);  // + 1 => '\0'
				size += sizeof(WCHAR); // два \0 нужно для корректной работы

				// https://devblogs.microsoft.com/oldnewthing/20130520-00/?p=4313
				HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, size);
				if (hglobal != nullptr)
				{
					DROPFILES* pglobal = static_cast<DROPFILES*>(GlobalLock(hglobal));
					if (pglobal != nullptr)
					{
						ZeroMemory(pglobal, size);
						pglobal->pFiles = sizeof(DROPFILES);
						pglobal->fWide = TRUE;

						// Копируем пути в аллоцированную память
						LPWSTR ptr = reinterpret_cast<LPWSTR>(pglobal + 1);
						for (auto& path : file_paths)
						{
							lstrcpyW(ptr, path.c_str());
							ptr = ptr + path.size() + 1; // + 1, чтобы не перезаписать \0
						}
						
						GlobalUnlock(hglobal);
						SetClipboardData(CF_HDROP, hglobal);
					}
					else
						GlobalUnlock(hglobal);
				}
			}

			else if (data->second == DataType::IMAGE)
			{
				//https://stackoverflow.com/a/37503692/15870457
				// Получаем информацию об исходном битмапе
				BITMAP bitmap;
				GetObject(static_cast<ImageData*>(data->first)->get_data(), sizeof(BITMAP), &bitmap);

				// Выделяем устройства для рисования
				HDC hdc = GetDC(NULL);
				HDC srcdc = CreateCompatibleDC(hdc);
				HDC dstdc = CreateCompatibleDC(hdc);

				// Создаем новый объект битмапа
				//hbitmap = CreateBitmapIndirect(&bitmap);
				if (hbitmap2)
					DeleteObject(hbitmap2);
				hbitmap2 = CreateCompatibleBitmap(hdc, bitmap.bmWidth, bitmap.bmHeight);

				HBITMAP srcorig = static_cast<HBITMAP>(SelectObject(srcdc, static_cast<ImageData*>(data->first)->get_data()));
				HBITMAP dstorig = static_cast<HBITMAP>(SelectObject(dstdc, hbitmap2));

				// Копируем содержимое из исходного битмапа в новый
				BitBlt(dstdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, srcdc, 0, 0, SRCCOPY);

				// Возвращаем старые устройства
				SelectObject(srcdc, srcorig);
				SelectObject(dstdc, dstorig);

				// Освобождаем ресурсы
				ReleaseDC(NULL, hdc);
				DeleteDC(srcdc);
				DeleteDC(dstdc);

				SetClipboardData(CF_BITMAP, hbitmap2);
				hbitmap1 = hbitmap2;
			}
		}
		// Закрываем буфер обмена
		CloseClipboard();
		SendMessage(hwnd, WM_CLIPBOARDUPDATE, static_cast<WPARAM>(1), NULL);

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

	case WM_SIZE:
	{
		// Получаем новые размеры окна
		int16_t new_width = LOWORD(lparam);
		int16_t new_height = HIWORD(lparam);

		// Изменяем размеры и положение Static
		SetWindowPos(_win_instance->get_hwnd_label(), nullptr,
			config::main_window::label_x, config::main_window::label_y, new_width, config::main_window::label_h, SWP_NOZORDER);

		// Изменяем положение Edit
		SetWindowPos(_win_instance->get_hwnd_edit(), nullptr,
			config::main_window::edit_x, config::main_window::edit_y,
			new_width - config::main_window::edit_delw, new_height - config::main_window::label_h - config::main_window::edit_delh, SWP_NOZORDER);

		break;
	}

	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO max_size = reinterpret_cast<LPMINMAXINFO>(lparam);
		max_size->ptMinTrackSize.x = config::main_window::main_window_width;
		max_size->ptMinTrackSize.y = config::main_window::main_window_height;
		break;
	}

	case WM_COMMAND:
	{
		switch (wparam)
		{
		case MESSAGE_MENU_HISTORY:
			ShowWindow(_win_instance->_child, SW_SHOWNORMAL);
			break;

		case MESSAGE_MENU_ABOUT:
		{
			std::wstring about_text = std::wstring(L"Версия программы: ") + config::program_version 
				+ std::wstring(L"\nАвтор: laynholt\n") + std::wstring(L"Дата написания: ") + config::program_date;

			MessageBox(NULL, about_text.c_str(), L"Справка", MB_ICONINFORMATION | MB_OK);
			break;
		}

		case MESSAGE_TRAY_OPEN_HISTORY:
			ShowWindow(_win_instance->_child, SW_SHOWNORMAL);
			break;

		case MESSAGE_TRAY_CLEAR_HISTORY:
			SendMessage(_win_instance->_child, MESSAGE_TRAY_CLEAR_HISTORY, NULL, NULL);
			break;

		case MESSAGE_TRAY_CLOSE_APP:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		default:
			break;
		}
		break;
	}

	case WM_SYSCOMMAND:
		// Обработка системных команд
		switch (LOWORD(wparam))
		{
		case SC_MINIMIZE:
			// Обработка команды сворачивания окна
			ShowWindow(hwnd, SW_HIDE);  // Скрыть окно
			ShowWindow(_win_instance->_child, SW_HIDE);  // Скрыть окно
			return 0;
		}
		break;

	case MESSAGE_MENU_CREATE:
		SetMenu(hwnd, _win_instance->_hmenu);
		break;

	case MESSAGE_TRAY_CALLBACK:
		// Обработка событий из иконки в трее
		switch (LOWORD(lparam)) 
		{
		case WM_RBUTTONDOWN:
			// Отображение контекстного меню при щелчке правой кнопкой мыши на иконке в трее
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hwnd);
			TrackPopupMenu(_win_instance->_hmenu_tray, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
			break;
		case WM_LBUTTONDOWN:
			ShowWindow(hwnd, SW_SHOWNORMAL);  // Разворачивание окна
			break;
		}
		break;

	case WM_DESTROY:
		RemoveClipboardFormatListener(hwnd);
		DeleteObject(hbitmap2);
		DestroyMenu(_win_instance->_hmenu);
		Shell_NotifyIcon(NIM_DELETE, &_win_instance->_notify_data);
		DestroyMenu(_win_instance->_hmenu_tray);

		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}
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
}

std::shared_ptr<winapp::Window> winapp::Window::create(LPCWSTR class_name, LPCWSTR window_name, DWORD style, int32_t x, int32_t y,
    int32_t width, int32_t height, HWND hwnd_parent, HMENU hmenu, HINSTANCE hinstance, LPVOID param)
{
    if (_win_instance.get() == nullptr)
    {
        _win_instance = std::shared_ptr<Window>(new Window(class_name, window_name,
            x, y, width, height, hwnd_parent, hmenu, hinstance, WS_OVERLAPPEDWINDOW, param));
		
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
	static HBITMAP hbitmap{};
	static WCHAR filePath[MAX_PATH]{};

	static bool crutch_against_event_duplication_when_self_recording_to_the_buffer{};

	switch (message)
	{
	case WM_CREATE:	// создание основного окна
	{
		AddClipboardFormatListener(hwnd);
		return 0;
	}
	case WM_CLIPBOARDUPDATE:
	{
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
						UINT numFiles = DragQueryFile(hdrop, 0xFFFFFFFF, nullptr, 0);

						if (numFiles > 0)
						{
							// Получаем путь к первому файлу в списке
							DragQueryFile(hdrop, 0, filePath, MAX_PATH);

							_win_instance->set_label_text(L"В буфере обмена сейчас файл:");
							ShowWindow(_win_instance->get_hwnd_edit(), SW_SHOW);
							_win_instance->set_edit_text(filePath);
						}
						GlobalUnlock(hdata);
						SendMessage(_win_instance->_child, MESSAGE_SEND_FILEPATH, NULL, reinterpret_cast<LPARAM>(filePath));
					}
				}
			}
			else if (is_image)
			{
				// Получаем изображение из буфера обмена
				hbitmap = static_cast<HBITMAP>(GetClipboardData(CF_BITMAP));

				if (hbitmap != nullptr)
				{
					_win_instance->set_label_text(L"В буфере обмена сейчас изображение:");
					ShowWindow(_win_instance->get_hwnd_edit(), SW_HIDE);
					SendMessage(_win_instance->_child, MESSAGE_SEND_IMAGE, NULL, reinterpret_cast<LPARAM>(hbitmap));
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

		if (hbitmap)
		{			
			BITMAP bitmap;
			GetObject(hbitmap, sizeof(BITMAP), &bitmap);
		
			HDC mdc = CreateCompatibleDC(hdc);
			HBITMAP old_bitmap = static_cast<HBITMAP>(SelectObject(mdc, hbitmap));

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

	case MESSAGE_GET_DATA_FROM_HISTORY:
	{
		auto data = reinterpret_cast<std::pair<BaseData*, DataType>*>(lparam);

		// Открываем буфер обмена
		if (OpenClipboard(nullptr))
		{
			// Очищаем текущие данные в буфере обмена
			EmptyClipboard();

			if (data->second == DataType::TEXT)
			{
				HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, static_cast<TextData*>(data->first)->get_data().size() * sizeof(WCHAR));
				if (hglobal != nullptr)
				{
					LPWSTR pglobal = static_cast<LPWSTR>(GlobalLock(hglobal));
					if (pglobal != nullptr)
					{
						wcscpy_s(pglobal, static_cast<TextData*>(data->first)->get_data().size() + 1, static_cast<TextData*>(data->first)->get_data().c_str());
						GlobalUnlock(hglobal);

						SetClipboardData(CF_UNICODETEXT, hglobal);
					}
					else
						GlobalUnlock(hglobal);
				}
			}

			else if (data->second == DataType::FILE_PATH)
			{
				// https://stackoverflow.com/questions/25708895/how-to-copy-files-by-win32-api-functions-and-paste-by-ctrlv-in-my-desktop
				// https://devblogs.microsoft.com/oldnewthing/20130520-00/?p=4313
				int32_t size = sizeof(DROPFILES) + (static_cast<TextData*>(data->first)->get_data().size() + 2) * sizeof(WCHAR);
				HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, size);
				if (hglobal != nullptr)
				{
					DROPFILES* pglobal = static_cast<DROPFILES*>(GlobalLock(hglobal));
					if (pglobal != nullptr)
					{
						ZeroMemory(pglobal, size);
						pglobal->pFiles = sizeof(DROPFILES);
						pglobal->fWide = TRUE;
						LPWSTR ptr = reinterpret_cast<LPWSTR>(pglobal + 1);
						lstrcpyW(ptr, static_cast<TextData*>(data->first)->get_data().c_str());

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
				DeleteObject(hbitmap);
				hbitmap = CreateCompatibleBitmap(hdc, bitmap.bmWidth, bitmap.bmHeight);

				HBITMAP srcorig = static_cast<HBITMAP>(SelectObject(srcdc, static_cast<ImageData*>(data->first)->get_data()));
				HBITMAP dstorig = static_cast<HBITMAP>(SelectObject(dstdc, hbitmap));

				// Копируем содержимое из исходного битмапа в новый
				BitBlt(dstdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, srcdc, 0, 0, SRCCOPY);

				// Возвращаем старые устройства
				SelectObject(srcdc, srcorig);
				SelectObject(dstdc, dstorig);

				// Освобождаем ресурсы
				ReleaseDC(NULL, hdc);
				DeleteDC(srcdc);
				DeleteDC(dstdc);

				SetClipboardData(CF_BITMAP, hbitmap);
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

	case MESSAGE_MENU_CREATE:
		SetMenu(hwnd, _win_instance->_hmenu);
		break;

	case WM_COMMAND:
	{
		switch (wparam)
		{
		case MESSAGE_MENU_HISTORY:
			ShowWindow(_win_instance->_child, SW_NORMAL);
			break;

		default:
			break;
		}
		break;
	}

	case WM_DESTROY:
		RemoveClipboardFormatListener(hwnd);
		DeleteObject(hbitmap);
		DestroyMenu(_win_instance->_hmenu);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}
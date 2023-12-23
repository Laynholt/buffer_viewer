#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <cstdint>

namespace winapp
{
	enum class DataType : int16_t
	{
		NONE = 0,
		TEXT = 1,
		FILE_PATH = 2,
		IMAGE = 3
	};

	class BaseData	{};

	class TextData : public BaseData
	{
	public:
		TextData(std::wstring data) : _data(data) {}
		TextData(LPCWSTR data) : _data(data) {}

		std::wstring& get_data() { return _data; }
		const std::wstring& get_data() const { return _data; }

	private:
		std::wstring _data;
	};

	class ImageData : public BaseData
	{
	public:
		ImageData() {}
		ImageData(const ImageData& img)
		{
			copy(img._data);
		}
		ImageData& operator=(const ImageData& img)
		{
			copy(img._data);
			return *this;
		}
		
		~ImageData()
		{
			if (_data != nullptr)
				DeleteObject(_data);
		}

		void copy(HBITMAP data)
		{	
			//https://stackoverflow.com/a/37503692/15870457
			// Получаем информацию об исходном битмапе
			BITMAP bitmap;
			GetObject(data, sizeof(BITMAP), &bitmap);

			// Выделяем устройства для рисования
			HDC hdc = GetDC(NULL);
			HDC srcdc = CreateCompatibleDC(hdc);
			HDC dstdc = CreateCompatibleDC(hdc);

			// Создаем новый объект битмапа
			//_data = CreateBitmapIndirect(&bitmap);
			if (_data != nullptr)
				DeleteObject(_data);
			_data = CreateCompatibleBitmap(hdc, bitmap.bmWidth, bitmap.bmHeight);

			HBITMAP srcorig = static_cast<HBITMAP>(SelectObject(srcdc, data));
			HBITMAP dstorig = static_cast<HBITMAP>(SelectObject(dstdc, _data));

			// Копируем содержимое из исходного битмапа в новый
			BitBlt(dstdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, srcdc, 0, 0, SRCCOPY);

			// Возвращаем старые устройства
			SelectObject(srcdc, srcorig);
			SelectObject(dstdc, dstorig);

			// Освобождаем ресурсы
			ReleaseDC(NULL, hdc);
			DeleteDC(srcdc);
			DeleteDC(dstdc);
		}

		HBITMAP get_data() { return _data; }
		const HBITMAP get_data() const { return _data; }

	private:
		HBITMAP _data{ nullptr };
	};

	class HistoryBuffer
	{

	public:
		void add_text_data(LPWSTR data);
		void add_filepath_data(LPWSTR data);
		void add_image_data(HBITMAP data);

		void clear();

		std::pair<BaseData*, DataType> get_object(int16_t index);
		
		int32_t get_size() { return _buffer_ind.size(); }

	private:
		void _add_data(void* data, DataType type);

	private:
		std::vector<std::pair<int16_t, DataType>> _buffer_ind;
		std::vector<TextData> _text_data;
		std::vector<ImageData> _image_data;
	};
}

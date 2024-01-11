#include "HistoryBuffer.h"

void winapp::HistoryBuffer::add_text_data(LPWSTR data)
{
	_add_data(data, DataType::TEXT);
}

void winapp::HistoryBuffer::add_filepath_data(LPWSTR data)
{
	_add_data(data, DataType::FILE_PATH);
}

void winapp::HistoryBuffer::add_image_data(HBITMAP data)
{
	_add_data(data, DataType::IMAGE);
}

void winapp::HistoryBuffer::pop_first()
{
	if (!_buffer_ind.size())
		return;

	auto object_type = _buffer_ind.begin()->second;
	if (object_type == DataType::TEXT || object_type == DataType::FILE_PATH)
	{
		_text_data.erase(_text_data.begin());
	}
	else if (object_type == DataType::IMAGE)
	{
		_image_data.erase(_image_data.begin());
	}
	_buffer_ind.erase(_buffer_ind.begin());

	for (auto& data_ind : _buffer_ind)
	{
		if (object_type == DataType::TEXT || object_type == DataType::FILE_PATH)
		{
			if (data_ind.second == DataType::TEXT || data_ind.second == DataType::FILE_PATH)
			{
				data_ind.first -= 1;
			}
		}
		else if (object_type == DataType::IMAGE)
		{
			if (data_ind.second == DataType::IMAGE)
			{
				data_ind.first -= 1;
			}
		}
	}
}

void winapp::HistoryBuffer::clear()
{
	_buffer_ind.clear();
	_text_data.clear();
	_image_data.clear();

	_buffer_ind.shrink_to_fit();
	_text_data.shrink_to_fit();
	_image_data.shrink_to_fit();
}

std::pair<winapp::BaseData*, winapp::DataType> winapp::HistoryBuffer::get_object(int16_t index)
{
	if (index < 0 || index >= _buffer_ind.size())
		return std::pair<BaseData*, DataType>(nullptr, DataType::NONE);
	
	if (_buffer_ind[index].second == DataType::TEXT || _buffer_ind[index].second == DataType::FILE_PATH)
		return std::pair<BaseData*, DataType>(&_text_data[_buffer_ind[index].first], _buffer_ind[index].second);

	if (_buffer_ind[index].second == DataType::IMAGE)
		return std::pair<BaseData*, DataType>(&_image_data[_buffer_ind[index].first], _buffer_ind[index].second);
}

void winapp::HistoryBuffer::_add_data(void* data, DataType type)
{
	if (type == DataType::TEXT || type == DataType::FILE_PATH)
	{
		_text_data.push_back(reinterpret_cast<LPCWSTR>(data));
		_buffer_ind.push_back(std::make_pair(_text_data.size() - 1, type));
	}
	else if (type == DataType::IMAGE)
	{
		_image_data.push_back(ImageData());
		_image_data.back().copy(reinterpret_cast<HBITMAP>(data));
		_buffer_ind.push_back(std::make_pair(_image_data.size() - 1, type));
	}
}

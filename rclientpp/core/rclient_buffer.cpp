#include "rclient_buffer.h"
#include <stdlib.h>
#include <algorithm>
#include <string.h>

RClientBuffer::RClientBuffer(size_t init_size)
	:_size(init_size),
	_read_pos(0),
	_write_pos(0)
{
	_buffer = (char*)malloc(init_size);
}

char* RClientBuffer::enable_size(size_t size)
{
	if (size > _size)
	{
		return NULL;
	}
	if (usable_size() < size)
	{
		return NULL;
	}
	if (writable_size() < size)
	{
		rearrange();
	}
	return write_ptr();
}

char* RClientBuffer::write_ptr()
{
	return _buffer + _write_pos;
}

char* RClientBuffer::read_ptr()
{
	return _buffer + _read_pos;
}

size_t RClientBuffer::readable_size()
{
	return _write_pos - _read_pos;
}

size_t RClientBuffer::usable_size()
{
	size_t use = _write_pos - _read_pos;
	if (_size >= use)
	{
		return _size - use;
	}
	return 0;
}

size_t RClientBuffer::writable_size()
{
	return _size - _write_pos;
}

void RClientBuffer::rearrange()
{
	if (readable_size() > 0)
	{
		std::copy(_buffer + _read_pos, _buffer + _write_pos, _buffer);
		_write_pos = _write_pos - _read_pos;
		_read_pos = 0;
	}
	else
	{
		_read_pos = 0;
		_write_pos = 0;
	}
}

void RClientBuffer::has_written(size_t size)
{
	if (_write_pos + size <= _size)
	{
		_write_pos += size;
	}
}

size_t RClientBuffer::get_read_off()
{
	return _read_pos;
}

size_t RClientBuffer::get_write_off()
{
	return _write_pos;
}

void RClientBuffer::set_read_off(size_t pos)
{
	_read_pos = pos;
}

void RClientBuffer::set_write_off(size_t pos)
{
	_write_pos = pos;
}

void RClientBuffer::has_read(size_t size)
{
	if (_read_pos + size <= _write_pos)
	{
		_read_pos += size;
	}
}
//
//size_t RClientBuffer::get_step()
//{
//	return _step_pos;
//}
//
//void RClientBuffer::set_step(size_t pos)
//{
//	_step_pos = pos;
//}

void RClientBuffer::reset()
{
	//memset(_buffer, 0x0, _size);
	_read_pos = 0;
	_write_pos = 0;
	//_step_pos = 0;
}

size_t RClientBuffer::size()
{
	return _size;
}

void RClientBuffer::resize(size_t newcap)
{
	if (newcap <= _size)
	{
		return;
	}
	char* new_buff = (char*)malloc(newcap);
	std::copy(_buffer, _buffer + _size, new_buff);
	_size = newcap;
	free(_buffer);
	_buffer = new_buff;
}

size_t RClientBuffer::write(const char* buf, size_t len)
{
	if (writable_size() >= len)
	{
		std::copy(buf, buf + len, write_ptr());
		has_written(len);
		return len;
	}
	else
	{
		enable_size(len);
		if (writable_size() >= len)
		{
			std::copy(buf, buf + len, write_ptr());
			has_written(len);
			return len;
		}
	}
	return 0;
}

size_t RClientBuffer::read(char* buf, size_t len)
{
	if (len <= readable_size())
	{
		std::copy(read_ptr(), read_ptr() + len, buf);
		return len;
	}
	return 0;
}

RClientBuffer::~RClientBuffer()
{
	if (_buffer)
	{
		free(_buffer);
		_buffer = NULL;
	}
}
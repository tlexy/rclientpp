#include "rclient_buffer.h"
#include <stdlib.h>
#include <algorithm>
#include <string.h>

RClientBuffer::RClientBuffer(const size_t init_size)
	:
    _buffer(static_cast<char*>(malloc(init_size))), _size(init_size)
{
}

char* RClientBuffer::enable_size(const size_t size)
{
	if (size > _size)
	{
		return nullptr;
	}
	if (usable_size() < size)
	{
		return nullptr;
	}
	if (writable_size() < size)
	{
		rearrange();
	}
	return write_ptr();
}

char* RClientBuffer::write_ptr() const
{
	return _buffer + _write_pos;
}

char* RClientBuffer::read_ptr() const
{
	return _buffer + _read_pos;
}

size_t RClientBuffer::readable_size() const
{
	return _write_pos - _read_pos;
}

size_t RClientBuffer::usable_size() const
{
	const size_t use = _write_pos - _read_pos;
	if (_size >= use)
	{
		return _size - use;
	}
	return 0;
}

size_t RClientBuffer::writable_size() const
{
	return _size - _write_pos;
}

void RClientBuffer::rearrange()
{
	std::copy(_buffer + _read_pos, _buffer + _write_pos, _buffer);
	_write_pos = _write_pos - _read_pos;
	_read_pos = 0;
}

void RClientBuffer::has_written(const size_t size)
{
	if (_write_pos + size <= _size)
	{
		_write_pos += size;
	}
}

size_t RClientBuffer::get_read_off() const
{
	return _read_pos;
}

size_t RClientBuffer::get_write_off() const
{
	return _write_pos;
}

void RClientBuffer::set_read_off(const size_t pos)
{
	_read_pos = pos;
}

void RClientBuffer::set_write_off(const size_t pos)
{
	_write_pos = pos;
}

void RClientBuffer::has_read(const size_t size)
{
	if (_read_pos + size <= _write_pos)
	{
		_read_pos += size;
	}
}

size_t RClientBuffer::get_step() const
{
	return _step_pos;
}

void RClientBuffer::set_step(const size_t pos)
{
	_step_pos = pos;
}

void RClientBuffer::reset()
{
	memset(_buffer, 0x0, _size);
	_read_pos = 0;
	_write_pos = 0;
	_step_pos = 0;
}

size_t RClientBuffer::size() const
{
	return _size;
}

void RClientBuffer::resize(const size_t newcap)
{
	if (newcap <= _size)
	{
		return;
	}
    const auto new_buff = static_cast<char*>(malloc(newcap));
	std::copy_n(_buffer, _size, new_buff);
	_size = newcap;
	free(_buffer);
	_buffer = new_buff;
}

size_t RClientBuffer::write(const char* buf, const size_t len)
{
	if (writable_size() >= len)
	{
		std::copy_n(buf, len, write_ptr());
		has_written(len);
		return len;
	}

    enable_size(len);
    if (writable_size() >= len)
    {
        std::copy_n(buf, len, write_ptr());
        has_written(len);
        return len;
    }
    return 0;
}

size_t RClientBuffer::read(char* buf, const size_t len) const
{
	if (len <= readable_size())
	{
		std::copy_n(read_ptr(), len, buf);
		return len;
	}
	return 0;
}

RClientBuffer::~RClientBuffer()
{
	if (_buffer)
	{
		free(_buffer);
		_buffer = nullptr;
	}
}
#include "ptr_buffer.h"
#include <stdlib.h>

namespace rcpp {

	PtrBuffer::PtrBuffer(const char* ptr, const size_t len)
		:_buf(const_cast<char*>(ptr)),
		_len(len)
	{}

	PtrBuffer::PtrBuffer()
		: _buf(nullptr),
		_len(0)
	{}

	PtrBuffer::~PtrBuffer()
	{
		if (_buf)
		{
			free(_buf);
		}
		_buf = nullptr;
	}

}
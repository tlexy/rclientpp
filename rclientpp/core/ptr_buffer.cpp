#include "ptr_buffer.h"
#include <stdlib.h>

namespace rcpp {

	PtrBuffer::PtrBuffer(const char* ptr, size_t len)
		:_buf((char*)ptr),
		_len(len)
	{}

	PtrBuffer::PtrBuffer()
		: _buf(NULL),
		_len(0)
	{}

	PtrBuffer::~PtrBuffer()
	{
		if (_buf)
		{
			free(_buf);
		}
		_buf = NULL;
	}

}
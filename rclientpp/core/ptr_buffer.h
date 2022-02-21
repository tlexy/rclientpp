
#ifndef RCPP_PTRBUFFER_H
#define RCPP_PTRBUFFER_H

#include <string.h>

namespace rcpp {

	class PtrBuffer
	{
	public:
		PtrBuffer();
		PtrBuffer(const char*, size_t);
		PtrBuffer(const PtrBuffer&) = delete;
		PtrBuffer(PtrBuffer&&) = delete;
		PtrBuffer& operator=(const PtrBuffer&) = delete;
		PtrBuffer& operator=(PtrBuffer&&) = delete;
		~PtrBuffer();

		size_t size() const { return _len; }

		const char* data() const { return _buf; }
		char* mdata() const { return _buf; }

	private:
		char* _buf;
		size_t _len;
	};

}

#endif	// 

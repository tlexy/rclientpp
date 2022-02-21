
#ifndef RCPP_PTRBUFFER_H
#define RCPP_PTRBUFFER_H

#include <string.h>

namespace rcpp {

	class PtrBuffer
	{
	public:
		PtrBuffer();
		PtrBuffer(const char*, size_t);

		~PtrBuffer();

		size_t size() { return _len; }

		const char* data() { return _buf; }
		char* mdata() { return _buf; }

	private:
		char* _buf;
		size_t _len;
	};

}

#endif	// 

#ifndef ASYNC_SOCKET_CLIENT
#define ASYNC_SOCKET_CLIENT

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.H>   
#else 
#include <sys/select.h>
#endif

namespace rcpp {

	class AsyncSocketClient
	{
	public:
		AsyncSocketClient();
        explicit AsyncSocketClient(int sockfd);

		int sockfd() const;

		bool connect(const char* ip_str, int port, int timeout_ms);

		size_t read(char* buf, size_t len, int timeout_ms);
		size_t write(const char* buf, size_t len, int timeout_ms);

		size_t read(char* buf, size_t len) const;
		size_t write(const char* buf, size_t len) const;

		void close();

	private:
		int _sockfd{ -1 };
		fd_set _rfds;
		fd_set _wfds;
		//fd_set _efds;//error fds
		struct timeval _tv;

		int _err_code;
	};

}

#endif
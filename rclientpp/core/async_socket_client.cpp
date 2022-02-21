#include "async_socket_client.h"
#include "sock_utils.h"
#include <string.h>

namespace rcpp {

	AsyncSocketClient::AsyncSocketClient()
	{
		FD_ZERO(&_rfds);
		FD_ZERO(&_wfds);
		//FD_ZERO(&_efds);
	}

	AsyncSocketClient::AsyncSocketClient(const int sockfd)
	{}

	int AsyncSocketClient::sockfd() const
	{
		return _sockfd;
	}

	bool AsyncSocketClient::connect(const char* ip_str, const int port, const int timeout_ms)
	{
        sockaddr_in server_address;
		const int confd = sockets::Socket(AF_INET, SOCK_STREAM, 0);
		if (confd < 0)
		{
			_err_code = confd;
			return false;
		}

		_sockfd = confd;
		//2.初始化服务器地址
		memset(&server_address, 0, sizeof(server_address));
		server_address.sin_family = AF_INET;
		inet_pton(AF_INET, ip_str, &server_address.sin_addr.s_addr);
		server_address.sin_port = htons(static_cast<unsigned short>(port));

		int ret = sockets::SetBlocking(_sockfd, false);
		if (ret < 0)
		{
			_err_code = ret;
			return false;
		}
		ret = sockets::setNoDelay(_sockfd);
		if (ret != 0)
		{
			_err_code = ret;
			return false;
		}
		ret = sockets::Connect(_sockfd, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address));
		if (ret == 0)
		{
			_err_code = ret;
			return true;
		}
#ifndef _WIN32
		if (errno != EINPROGRESS)
		{
			_err_code = ret;
			return false;
		}
#else 
		ret = WSAGetLastError();
		if (ret != WSAEWOULDBLOCK)
		{
			_err_code = ret;
			return false;
		}
#endif

		FD_SET(_sockfd, &_wfds);
        const timeval tv{ timeout_ms / 1000 , (timeout_ms % 1000) * 1000 };
		ret = select(_sockfd + 1, nullptr, &_wfds, nullptr, &tv);
		if (ret == 0)
		{
			_err_code = ret;
			printf("connect, select socket select timeout\n");
			return false;
		}


		socklen_t length = sizeof(_err_code);
		//获取任意类型、任意状态套接口的选项当前值
		if (getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&_err_code), &length) < 0)
		{
			printf("get socket option failed\n");
			return false;
		}
		if (_err_code != 0)
		{
			printf("connection failed after select with the error:[%s] errno %d \n", strerror(_err_code), _err_code);
			return false;
		}

		if (ret > 0)
		{
			if (FD_ISSET(_sockfd, &_wfds))
			{
				printf("connect successfully...\n");
				return true; //connect successfully...
			}
		}
#ifdef _WIN32
		_err_code = WSAGetLastError();
		return false;
#else 
		return false;
#endif
	}

	size_t AsyncSocketClient::write(const char* buf, const size_t len, const int timeout_ms)
	{
		auto written_len = write(buf, len);
		if (written_len == len)
		{
			return len;
		}
		while (written_len < len)
		{
			FD_ZERO(&_wfds);
			FD_ZERO(&_rfds);

			FD_SET(_sockfd, &_wfds);
			_tv.tv_sec = timeout_ms / 1000;
			_tv.tv_usec = (timeout_ms % 1000) * 1000;
			const int ret = select(_sockfd + 1, nullptr, &_wfds, nullptr, &_tv);
			if (ret == 0)
			{
				_err_code = ret;
				printf("read, select socket select timeout\n");
				return written_len;
			}
			if (FD_ISSET(_sockfd, &_wfds))
			{
				written_len += write(buf + written_len, len - written_len);
			}
		}
		return written_len;
	}

	size_t AsyncSocketClient::read(char* buf, const size_t len, const int timeout_ms)
	{
		const auto read_len = read(buf, len);
		if (read_len > 0)
		{
			return read_len;
		}
		FD_ZERO(&_rfds);
		FD_ZERO(&_wfds);

		FD_SET(_sockfd, &_rfds);

		_tv.tv_sec = timeout_ms / 1000;
		_tv.tv_usec = (timeout_ms % 1000) * 1000;
		const int ret = select(_sockfd + 1, &_rfds, nullptr, nullptr, &_tv);
		if (ret == 0)
		{
			_err_code = ret;
			printf("read, select socket select timeout\n");
			return 0;
		}
		if (FD_ISSET(_sockfd, &_rfds))
		{
			return read(buf, len);
		}

		return 0;
	}

	size_t AsyncSocketClient::read(char* buf, const size_t len) const
    {
		return sockets::Read(_sockfd, buf, len);
	}

	size_t AsyncSocketClient::write(const char* buf, const size_t len) const
    {
		return sockets::Write(_sockfd, buf, len);
	}

	void AsyncSocketClient::close()
	{
		if (_sockfd > 0)
		{
			sockets::Close(_sockfd);
		}
		_sockfd = -1;
	}

}
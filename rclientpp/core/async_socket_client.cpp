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

	/*AsyncSocketClient::AsyncSocketClient(int sockfd)
	{}*/

	AsyncSocketClient::~AsyncSocketClient()
	{
		close();
	}

	int AsyncSocketClient::sockfd() const
	{
		return _sockfd;
	}

	bool AsyncSocketClient::connect(const char* ip_str, int port, int timeoutms)
	{
		struct sockaddr_in serveraddr;
		int confd = sockets::Socket(AF_INET, SOCK_STREAM, 0);
		if (confd < 0)
		{
			_err_code = confd;
			return false;
		}

		_sockfd = confd;
		//2.初始化服务器地址
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		inet_pton(AF_INET, ip_str, &serveraddr.sin_addr.s_addr);
		serveraddr.sin_port = htons(port);

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
		ret = sockets::Connect(_sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
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
		struct timeval tv;
		tv.tv_sec = timeoutms / 1000;
		tv.tv_usec = (timeoutms % 1000) * 1000;
		ret = select(_sockfd + 1, NULL, &_wfds, NULL, &tv);
		if (ret == 0)
		{
			_err_code = ret;
			printf("connect, select socket select timeout\n");
			return false;
		}


		socklen_t length = sizeof(_err_code);
		//获取任意类型、任意状态套接口的选项当前值
		if (getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, (char*)&_err_code, &length) < 0)
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

	int AsyncSocketClient::write(const char* buf, int len, int timeoutms)
	{
		int written_len = write(buf, len);
		if (written_len == len)
		{
			return len;
		}
		while (written_len < len)
		{
			FD_ZERO(&_wfds);
			FD_ZERO(&_rfds);
			//FD_ZERO(&_efds);

			FD_SET(_sockfd, &_wfds);
			_tv.tv_sec = timeoutms / 1000;
			_tv.tv_usec = (timeoutms % 1000) * 1000;
			int ret = select(_sockfd + 1, NULL, &_wfds, NULL, &_tv);
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

	int AsyncSocketClient::read(char* buf, int len, int timeoutms)
	{
		int readlen = read(buf, len);
		if (readlen > 0)
		{
			return readlen;
		}
		FD_ZERO(&_rfds);
		FD_ZERO(&_wfds);
		//FD_ZERO(&_efds);

		FD_SET(_sockfd, &_rfds);

		_tv.tv_sec = timeoutms / 1000;
		_tv.tv_usec = (timeoutms % 1000) * 1000;
		int ret = select(_sockfd + 1, &_rfds, NULL, NULL, &_tv);
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

	int AsyncSocketClient::read(char* buf, int len)
	{
		return sockets::Read(_sockfd, buf, len);
	}

	int AsyncSocketClient::write(const char* buf, int len)
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
#include "sock_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if !defined(_WIN32)
	#include <netinet/tcp.h>
	#include <fcntl.h>
#define SOCKET_ERROR -1
#endif

namespace sockets
{

	//void perr_exit(const char *s)
	//{

	//}

	void Init()
	{
#if defined(_WIN32)
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
	}

	void Destroy()
	{
#if defined(_WIN32)
		WSACleanup();
#endif
	}

	void err_exit(const char *s)
	{
		printf("%s\n", s);
		//exit(1);
	}

	uint32_t ip2a(const char* ip)
	{
		return inet_addr(ip);
	}

	int Connect(int fd, const struct sockaddr *sa, socklen_t salen)
	{
		int ret = 0;
		if ((ret = connect(fd, sa, salen)) < 0)
		{
			//err_exit("connect error");
			return ret;
		}
		return 0;
	}

	int ConnectTcp(const char* ip_str, int port, int timeoutms)
	{
		struct sockaddr_in serveraddr;
		int confd = Socket(AF_INET, SOCK_STREAM, 0);
		if (timeoutms > 0 && confd > 0)
		{
			SetSendTimeout(confd, timeoutms);
			memset(&serveraddr, 0, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			inet_pton(AF_INET, ip_str, &serveraddr.sin_addr.s_addr);
			serveraddr.sin_port = htons(port);
			int ret = Connect(confd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
			if (ret != 0)
			{
				return -1;
			}
			return confd;
		}
		return confd;
	}

	int SetRecvTimeout(int fd, int ms)
	{
#ifdef _WIN32
		int TimeOut = ms;
#else
		struct timeval TimeOut = { ms / 1000,1000 * (ms % 1000) };
#endif
		if (::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&TimeOut, sizeof(TimeOut)) == SOCKET_ERROR)
		{
			return -1;
		}
		return 0;
	}

	int SetSendTimeout(int fd, int ms)
	{
#ifdef _WIN32
		int TimeOut = ms;
#else
		struct timeval TimeOut = { ms / 1000,1000 * (ms % 1000) };
#endif
		if (::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&TimeOut, sizeof(TimeOut)) == SOCKET_ERROR)
		{
			return -1;
		}
		return 0;
	}

	int Socket(int family, int type, int protocol)
	{
		int n;
		if ((n = socket(family, type, protocol)) < 0)
			err_exit("socket error");
		return n;
	}

	int Read(int fd, void *ptr, size_t nbytes)
	{
		int n;
	again:
#if !defined(_WIN32)
		if ((n = read(fd, ptr, nbytes)) == -1) {
			if (errno == EINTR)
				goto again;
			else
				return -1;
		}
#else 
		n = recv(fd, (char*)ptr, nbytes, 0);
#endif
		return n;
	}

	int Write(int fd, const void *ptr, size_t nbytes)
	{
		int n;
	again:
#if !defined(_WIN32)
		if ((n = write(fd, ptr, nbytes)) == -1) {
			if (errno == EINTR)
				goto again;
			else
				return -1;
		}
#else 
		n = send(fd, (const char*)ptr, nbytes, 0);
#endif
		return n;
	}

	void Close(int fd)
	{
#if defined(_WIN32)
		closesocket(fd);
#else 
		close(fd);
#endif 
	}

	int KeepAlive(int fd)
	{
		int val = 1;
		int ret = 0;
		ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&val, sizeof(val));
		return ret;
	}

	int SetBlocking(int fd, bool is_blocking)
	{
#ifndef _WIN32
		int flags;

		/* Set the socket nonblocking.
		 * Note that fcntl(2) for F_GETFL and F_SETFL can't be
		 * interrupted by a signal. */
		if ((flags = fcntl(fd, F_GETFL)) == -1) {
			return flags;
		}

		if (is_blocking)
			flags &= ~O_NONBLOCK;
		else
			flags |= O_NONBLOCK;
		int ret = 0;
		if ( (ret = fcntl(fd, F_SETFL, flags)) == -1) {
			return ret;
		}
#else
		u_long mode = is_blocking ? 0 : 1;
		return ioctlsocket(fd, FIONBIO, &mode);
#endif /* _WIN32 */
		return 0;
	}

	int setNoDelay(int fd)
	{
		int yes = 1;
		if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(yes)) == -1) {
			return -1;
		}
		return 0;
	}

	void getIpByDoname(const std::string& doname, std::vector<std::string>& ip_list)
	{
		/*
		int i = 0;
		struct in_addr addr;
		DWORD dwError;
		struct hostent *remoteHost;
		const char* host_name = doname.c_str();
		remoteHost = gethostbyname(host_name);

		char **pAlias;

		if (remoteHost == NULL) 
		{
			dwError = WSAGetLastError();
			if (dwError != 0)
			{
				if (dwError == WSAHOST_NOT_FOUND) 
				{
					printf("Host not found\n");
				}
				else if (dwError == WSANO_DATA) 
				{
					printf("No data record found\n");
				}
				else 
				{
					printf("Function failed with error: %ld\n", dwError);
				}
			}
		}
		else 
		{
			//printf("\tOfficial name: %s\n", remoteHost->h_name);
			for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) 
			{
				//printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
			}
			//printf("\tAddress type: ");
			switch (remoteHost->h_addrtype) 
			{
			case AF_INET:
				//printf("AF_INET\n");
				break;
			//case AF_NETBIOS:
				//printf("AF_NETBIOS\n");
			//	break;
			default:
				//printf(" %d\n", remoteHost->h_addrtype);
				break;
			}
			//printf("\tAddress length: %d\n", remoteHost->h_length);

			i = 0;
			if (remoteHost->h_addrtype == AF_INET)
			{
				while (remoteHost->h_addr_list[i] != 0) 
				{
					addr.s_addr = *(u_long *)remoteHost->h_addr_list[i++];
					char* ip = inet_ntoa(addr);
					//printf("\tIP Address #%d: %s\n", i, ip);
					std::string ip_str = std::string(ip, strlen(ip));
					ip_list.push_back(ip_str);
				}
			}
			//else if (remoteHost->h_addrtype == AF_NETBIOS)
			//{
			//	printf("NETBIOS address was returned\n");
			//}
		}
		*/
	}

	bool isIp(const std::string& str)
	{
		if (str.size() < 7)
		{
			return false;
		}
		std::vector<int> vecs;
		int offset = 0;
		size_t pos = str.find('.', offset);
		int count = 1;
		while (pos != std::string::npos)
		{
			/*if (pos == std::string::npos)
			{
				return false;
			}*/
			++count;
			if (count >= 5)
			{
				return false;
			}
			std::string s_num_1 = str.substr(offset, pos - offset);
			if (!isNum(s_num_1))
			{
				return false;
			}
			int num_1 = std::atoi(s_num_1.c_str());
			vecs.push_back(num_1);

			offset = pos + 1;
			pos = str.find('.', offset);
		}
		//
		if (count != 4)
		{
			return false;
		}
		std::string s_num = str.substr(offset);
		if (!isNum(s_num))
		{
			return false;
		}
		int num = std::atoi(s_num.c_str());
		vecs.push_back(num);
		
		for (int i = 0; i < vecs.size(); ++i)
		{
			if (vecs[i] < 0 || vecs[i] > 255)
			{
				return false;
			}
		}
		return true;
	}

	bool isNum(const std::string& str)
	{
		for (int i = 0; i < str.size(); ++i)
		{
			if (str[i] < '0' || str[i] > '9')
			{
				return false;
			}
		}
		return true;
	}

	uint64_t hostToNetwork64(uint64_t host64)
	{
		uint32_t pre = 0;
		uint32_t suf = 0;
		char* p = (char*)&host64;
		memcpy(&pre, p, sizeof(uint32_t));
		memcpy(&suf, p + sizeof(uint32_t), sizeof(uint32_t));
		uint32_t npre = (uint32_t)htonl(pre);
		uint32_t nsuf = (uint32_t)htonl(suf);
		uint64_t ret = 0;
		p = (char*)&ret;
		memcpy(p, &nsuf, sizeof(nsuf));
		memcpy(p + sizeof(nsuf), &npre, sizeof(npre));
		return ret;
	}
}

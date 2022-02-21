#ifndef CROSS_TCP_CLIENT_H
#define CROSS_TCP_CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>
#if defined(_WIN32)
#include<ws2tcpip.h>
#include <winsock2.h>
#else
	#if !defined(_WIN32)
	#include <errno.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	#define DWORD long
	#endif
#endif
namespace sockets
{
	void Init();
	void Destroy();
	//void perr_exit(const char *s);
	void err_exit(const char *s);
	int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
	int Socket(int family, int type, int protocol);
	std::size_t Read(int fd, void *ptr, size_t nbytes);
	int Write(int fd, const void *ptr, size_t nbytes);
	void Close(int fd);

	int KeepAlive(int fd);
	int SetBlocking(int fd, bool is_blocking);
	int setNoDelay(int fd);

	int SetRecvTimeout(int fd, int ms);
	int SetSendTimeout(int fd, int ms);

	void getIpByDoname(const std::string&, std::vector<std::string>&);
	bool isIp(const std::string& str);
	bool isNum(const std::string& str);

	uint64_t hostToNetwork64(uint64_t host64);

	uint32_t ip2a(const char* ip);

	///上层API
	int ConnectTcp(const char* ip_str, int port, int timeoutms = 0);

}
#endif
#ifndef RCPP_RCLIENT_H
#define RCPP_RCLIENT_H

#include "rclient_def.h"
#include <string>

#include "../parser/map_parser.h"
#include "../parser/array_parser.h"

NS_1

class AsyncSocketClient;

class RClient
{
public:
	RClient(std::string ipstr, int port);

	int connect(const std::string& username, const std::string& auth, int timeout_ms = 10000);
	int connect(const std::string& auth, int timeout_ms = 10000);
	int connect(int timeout_ms = 10000);

	size_t use_resp3();
    static int use_resp2();

	void close();

	static void set_read_timeout(int millisec);

	//将命令发送出去
	size_t command(const char* cmd, std::size_t len);
	//接收返回
	size_t recv(void* buf, int len) const;

	int get_error(std::string& errstr) const;

	std::shared_ptr<BaseValue> get_results(size_t& ret_code);

	std::string strerror();

private:
	int do_connect(const std::string& cmd);

	void _get_error(std::shared_ptr<BaseValue>) const;

	int _sock_param() const;

private:
	int _sockfd{-1};
	std::shared_ptr<AsyncSocketClient> _aclient;

	std::string _ipstr;
	int _port;
	int _err_code{};
	static int _read_timeout;
	std::string _strerr;
	std::shared_ptr<RClientBuffer> _bufptr;
	std::shared_ptr<MapParser> _map_parser;
	std::shared_ptr<ArrayParser> _array_parser;
};

NS_2

#endif
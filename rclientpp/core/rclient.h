﻿#ifndef RCPP_RCLIENT_H
#define RCPP_RCLIENT_H

#include "rclient_def.h"
#include <string>
#include <atomic>

#include "../parser/map_parser.h"
#include "../parser/array_parser.h"

NS_1

class AsyncSocketClient;

class RClient
{
public:
	RClient(const std::string& ipstr, int port);

	int connect(const std::string& username, const std::string& auth, int timeoutms = 10000);
	int connect(const std::string& auth, int timeoutms = 10000);
	int connect(int timeoutms = 10000);

	int reconnect(int timeoutms = 500);

	int check_connection();
	bool is_connected();

	int use_resp3();
	int use_resp2();

	RedisRoleType get_role();

	void close();

	void set_read_timeout(int millisec);

	//将命令发送出去
	int command(const char* cmd, int len);
	//接收返回
	int recv(void* buf, int len);

	int get_error(std::string& errstr);

	std::shared_ptr<BaseValue> get_results(int& ret_code);

	std::shared_ptr<BaseValue> do_parse(int& ret_code);
	//返回了多个结果，而没有处理完成？
	int has_more_data() const;

	std::string strerror();

	void set_error_str(const std::string&);

private:
	int do_connect(const std::string& cmd);

	void _get_error(std::shared_ptr<BaseValue>);

	int _sock_param();

private:
	int _sockfd{-1};
	std::shared_ptr<AsyncSocketClient> _aclient;

	std::string _ipstr;
	int _port;
	std::string _user;
	std::string _pass;

	std::atomic<bool> _is_connect;

	int _err_code;
	int _read_timeout{30000};
	std::string _strerr;
	std::shared_ptr<RClientBuffer> _bufptr;
	std::shared_ptr<MapParser> _map_parser;
	std::shared_ptr<ArrayParser> _array_parser;
};

NS_2

#endif
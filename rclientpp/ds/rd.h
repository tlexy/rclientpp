#ifndef RCPP_RD_BASE_H
#define RCPP_RD_BASE_H

#include "../core/rclient_def.h"
#include <memory>
#include <string>
#include <stdint.h>

NS_1

class RClient;

class Rd
{
public:
	Rd(std::shared_ptr<RClient> connection);

	void set_connection(std::shared_ptr<RClient> connection);

	//send command and wait the result.
	std::shared_ptr<BaseValue> redis_command(const char* cmd, int len, int& ret_code);

	bool get_boolean(const std::string& cmd);
	//int get_integer(const std::string& cmd);

	void set_err_str(const std::string&);
	std::string get_err_str();

protected:
	std::shared_ptr<RClient> _client;
	std::string _crlf;
};

NS_2

#endif
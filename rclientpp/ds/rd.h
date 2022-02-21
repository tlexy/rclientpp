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
    explicit Rd(std::shared_ptr<RClient> connection);

	void set_connection(const std::shared_ptr<RClient>& connection);

	//send command and wait the result.
	std::shared_ptr<BaseValue> redis_command(const char* cmd, std::size_t len, size_t& ret_code) const;

	bool get_boolean(const std::string& cmd) const;
	//int get_integer(const std::string& cmd);

protected:
	std::shared_ptr<RClient> _client;
	std::string _crlf;
};

NS_2

#endif
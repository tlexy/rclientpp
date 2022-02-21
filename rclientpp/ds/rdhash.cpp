#include "rdhash.h"
#include "../core/rclient.h"

NS_1

RdHash::RdHash(std::shared_ptr<RClient> connection)
	:Rd(connection)
{
}

int RdHash::hset(const std::string& key, const std::string& field, const std::string& val)
{
	std::string cmd = "hset ";
	cmd = cmd + key + " " + field + " " + val + _crlf;
	int ret_code = 0;
	auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (ret_code != 0)
	{
		return 0;
	}
	if (ptr && ptr->value_type() == ParserType::Number)
	{
		return (std::dynamic_pointer_cast<RedisValue>(ptr))->u.int_val_;
	}
	return 0;
}

std::shared_ptr<RedisValue> RdHash::hget(const std::string& key, const std::string& field)
{
	std::string cmd = "hget ";
	cmd = cmd + key + " " + field + _crlf;
	int ret_code = 0;
	auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (ret_code != 0)
	{
		return nullptr;
	}
	return std::dynamic_pointer_cast<RedisValue>(ptr);
}

std::shared_ptr<RedisValue> RdHash::hmget(const std::string& key, std::initializer_list<std::string> fields)
{
	std::string cmd = "hmget ";
	cmd = cmd + key;
	auto it = fields.begin();
	for (; it != fields.end(); ++it)
	{
		cmd = cmd + " " + (*it);
	}
	cmd += _crlf;
	int ret_code = 0;
	auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (ret_code != 0)
	{
		return nullptr;
	}
	return std::dynamic_pointer_cast<RedisValue>(ptr);
}

bool RdHash::hexists(const std::string& key, const std::string& field)
{
	return false;
}

int RdHash::hdel(const std::string& key, const std::string& field)
{
	return 0;
}

int RdHash::hlen(const std::string& key, const std::string& field)
{
	return 0;
}

NS_2
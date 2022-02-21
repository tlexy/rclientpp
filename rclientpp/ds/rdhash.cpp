#include "rdhash.h"
#include "../core/rclient.h"

NS_1

RdHash::RdHash(const std::shared_ptr<RClient>& connection)
	:Rd(connection)
{
}

std::size_t RdHash::hset(const std::string& key, const std::string& field, const std::string& val) const
{
	std::string cmd = "hset ";
	cmd += key + " " + field + " " + val + _crlf;
	size_t ret_code = 0;
	const auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (ret_code != 0)
	{
		return 0;
	}
	if (ptr && ptr->value_type() == ParserType::Number)
	{
		return std::dynamic_pointer_cast<RedisValue>(ptr)->u.int_val_;
	}
	return 0;
}

std::shared_ptr<RedisValue> RdHash::hget(const std::string& key, const std::string& field) const
{
	std::string cmd = "hget ";
	cmd += key + " " + field + _crlf;
	size_t ret_code = 0;
	const auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (ret_code != 0)
	{
		return nullptr;
	}
	return std::dynamic_pointer_cast<RedisValue>(ptr);
}

std::shared_ptr<RedisValue> RdHash::hmget(const std::string& key, std::initializer_list<std::string> fields) const
{
	std::string cmd = "hmget ";
	cmd = cmd + key;
    for (auto it = fields.begin(); it != fields.end(); ++it)
	{
		cmd += " " + (*it);
	}
	cmd += _crlf;
	size_t ret_code = 0;
	const auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
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
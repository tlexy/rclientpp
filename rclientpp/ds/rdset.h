#ifndef RCPP_RD_SET_H
#define RCPP_RD_SET_H

#include "rd.h"
#include <initializer_list>

NS_1

class RdSet : public Rd
{
public:
    explicit RdSet(std::shared_ptr<RClient> connection);
	RdSet(const RdSet&) = delete;
	RdSet(RdSet&&) = delete;
	RdSet& operator=(const RdSet&) = delete;
	RdSet& operator=(RdSet&&) = delete;
	~RdSet() = default;

	template<typename T>
	std::size_t sadd(const std::string& key, std::initializer_list<T> members)
	{
		std::string cmd = "SADD ";
		cmd = cmd + key;
		auto it = members.begin();
		for (; it != members.end(); ++it)
		{
			cmd = cmd + " " + std::to_string(*it);
		}
		cmd += _crlf;
		size_t ret_code = 0;
		const auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
		if (ret_code != 0)
		{
			return 0;
		}
		if (!ptr)
		{
			return 0;
		}
		if (ptr->value_type() == ParserType::Number)
		{
			const auto pptr = std::dynamic_pointer_cast<RedisValue>(ptr);
			return pptr->u.int_val_;
		}
		return 0;
	}

	std::shared_ptr<BaseValue> smembers(const std::string& key) const;

	bool is_member(const std::string& key, const std::string& value) const
    {
		std::string cmd = "SISMEMBER ";
		cmd = cmd + key + " " + value + _crlf;
		return get_boolean(cmd);
	}

	template<typename T>
	bool is_member(const std::string& key, const T& value)
	{
		std::string cmd = "SISMEMBER ";
		cmd = cmd + key + " " + std::to_string(value) + _crlf;
		return get_boolean(cmd);
	}

private:

};

NS_2

#endif
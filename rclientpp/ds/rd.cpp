#include "rd.h"
#include "../core/rclient.h"

NS_1

Rd::Rd(std::shared_ptr<RClient> connection)
	:_client(connection),
	_crlf("\r\n")
{}

void Rd::set_connection(std::shared_ptr<RClient> connection)
{
	_client = connection;
}

void Rd::set_err_str(const std::string& str)
{
	_client->set_error_str(str);
}

std::string Rd::get_err_str()
{
	return _client->strerror();
}

std::shared_ptr<BaseValue> Rd::redis_command(const char* cmd, int len, int& ret_code)
{
	int ret = _client->command(cmd, len);
	if (ret != len)
	{
		return nullptr;
	}
	auto ptr = _client->get_results(ret_code);
	return ptr;
}

bool Rd::get_boolean(const std::string& cmd)
{
	int ret_code = 0;
	auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (!ptr || ret_code != 0)
	{
		//something error...
		return false;
	}
	if (ptr->is_boolean())
	{
		auto pptr = std::dynamic_pointer_cast<RedisValue>(ptr);
		return pptr->u.boolean_val_;
	}
	else if (ptr->value_type() == ParserType::Number)
	{
		auto pptr = std::dynamic_pointer_cast<RedisValue>(ptr);
		return pptr->u.int_val_ == 1;
	}
	//other error
	return false;
}

NS_2
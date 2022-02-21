#include "rd.h"
#include "../core/rclient.h"

NS_1

Rd::Rd(std::shared_ptr<RClient> connection)
	:_client(std::move(connection)),
	_crlf("\r\n")
{}

void Rd::set_connection(const std::shared_ptr<RClient>& connection)
{
	_client = connection;
}

std::shared_ptr<BaseValue> Rd::redis_command(const char* cmd, const std::size_t len, size_t& ret_code) const
{
	const auto ret = _client->command(cmd, len);
	if (ret != len)
	{
		return nullptr;
	}
	auto ptr = _client->get_results(ret_code);
	return ptr;
}

bool Rd::get_boolean(const std::string& cmd) const
{
	size_t ret_code = 0;
	const auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (!ptr || ret_code != 0)
	{
		//something error...
		return false;
	}
	if (ptr->is_boolean())
	{
		const auto pptr = std::dynamic_pointer_cast<RedisValue>(ptr);
		return pptr->u.boolean_val_;
	}
	if (ptr->value_type() == ParserType::Number)
	{
		const auto pptr = std::dynamic_pointer_cast<RedisValue>(ptr);
		return pptr->u.int_val_ == 1;
	}
	//other error
	return false;
}

NS_2
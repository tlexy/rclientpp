#include "rdstring.h"

#include <utility>
#include "../core/rclient.h"

NS_1

RdString::RdString(std::shared_ptr<RClient> connection)
	:Rd(std::move(connection))
{
}

int RdString::set(const std::string& key, const std::string& value,
	uint64_t seconds, uint64_t millisec, const std::string& option) const
{
	std::string cmd = "set ";
	cmd = cmd + key + " " + value;
	if (seconds > 0 && millisec == 0)
	{
		cmd = cmd + " EX " + std::to_string(seconds);
	}
	if (millisec > 0)
	{
		cmd = cmd + " PX " + std::to_string(seconds);
	}
	if (option == std::string("NX") || option == std::string("XX"))
	{
		cmd = cmd + " " + option;
	}
	cmd += _crlf;
	/*int ret = _client->command(cmd.c_str(), cmd.size());
	if (ret != cmd.size())
	{
		return ret;
	}
	auto ptr = _client->get_results();
	*/
	size_t ret_code = 0;
	const auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (ret_code != 0)
	{
		return 0;
	}
	if (ptr && ptr->is_ok())
	{
		return 1;
	}
	return 0;
}

std::shared_ptr<BaseValue> RdString::get(const std::string& key) const
{
	std::string cmd = "get ";
	cmd += key;

	cmd += _crlf;
	size_t ret_code = 0;
	return redis_command(cmd.c_str(), cmd.size(), ret_code);
}

NS_2
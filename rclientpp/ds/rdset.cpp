#include "rdset.h"

#include <utility>

NS_1
    RdSet::RdSet(std::shared_ptr<RClient> connection)
	:Rd(std::move(connection))
{
}

std::shared_ptr<BaseValue> RdSet::smembers(const std::string& key) const
{
	std::string cmd = "SMEMBERS ";
	cmd = cmd + key + _crlf;

	size_t ret_code = 0;
	auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (ret_code != 0)
	{
		return nullptr;
	}
	return ptr;
}

NS_2
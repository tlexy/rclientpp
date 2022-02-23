#include "publisher.h"

NS_1

Publisher::Publisher(std::shared_ptr<RClient> connection)
	:Rd(connection)
{}

int Publisher::publish(const std::string& chl, const std::string& msg)
{
	std::string cmd = "PUBLISH ";
	cmd = cmd + chl + " " + msg + _crlf;
	int ret_code = 0;
	auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	if (ptr)
	{
		if (ptr->is_number())
		{
			return ptr->get_number();
		}
		return RESP_FORMAT_UNKNOWN;
	}
	else
	{
		return ret_code;
	}
}

Publisher::~Publisher()
{}

NS_2
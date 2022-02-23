#include "subscriber.h"
#include "../core/rclient.h"

NS_1

Subscriber::Subscriber(std::shared_ptr<RClient> connection)
	:Rd(connection)
{}

int Subscriber::subscribe(std::initializer_list<std::string> channels)
{
	if (channels.size() == 0)
	{
		return 0;
	}
	std::string cmd = "SUBSCRIBE";
	auto it = channels.begin();
	for (; it != channels.end(); ++it)
	{
		cmd = cmd + " " + (*it);
	}
	cmd += _crlf;
	int ret_code = 0;
	int count = 0;
	auto ptr = redis_command(cmd.c_str(), cmd.size(), ret_code);
	int rest = _client->has_more_data();
	while (ptr && rest > 0)
	{
		ptr = _client->do_parse(ret_code);
		if (ptr)
		{
			rest = _client->has_more_data();
		}
	}
	if (ptr)
	{
		if (ptr->value_type() == ParserType::Array || ptr->value_type() == ParserType::Push)
		{
			auto pptr = std::dynamic_pointer_cast<RedisComplexValue>(ptr);
			if (pptr->results.size() == 3)
			{
				auto it = pptr->results.begin(); 
				++it;
				++it;
				if ((*it)->is_number())
				{
					count = (*it)->get_number();
				}
			}
		}
	}
	return count;
}

int Subscriber::recv_msg(SubsHandleFunc func)
{
	_func = func;

	auto channel = std::make_shared<std::string>();
	auto message = std::make_shared<std::string>();
	_is_stop = false;
	_client->set_read_timeout(1000);
	int ret_code = 0;
	int count = 0;
	int rest = 0;
	while ((ret_code == 0 || ret_code == TCP_TIMEOUT)
		&& !_is_stop)
	{
		auto ptr = _client->get_results(ret_code);
		while (ptr)
		{
			count = handle_publish(ptr, channel, message);
			if (count == 0)
			{
				break;
			}
			else
			{
				if (_func)
				{
					_func(channel, message);
				}
			}
			rest = _client->has_more_data();
			if (rest > 0)
			{
				ptr = _client->do_parse(ret_code);
			}
			else
			{
				break;
			}
			
		}
		//
		if (ret_code != 0 && ret_code != TCP_TIMEOUT)
		{
			printf("subscriber recv msg error: %d\n", ret_code);
			break;
		}
	}
	return 0;
}

int Subscriber::handle_publish(std::shared_ptr<BaseValue>ptr, std::shared_ptr<std::string> channel, std::shared_ptr<std::string> message)
{
	if (ptr->value_type() != ParserType::Array && ptr->value_type() != ParserType::Push)
	{
		printf("subscriber get wrong type: %d\n", ptr->value_type());
		return 0;
	}
	auto pptr = std::dynamic_pointer_cast<RedisComplexValue>(ptr);
	if (pptr->results.size() != 3)
	{
		printf("subscriber results count wrong type: %d\n", pptr->results.size());
		return 0;
	}
	auto it = pptr->results.begin();
	//std::string stype = (*it)->get_string();
	++it;
	*channel = (*it)->get_string();
	++it;
	*message = (*it)->get_string();
	return 1;
}

void Subscriber::stop_recv()
{
	_is_stop = true;
}

NS_2
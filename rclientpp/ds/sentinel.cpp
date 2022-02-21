#include "sentinel.h"
#include "../core/rclient.h"
//#include "../core/async_socket_client.h"

NS_1

Sentinel::Sentinel(const SentinelOptions& options)
	:_options(options)
{
}

int Sentinel::check()
{
	for (int i = 0; i < _options.nodes.size(); ++i)
	{
		auto ac = std::make_shared<RClient>(_options.nodes[i].first, _options.nodes[i].second);
		printf("connect to sentinel server[%s:%d]\n", _options.nodes[i].first.c_str(), _options.nodes[i].second);
		bool flag = ac->connect(_options.connect_timeout);
		if (flag != 0)
		{
			printf("connect to sentinel server[%s:%d] failed\n", _options.nodes[i].first.c_str(), _options.nodes[i].second);
			continue;
		}
		printf("connect to sentinel server[%s:%d] successfully\n", _options.nodes[i].first.c_str(), _options.nodes[i].second);
		ac->set_read_timeout(_options.socket_timeout);
		//if (_options.password.size() > 0)
		//{
		//	//AUTH...
		//}
		int ret = ac->use_resp3();
		if (ret != 0)
		{
			printf("server[%s:%d] user RESP 3 failed\n", _options.nodes[i].first.c_str(), _options.nodes[i].second);
		}
		_sentinel_connetions.push_back(ac);
		printf("\n");
	}
	return _sentinel_connetions.size();
}

std::vector<RedisNode> Sentinel::get_masters()
{
	std::vector<RedisNode> nodes;

	std::string cmd = "SENTINEL masters\r\n";
	for (int i = 0; i < _sentinel_connetions.size(); ++i)
	{
		int ret = _sentinel_connetions[i]->command(cmd.c_str(), cmd.size());
		if (ret != cmd.size())
		{
			continue;
		}
		auto ptr = _sentinel_connetions[i]->get_results(ret);
		if (!ptr || ret != 0)
		{
			continue;
		}
		if (ptr->value_type() != ParserType::Array)
		{
			//something error...
			continue;
		}
		auto pptr = std::dynamic_pointer_cast<RedisComplexValue>(ptr);
		//auto it = pptr->results.begin();
		//for (; it != pptr->results.end(); ++it)
		//{
		//	if ((*it)->value_type() != ParserType::Array
		//		&& (*it)->value_type() != ParserType::Map)
		//	{
		//		//something error...
		//		return nodes;
		//	}
		//	auto master_ptr = std::dynamic_pointer_cast<RedisComplexValue>(*it);
		//	RedisNode node;
		//	node.attrs = to_attrs(master_ptr);
		//	if (node.attrs)
		//	{
		//		node.attrs->query("name", node.name);
		//		node.attrs->query("ip", node.ip);
		//		std::string port;
		//		node.attrs->query("port", port);
		//		node.port = std::atoi(port.c_str());
		//	}

		//	nodes.push_back(node);
		//}
		auto attris = to_bulk_attrs(pptr);
		for (int i = 0; i < attris.size(); ++i)
		{
			if (!attris[i])
			{
				continue;
			}
			RedisNode node;
			node.attrs = attris[i];
			node.attrs->query("name", node.name);
			node.attrs->query("ip", node.ip);
			std::string port;
			node.attrs->query("port", port);
			node.port = std::atoi(port.c_str());
			nodes.push_back(node);
		}
		break;
	}
	return nodes;
}

std::shared_ptr<RedisNode> Sentinel::get_master_by_name(const std::string& master_name)
{
	std::string cmd = "SENTINEL get-master-addr-by-name ";
	cmd = cmd + master_name + "\r\n";
	for (int i = 0; i < _sentinel_connetions.size(); ++i)
	{
		int ret = _sentinel_connetions[i]->command(cmd.c_str(), cmd.size());
		if (ret != cmd.size())
		{
			continue;
		}
		auto ptr = _sentinel_connetions[i]->get_results(ret);
		if (ret != 0)
		{
			continue;
		}
		if (ptr && ptr->value_type() == ParserType::Array)
		{
			auto pptr = std::dynamic_pointer_cast<RedisComplexValue>(ptr);
			if (pptr->count != 2)
			{
				return nullptr;
			}
			std::shared_ptr<RedisNode> node = std::make_shared<RedisNode>();
			node->ip = get_string(*pptr->results.begin());
			node->port = get_number(*(++pptr->results.begin()));
			return node;
		}
	}
	return nullptr;
}

std::vector<RedisNode> Sentinel::get_slaves(const std::string& master_name)
{
	std::vector<RedisNode> nodes;

	std::string cmd = "SENTINEL slaves ";
	cmd = cmd + master_name + "\r\n";
	for (int i = 0; i < _sentinel_connetions.size(); ++i)
	{
		int ret = _sentinel_connetions[i]->command(cmd.c_str(), cmd.size());
		if (ret != cmd.size())
		{
			continue;
		}
		auto ptr = _sentinel_connetions[i]->get_results(ret);
		if (!ptr || ret != 0)
		{
			continue;
		}
		if (ptr->value_type() != ParserType::Array)
		{
			//something error...
			continue;
		}
		auto pptr = std::dynamic_pointer_cast<RedisComplexValue>(ptr);
		auto attris = to_bulk_attrs(pptr);
		for (int i = 0; i < attris.size(); ++i)
		{
			if (!attris[i])
			{
				continue;
			}
			RedisNode node;
			node.attrs = attris[i];
			node.attrs->query("name", node.name);
			node.attrs->query("ip", node.ip);
			std::string port;
			node.attrs->query("port", port);
			node.port = std::atoi(port.c_str());
			nodes.push_back(node);
		}
		break;
	}

	return nodes;
}


NS_2
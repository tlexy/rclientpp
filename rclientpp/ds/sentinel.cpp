#include "sentinel.h"
#include "../core/rclient.h"
//#include "../core/async_socket_client.h"

NS_1

Sentinel::Sentinel(SentinelOptions options)
	:_options(std::move(options))
{
}

std::size_t Sentinel::check()
{
	for (auto& node : _options.nodes)
    {
		auto ac = std::make_shared<RClient>(node.first, node.second);
		printf("connect to sentinel server[%s:%d]\n", node.first.c_str(), node.second);
		const bool flag = ac->connect(_options.connect_timeout);
		if (flag != 0)
		{
			printf("connect to sentinel server[%s:%d] failed\n", node.first.c_str(), node.second);
			continue;
		}
		printf("connect to sentinel server[%s:%d] successfully\n", node.first.c_str(), node.second);
		ac->set_read_timeout(_options.socket_timeout);
		//if (_options.password.size() > 0)
		//{
		//	//AUTH...
		//}
		int ret = ac->use_resp3();
		if (ret != 0)
		{
			printf("server[%s:%d] user RESP 3 failed\n", node.first.c_str(), node.second);
		}
		_sentinel_connections.push_back(ac);
		printf("\n");
	}
	return _sentinel_connections.size();
}

std::vector<RedisNode> Sentinel::get_masters() const
{
	std::vector<RedisNode> nodes;

	const std::string cmd = "SENTINEL masters\r\n";
	for (const auto& _sentinel_connection : _sentinel_connections)
    {
		auto ret = _sentinel_connection->command(cmd.c_str(), cmd.size());
		if (ret != cmd.size())
		{
			continue;
		}
		const auto ptr = _sentinel_connection->get_results(ret);
		if (!ptr || ret != 0)
		{
			continue;
		}
		if (ptr->value_type() != ParserType::Array)
		{
			//something error...
			continue;
		}
		const auto pptr = std::dynamic_pointer_cast<RedisComplexValue>(ptr);
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
		const auto attris = to_bulk_attrs(pptr);
		for (const auto& attri : attris)
        {
			if (!attri)
			{
				continue;
			}
			RedisNode node;
			node.attrs = attri;
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

std::shared_ptr<RedisNode> Sentinel::get_master_by_name(const std::string& master_name) const
{
	std::string cmd = "SENTINEL get-master-addr-by-name ";
	cmd += master_name + "\r\n";
	for (const auto& _sentinel_connection : _sentinel_connections)
    {
		auto ret = _sentinel_connection->command(cmd.c_str(), cmd.size());
		if (ret != cmd.size())
		{
			continue;
		}
		const auto ptr = _sentinel_connection->get_results(ret);
		if (ret != 0)
		{
			continue;
		}
		if (ptr && ptr->value_type() == ParserType::Array)
		{
			const auto pptr = std::dynamic_pointer_cast<RedisComplexValue>(ptr);
			if (pptr->count != 2)
			{
				return nullptr;
			}
            auto node = std::make_shared<RedisNode>();
			node->ip = get_string(*pptr->results.begin());
			node->port = static_cast<int>(get_number(*(++pptr->results.begin())));
			return node;
		}
	}
	return nullptr;
}

std::vector<RedisNode> Sentinel::get_slaves(const std::string& master_name) const
{
	std::vector<RedisNode> nodes;

	std::string cmd = "SENTINEL slaves ";
	cmd += master_name + "\r\n";
	for (const auto& _sentinel_connection : _sentinel_connections)
    {
		auto ret = _sentinel_connection->command(cmd.c_str(), cmd.size());
		if (ret != cmd.size())
		{
			continue;
		}
		const auto ptr = _sentinel_connection->get_results(ret);
		if (!ptr || ret != 0)
		{
			continue;
		}
		if (ptr->value_type() != ParserType::Array)
		{
			//something error...
			continue;
		}
		const auto pptr = std::dynamic_pointer_cast<RedisComplexValue>(ptr);
		const auto attris = to_bulk_attrs(pptr);
		for (const auto& attri : attris)
        {
			if (!attri)
			{
				continue;
			}
			RedisNode node;
			node.attrs = attri;
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
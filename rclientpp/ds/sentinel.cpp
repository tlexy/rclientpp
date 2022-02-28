#include "sentinel.h"
#include "../core/rclient.h"
#include "subscriber.h"
#include "../core/rclient_utils.h"
//#include "../core/async_socket_client.h"

NS_1

Sentinel::Sentinel(const SentinelOptions& options)
	:_options(options)
{
}

int Sentinel::check()
{
	_sentinel_connetions.clear();
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

void Sentinel::event_func(std::shared_ptr<std::string> ev, std::shared_ptr<std::string> msg)
{
	printf("event: %s, msg: %s\n", ev->c_str(), msg->c_str());
}

void Sentinel::event_funcp(std::shared_ptr<std::string> pat, std::shared_ptr<std::string> ev, std::shared_ptr<std::string> msg)
{
	printf("pattern: %s, event: %s, msg: %s\n", pat->c_str(), ev->c_str(), msg->c_str());

	std::vector<std::string> vecs;
	split(*msg, " ", vecs);
	if (vecs.size() < 4)
	{
		printf("event func msg error\n");
		return;
	}

	SentinelEventType ev_type = SentinelEventType::PsDown;
	if (*ev == std::string("+sdown"))
	{

	}
	else if (*ev == std::string("-sdown"))
	{
		ev_type = SentinelEventType::SsDown;
	}
	else if (*ev == std::string("+odown"))
	{
		ev_type = SentinelEventType::PoDown;
	}
	else if (*ev == std::string("-odown"))
	{
		ev_type = SentinelEventType::SoDown;
	}
	else if (*ev == std::string("+failover-end"))
	{
		ev_type = SentinelEventType::FailoverEnd;
	}
	else if (*ev == std::string("+switch-master"))
	{
		ev_type = SentinelEventType::SwitchMaster;
		auto old_node = std::make_shared<RedisNode>();
		old_node->name = vecs[0];
		old_node->ip = vecs[1];
		old_node->port = std::atoi(vecs[2].c_str());

		auto new_node = std::make_shared<RedisNode>();
		new_node->name = vecs[0];
		new_node->ip = vecs[3];
		new_node->port = std::atoi(vecs[4].c_str());

		if (_event_func)
		{
			_event_func(ev_type, old_node, new_node);
		}
		return;
	}
	else if (*ev == std::string("+slave"))
	{
		ev_type = SentinelEventType::PSlave;
	}
	else
	{
		printf("unhandle event: ev:%s, %s\n", ev->c_str(), msg->c_str());
		return;
	}

	std::shared_ptr<RedisNode> node1 = std::make_shared<RedisNode>();
	std::string inst_type = vecs[0];
	if (inst_type == "master")
	{
		node1->role = RedisRoleType::Master;
	}
	else if (inst_type == "slave")
	{
		node1->role = RedisRoleType::Slave;
	}
	node1->name = vecs[1];
	node1->ip = vecs[2];
	node1->port = std::atoi(vecs[3].c_str());

	std::shared_ptr<RedisNode> master_node = nullptr;
	if (vecs.size() > 6 && vecs[4] == "@")
	{
		master_node = std::make_shared<RedisNode>();
		master_node->role = RedisRoleType::Master;
		master_node->name = vecs[5];
		master_node->ip = vecs[6];
		master_node->port = std::atoi(vecs[7].c_str());
	}

	if (_event_func)
	{
		_event_func(ev_type, node1, master_node);
	}
}

void Sentinel::regist_handle(SentinelEventHandleFunc func)
{
	_event_func = func;
}

void Sentinel::subscribe_event()
{
	do_subscribe_event({ "*" });
}

void Sentinel::stop_subscribe_event()
{
	std::lock_guard<std::mutex> lock(_subs_mutex);
	for (int i = 0; i < _subscribers.size(); ++i)
	{
		_subscribers[i]->stop_recv();
	}
}

int Sentinel::do_subscribe_event(std::initializer_list<std::string> events)
{
	for (int i = 0; i < _sentinel_connetions.size(); ++i)
	{
		_sentinel_connetions[i]->set_read_timeout(1000);
	}

	_subscribers.clear();
	using namespace std::placeholders;
	for (int i = 0; i < _sentinel_connetions.size(); ++i)
	{
		if (_sentinel_connetions[i]->check_connection())
		{
			continue;
		}
		auto subscriber = std::make_shared<Subscriber>(_sentinel_connetions[i]);
		{
			std::lock_guard<std::mutex> lock(_subs_mutex);
			_subscribers.push_back(subscriber);
		}
		/*int ret = subscriber->subscribe(events);
		if (ret == events.size())
		{
			subscriber->regist_handle(std::bind(&Sentinel::event_func, this, _1, _2));
			subscriber->recv_msg();
		}*/
		int ret = subscriber->p_subscribe(events);
		if (ret == events.size())
		{
			subscriber->regist_p_handle(std::bind(&Sentinel::event_funcp, this, _1, _2, _3));
			ret = subscriber->recv_p_msg();
		}
		else
		{
			printf("subscribe_event, ret: %d\n", ret);
		}
		break;
	}
	return 0;
}


NS_2
#include "core/rclient_def.h"
#include "core/rclient.h"
#include <iostream>
#include "core/sock_utils.h"
#include "ds/rdstring.h"
#include "ds/rdhash.h"
#include "core/async_socket_client.h"
#include <cstring>
#include <algorithm>
#include "ds/sentinel.h"
#include "ds/rdset.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "Userenv.lib")

using namespace rcpp;

void test_connect()
{
	std::string ip("81.71.77.77");
	int port = 6379;

	RClient* client = new RClient(ip, port);
	int ret = client->connect("123456");
	if (ret != 0)
	{
		std::cout << "Err: " << client->strerror() << std::endl;
	}
}

void test_string()
{
	std::string ip("81.71.77.77");
	int port = 6379;

	auto client = std::make_shared<RClient>(ip, port);
	int ret = client->connect("123456");
	if (ret != 0)
	{
		std::cout << "Err: " << client->strerror() << std::endl;
		return;
	}

	ret = client->use_resp3();
	if (ret != 0)
	{
		return;
	}

	auto dss = std::make_shared<RdString>(client);
	/*dss->get("test2");
	dss->set("test1", "abc");
	dss->get("test1");
	dss->get("test3");
	dss->get("test5");
	dss->get("test6");*/

	auto ptr = dss->redis_command("hgetall aaa\r\n", strlen("hgetall aaa\r\n"), ret);
	int a = 1;
}

void test_hash()
{
	std::string ip("81.71.77.77");
	int port = 6379;

	auto client = std::make_shared<RClient>(ip, port);
	int ret = client->connect("123456");
	if (ret != 0)
	{
		std::cout << "Err: " << client->strerror() << std::endl;
		return;
	}

	ret = client->use_resp3();
	if (ret != 0)
	{
		return;
	}
	auto dss = std::make_shared<RdHash>(client);
	dss->hset("bbb", "name", "lin");
	dss->hmget("bbb", {"name1", "name2", "3"});
}

void test_sentinel()
{
	SentinelOptions options;
	options.nodes = { {"81.71.77.77", 26379}, {"81.71.77.78", 26379},{"81.71.77.79", 26379} };

	Sentinel* sent = new Sentinel(options);
	int count = sent->check();
	auto nodes = sent->get_masters();
	if (nodes.size() > 0)
	{
		auto slaves = sent->get_slaves(nodes[0].name);
		auto master = sent->get_master_by_name(nodes[0].name);
		int b = 2;
	}
	int a = 1;
}

void test_set()
{
	std::string ip("81.71.77.77");
	int port = 6379;

	auto client = std::make_shared<RClient>(ip, port);
	int ret = client->connect("123456");
	if (ret != 0)
	{
		std::cout << "Err: " << client->strerror() << std::endl;
		return;
	}

	ret = client->use_resp3();
	if (ret != 0)
	{
		return;
	}
	auto set_client = std::make_shared<RdSet>(client);
	int count = set_client->sadd("test_set", { 10, 9, 8 });
	std::cout << "set count: " << count << std::endl;
	auto results = set_client->smembers("test_set");
	if (results)
	{
		if (results->value_type() != ParserType::Set
			&& results->value_type() != ParserType::Array)
		{
			return;
		}
		std::cout << "values:";
		auto ptr = std::dynamic_pointer_cast<RedisComplexValue>(results);
		auto it = ptr->results.begin();
		for (; it != ptr->results.end(); ++it)
		{
			if ((*it)->is_string())
			{
				auto ptr = std::dynamic_pointer_cast<RedisValue>(*it);
				std::cout << " " << ptr->str_val_;
			}
		}
		std::cout << std::endl;
	}
	bool flag = set_client->is_member("test_set", 2);
	std::cout << "flag:" << flag << std::endl;
	int a = 1;
}

int main()
{
	rcppsockets::Init();

	/*char buff[64] = { '\0' };
	std::string ss = "hello aaa";
	std::copy(ss.c_str(), ss.c_str() + ss.size(), buff);
	int a = 1;*/

	//1. test_connect();
	//2. test_string();
	//3. test_async_client();
	//4. test_hash();
	//5. test_sentinel();
	//6. test_set();

	/* connect to redis server...
	*
	std::string ip("81.71.41.235");
	int port = 6380;

	auto client = std::make_shared<RClient>(ip, port);
	int ret = client->connect("tttaBa231_?");
	if (ret != 0)
	{
		std::cout << "Err: " << client->strerror() << std::endl;
	}
	else
	{
		std::cout << "connect successfully..." << std::endl;
	}
	*/

	/* Send command and wait for responses
	* 
	std::string ip("81.71.41.235");
	int port = 6380;

	auto client = std::make_shared<RClient>(ip, port);
	int ret = client->connect("tttaBa231_?");
	if (ret != 0)
	{
		std::cout << "Err: " << client->strerror() << std::endl;
	}
	else
	{
		std::cout << "connect successfully..." << std::endl;
	}

	//transfer to RESP 3
	ret = client->use_resp3();
	if (ret != 0)
	{
		return 1;
	}
	std::string cmd = "sadd test_set 101 102\r\n";
	int written_len = client->command(cmd.c_str(), cmd.size());
	if (written_len != cmd.size())
	{
		//send error
		return 1;
	}
	int ret_code = 0;
	auto result_ptr = client->get_results(ret_code);
	if (result_ptr)
	{
		if (result_ptr->value_type() == ParserType::Number)
		{
			auto ptr = std::dynamic_pointer_cast<RedisValue>(result_ptr);
			std::cout << "result : " << ptr->u.int_val_ << std::endl;
		}
	}
	else
	{
		std::cout << "get results error: " << ret_code << std::endl;
	}
	*/

	std::cin.get();

	rcppsockets::Destroy();

	return 0;
}
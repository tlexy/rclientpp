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
    const std::string ip("81.71.77.77");
	constexpr int port = 6379;

    auto* client = new RClient(ip, port);
    const int ret = client->connect("123456");
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
	const int ret = client->connect("123456");
	if (ret != 0)
	{
		std::cout << "Err: " << client->strerror() << std::endl;
		return;
	}

	auto ret2 = client->use_resp3();
	if (ret2 != 0)
	{
		return;
	}

    const auto dss = std::make_shared<RdString>(client);
	/*dss->get("test2");
	dss->set("test1", "abc");
	dss->get("test1");
	dss->get("test3");
	dss->get("test5");
	dss->get("test6");*/

	auto ptr = dss->redis_command("hgetall aaa\r\n", strlen("hgetall aaa\r\n"), ret2);
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

	const auto ret2 = client->use_resp3();
	if (ret2 != 0)
	{
		return;
	}
    const auto dss = std::make_shared<RdHash>(client);
	dss->hset("bbb", "name", "lin");
	dss->hmget("bbb", {"name1", "name2", "3"});
}

void test_sentinel()
{
	SentinelOptions options;
	options.nodes = { {"81.71.77.77", 26379}, {"81.71.77.78", 26379},{"81.71.77.79", 26379} };

    const auto sent = new Sentinel(options);
	auto count = sent->check();
    const auto nodes = sent->get_masters();
	if (!nodes.empty())
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

	const auto ret2 = client->use_resp3();
	if (ret2 != 0)
	{
		return;
	}
    const auto set_client = std::make_shared<RdSet>(client);
    const auto count = set_client->sadd("test_set", { 10, 9, 8 });
	std::cout << "set count: " << count << std::endl;
    const auto results = set_client->smembers("test_set");
	if (results)
	{
		if (results->value_type() != ParserType::Set
			&& results->value_type() != ParserType::Array)
		{
			return;
		}
		std::cout << "values:";
        const auto ptr = std::dynamic_pointer_cast<RedisComplexValue>(results);
        for (auto& result : ptr->results)
        {
			if (result->is_string())
			{
				auto ptr = std::dynamic_pointer_cast<RedisValue>(result);
				std::cout << " " << ptr->str_val_;
			}
		}
		std::cout << std::endl;
	}
    const bool flag = set_client->is_member("test_set", 2);
	std::cout << "flag:" << flag << std::endl;
}

int main()
{
	sockets::Init();

	/*char buff[64] = { '\0' };
	std::string ss = "hello aaa";
	std::copy(ss.c_str(), ss.c_str() + ss.size(), buff);
	int a = 1;*/

	//1. test_connect();
	//2. test_string();
	//3. test_async_client();
	//4. test_hash();
	//5. test_sentinel();
	test_set();
	

	std::cin.get();

	sockets::Destroy();

	return 0;
}
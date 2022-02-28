#ifndef RCPP_RD_SENTINEL_H
#define RCPP_RD_SENTINEL_H

#include "rd.h"
#include <memory>
#include <vector>
#include <utility>
#include <initializer_list>
#include <functional>
#include <mutex>

NS_1

//class AsyncSocketClient;

struct SentinelOptions 
{
    std::vector<std::pair<std::string, int>> nodes;

    std::string password; //to do...

    bool keep_alive = true;

    int connect_timeout{ 3000 };

    int socket_timeout{ 10000 };

    int retry_interval{ 1000 };

    std::size_t max_retry = 3; //to do...
};

//redis节点
struct RedisNode
{
    RedisRoleType role;
    std::string name;
    std::string ip;
    int port;
    std::shared_ptr<Attributes> attrs;
};

/*
* <instance-type> <name> <ip> <port> @ <master-name> <master-ip> <master-port>
* something master is nullptr
* */
using SentinelEventHandleFunc = std::function<void(SentinelEventType, std::shared_ptr<RedisNode> name, std::shared_ptr<RedisNode> master_name)>;

class Subscriber;

class Sentinel
{
public:
	Sentinel(const SentinelOptions&);

    //检查sentinel节点的连接情况，并返回可连接的sentinel节点数量
    int check();

    std::vector<RedisNode> get_masters();
    std::shared_ptr<RedisNode> get_master_by_name(const std::string&);
    std::vector<RedisNode> get_slaves(const std::string&);

    void regist_handle(SentinelEventHandleFunc func);

    void subscribe_event();
    void stop_subscribe_event();

private:
    void event_func(std::shared_ptr<std::string>, std::shared_ptr<std::string>);
    void event_funcp(std::shared_ptr<std::string>, std::shared_ptr<std::string>, std::shared_ptr<std::string>);

    int do_subscribe_event(std::initializer_list<std::string> events);

private:
    SentinelOptions _options;
    SentinelEventHandleFunc _event_func;
    std::vector<std::shared_ptr<RClient>> _sentinel_connetions;
    std::vector<std::shared_ptr<Subscriber>> _subscribers;
    std::mutex _subs_mutex;
};

NS_2

#endif
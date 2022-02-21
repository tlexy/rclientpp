#ifndef RCPP_RD_SENTINEL_H
#define RCPP_RD_SENTINEL_H

#include "rd.h"
#include <memory>
#include <vector>
#include <utility>

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

    std::size_t max_retry = 3;
};

//redis节点
struct RedisNode
{
    std::string name;
    std::string ip;
    int port;
    std::shared_ptr<Attributes> attrs;
};

class Sentinel
{
public:
	Sentinel(const SentinelOptions&);

    //检查sentinel节点的连接情况，并返回可连接的sentinel节点数量
    int check();

    std::vector<RedisNode> get_masters();
    std::shared_ptr<RedisNode> get_master_by_name(const std::string&);
    std::vector<RedisNode> get_slaves(const std::string&);

private:
    SentinelOptions _options;

    std::vector<std::shared_ptr<RClient>> _sentinel_connetions;
};

NS_2

#endif
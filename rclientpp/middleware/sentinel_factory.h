#ifndef RCPP_MW_SENTINEL_FACTOROY_H
#define RCPP_MW_SENTINEL_FACTOROY_H

#include "../ds/sentinel.h"
#include <thread>
#include <queue>
#include <atomic>

NS_1

enum SfTaskType
{
	etInvalidTaskType = 0,
	etCheckMaster = 1,	//需要检查master的连接
	etMasterObjDown = 2,  //master下线了
	etMasterObjUp = 3,	//master恢复了
	etMasterChange = 4,	//有新的master
	etMasterCheck = 5, //定期检查状态并重新连接master...
	etRedisCommandBegin = 100,
	etRedisCommandNormal = 101,
	etSfTaskTypeEnd
};

struct SfTask
{
	SfTask()
		:type(etInvalidTaskType)
	{}
	SfTaskType type;
	std::string cmd;
};

struct SfTaskLess
{
	bool operator() (std::shared_ptr<SfTask> a, std::shared_ptr<SfTask> b)
	{
		return a->type > b->type;
	}
};

struct SfThreadQueue
{
	std::mutex mutex;
	std::atomic<int> state;//0: ok; 1: busy; 2: unusable
	std::priority_queue<std::shared_ptr<SfTask>, std::vector<std::shared_ptr<SfTask>>, SfTaskLess> queue;
};

using MasteStateHandle = std::function<void(SentinelEventType ev, std::shared_ptr<RedisNode>)>;

/*
* 可能的错误：连接断开、连接超时、master不可用、master更换
* 解决方案：
* 1 连接断开或者连接超时时，首先尝试检查连接，如果连接有问题，尝试重新连接，
*	重连也不成功时，认为master不可用。
* 2 当master不可用时或者更换时，将尝试重新连接master，尝试一次连接不成功后，
	退出命令执行线程，该线程将不再可用于执行redis命令
* 3 当外部收到ConnectNewMasterFailed通知时，说明执行命令的线程退出了
* 4 当外部收到MasterObjDown通知时，需要特别注意，这时线程没有退出，可能永远也无法返回
* 5 对于执行失败的命令，永远不会重新执行；重新连接后的线程队列，会执行之前未执行的任务。
*/
class SentinelFactory
{
public:
	SentinelFactory(const SentinelOptions&, const std::string& master_name, const std::string& master_auth);
	~SentinelFactory();

	bool init(int master_connection_pool_size);

	std::shared_ptr<RedisNode> get_master_node();

	void set_state_handler(MasteStateHandle);

	//检查master连接状态并尝试重新连接
	//返回状态正常的连接个数
	int check_and_retry_master();

	//异步redis命令
	int async_command(const std::string&);
	int async_command(const char* cmd, int len);

	//you should run it with a new thread
	int start();
	void stop();

	static std::string to_string(SentinelEventType);

	//std::shared_ptr<RClient> get_master_connection();

private:
	void sentinel_event_handle(SentinelEventType, std::shared_ptr<RedisNode> name, std::shared_ptr<RedisNode> master_name);

	int sentinel_state_handle();
	//执行cmd线程
	//当处于错误状态并且没有新的事件发生时，错误不会自己恢复，线程也不会退出
	void command_thread(int i);

private:
	SentinelOptions _config;
	std::string _master_name;
	std::string _master_auth;
	std::shared_ptr<Sentinel> _sentinel;
	//std::vector<std::shared_ptr<RClient>> _master_connections;
	int _ms_count{0};
	std::mutex _master_mutex;
	std::shared_ptr<RedisNode> _master_node;
	bool _is_state_stop{ true };
	bool _cmd_thread_stop{ true };
	MasteStateHandle _state_handler;
	std::vector<std::shared_ptr<std::thread>> _th_group;
	std::vector<std::shared_ptr<SfThreadQueue>> _qu_group;//优先队列
};

NS_2

#endif
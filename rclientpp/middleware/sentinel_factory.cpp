#include "sentinel_factory.h"
#include "../core/rclient.h"

NS_1

SentinelFactory::SentinelFactory(const SentinelOptions& options, const std::string& master_name, const std::string& master_auth)
	:_config(options),
	_master_name(master_name),
	_master_auth(master_auth),
	_sentinel(std::make_shared<Sentinel>(options)),
	_master_node(std::shared_ptr<RedisNode>()),
	_state_handler(MasteStateHandle())
{
}

//std::shared_ptr<RClient> SentinelFactory::get_master_connection()
//{
//	if (_master_connections.size() < 1 || _ms_count < 1)
//	{
//		return nullptr;
//	}
//	if (_ms_count >= _master_connections.size())
//	{
//		_ms_count = 0;
//	}
//	for (int i = _ms_count; i < _master_connections.size(); ++i)
//	{
//		if (_master_connections[i]->is_connected())
//		{
//			return _master_connections[i++];
//		}
//	}
//	return nullptr;
//}

bool SentinelFactory::init(int master_connection_pool_size)
{
	//check sentinel node and get master infos
	_ms_count = master_connection_pool_size;
	int count = _sentinel->check();
	if (count < 1)
	{
		return false;
	}
	
	auto node = _sentinel->get_master_by_name(_master_name);
	if (!node)
	{
		return false;
	}
	_master_node = node;
	printf("master: %s-%d\n", _master_node->ip.c_str(), _master_node->port);
	//set the sentinel event handler
	using namespace std::placeholders;
	_sentinel->regist_handle(std::bind(&SentinelFactory::sentinel_event_handle, this, _1, _2, _3));
	
	//create thread pool to handle command...
	for (int i = 0; i < _ms_count; ++i)
	{
		auto quptr = std::make_shared<SfThreadQueue>();
		quptr->state = 2;
		_qu_group.push_back(quptr);
	}
	for (int i = 0; i < _ms_count; ++i)
	{
		_th_group.push_back(std::make_shared<std::thread>(&SentinelFactory::command_thread, this, i));
	}
	return true;
}

int SentinelFactory::async_command(const std::string& rcmd)
{
	static unsigned int turn = 0;
	if (turn >= _qu_group.size())
	{
		turn = 0;
	}
	if (_qu_group[turn]->state == 2)
	{
		for (int i = 0; i < _qu_group.size(); ++i)
		{
			if (_qu_group[i]->state != 2)
			{
				auto task = std::make_shared<SfTask>();
				task->type = etRedisCommandNormal;
				task->cmd = rcmd;
				_qu_group[turn]->mutex.lock();
				_qu_group[turn]->queue.push(task);
				_qu_group[turn]->mutex.unlock();
				return 1;
			}
		}
	}
	else
	{
		auto task = std::make_shared<SfTask>();
		task->type = etRedisCommandNormal;
		task->cmd = rcmd;
		_qu_group[turn]->mutex.lock();
		_qu_group[turn]->queue.push(task);
		_qu_group[turn]->mutex.unlock();
		return 1;
	}
	++turn;
	return 0;
}

int SentinelFactory::check_and_retry_master()
{
	int ret = 0;
	auto task = std::make_shared<SfTask>();
	task->type = etMasterCheck;
	//将消息发送到每一个线程
	for (int i = 0; i < _qu_group.size(); ++i)
	{
		if (_qu_group[i]->state != 0)
		{
			std::lock_guard<std::mutex> lock(_qu_group[i]->mutex);
			_qu_group[i]->queue.push(task);
		}
		else
		{
			++ret;
		}
	}
	return ret;
}

void SentinelFactory::command_thread(int idx)
{
	if (_qu_group.size() < idx + 1)
	{
		printf("queue group too small\n");
		return;
	}
	auto client = std::make_shared<RClient>(_master_node->ip, _master_node->port);
	if (!client)
	{
		return;
	}
	int ret = client->connect(_master_auth);
	if (ret != 0)
	{
		return;
	}
	client->use_resp3();
	auto role = client->get_role();
	if (role != RedisRoleType::Master)
	{
		printf("[%s-%d] is not master node\n", _master_node->ip.c_str(), _master_node->port);
		return;
	}
	_cmd_thread_stop = false;
	printf("SentinelFactory, thread start: %d\n", idx);
	int ret_code = 0;
	auto qu = _qu_group[idx];
	qu->state = 0;//开始接收外部请求
	int state = 0;//0: normal, 1: check for connection, 2: change master, 3: master odown
	while (!_cmd_thread_stop)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		//1. 从任务列表中获取任务
		//2. 执行任务
		std::shared_ptr<SfTask> task = nullptr;
		qu->mutex.lock();
		if (!qu->queue.empty())
		{
			task = qu->queue.top();
			qu->queue.pop();
		}
		qu->mutex.unlock();

		if (!task)
		{
			continue;
		}
		if (task->type == etInvalidTaskType)
		{
			continue;
		}
		if (task->type == etSfTaskTypeEnd)
		{
			break;
		}
		if (task->type > etRedisCommandBegin)
		{
			qu->state = 1;
			int w_len = client->command(task->cmd.c_str(), task->cmd.size());
			if (w_len != task->cmd.size())
			{
				printf("write redis command error, code: %d\n", w_len);
				continue;
			}
			auto ptr = client->get_results(ret_code);
			if (ret_code != 0)//超时或者出错
			{
				task->type = etCheckMaster;
				goto M_ERROR;
			}
			else
			{
				if (ptr->value_type() == ParserType::BlobError
					|| ptr->value_type() == ParserType::SimpleError)
				{
					printf("redis comand error: %s, thread: %d\n", ptr->get_string().c_str(), idx);
				}
			}
			qu->state = 0;
			continue;
		}

	M_ERROR:
		//处理错误。。。
		{
			if (state == 3)
			{
				//当+odown发生后，只处理-odown以及switch master
				if (task->type != etMasterObjUp && task->type != etMasterChange)
				{
					continue;
				}
			}
			if (task->type == etMasterCheck && qu->state == 0)
			{
				//状态正常的情况下，周期性的检查不需要处理
				continue;
			}
			qu->state = 2;//不可用
			if (task->type < etRedisCommandBegin)
			{
				if (task->type == etCheckMaster || task->type == etMasterObjUp)
				{
					state = 1;
				}
				else if (task->type == etMasterChange || task->type == etMasterCheck)
				{
					state = 2;
				}
				else if (task->type == etMasterObjDown)
				{
					state = 3;
				}
			}
			//
			if (state == 1)
			{
				//检查连接
				int ret = client->check_connection();
				if (ret == 0)
				{
					qu->state = 0;//恢复可用
					state = 0;
					continue;
				}
				else
				{
					////重新连接
					//ret = client->reconnect(3000);
					//if (ret == 0)
					//{
					//	state = 0;
					//	qu->state = 0;
					//	continue;
					//}
					//else
					//{
						qu->state = 2;
						state = 2;//需要重新连接master
					//}
				}
			}
			//
			if (state == 2)
			{
				client->close();
				//检查新的主节点。。。
				if (!_master_node)
				{
					_state_handler(SentinelEventType::ConnectNewMasterFailed, _master_node);
					printf("master node is nullptr\n");
					//break;
					continue;
				}
				client = std::make_shared<RClient>(_master_node->ip, _master_node->port);
				if (!client)
				{
					_state_handler(SentinelEventType::ConnectNewMasterFailed, _master_node);
					//break;
					continue;
				}
				int ret = client->connect(_master_auth);
				if (ret != 0)
				{
					_state_handler(SentinelEventType::ConnectNewMasterFailed, _master_node);
					//break;
					continue;
				}
				//检查是否是主节点
				if (client->get_role() != RedisRoleType::Master)
				{
					_state_handler(SentinelEventType::ConnectNewMasterFailed, _master_node);
					client->close();
					continue;
				}
				state = 0;
				qu->state = 0;
				_state_handler(SentinelEventType::ConnectNewMasterSucceed, _master_node);
			}
			//
			if (state == 3)
			{
				//出错了，等待恢复
			}
		}
	}
	
	printf("SentinelFactory, thread stop: %d\n", idx);
}

std::string SentinelFactory::to_string(SentinelEventType ev)
{
	if (ev == SentinelEventType::SsDown)
	{
		return std::string("SsDown");
	}
	else if (ev == SentinelEventType::PsDown)
	{
		return std::string("PsDown");
	}
	else if (ev == SentinelEventType::SoDown)
	{
		return std::string("SoDown");
	}
	else if (ev == SentinelEventType::PoDown)
	{
		return std::string("PoDown");
	}
	else if (ev == SentinelEventType::FailoverEnd)
	{
		return std::string("FailoverEnd");
	}
	else if (ev == SentinelEventType::SwitchMaster)
	{
		return std::string("SwitchMaster");
	}
	else if (ev == SentinelEventType::PSlave)
	{
		return std::string("PSlave");
	}
	else if (ev == SentinelEventType::SentinelSubscribeReturn)
	{
		return std::string("SentinelSubscribeReturn");
	}
	else if (ev == SentinelEventType::ConnectNewMasterFailed)
	{
		return std::string("ConnectNewMasterFailed");
	}
	else if (ev == SentinelEventType::ConnectNewMasterSucceed)
	{
		return std::string("ConnectNewMasterSucceed");
	}
	else if (ev == SentinelEventType::MasterObjDown)
	{
		return std::string("MasterObjDown");
	}
	else 
	{
		return std::string("aba");
	}
}

std::shared_ptr<RedisNode> SentinelFactory::get_master_node()
{
	std::lock_guard<std::mutex> lock(_master_mutex);
	return _master_node;
}

void SentinelFactory::set_state_handler(MasteStateHandle handler)
{
	_state_handler = handler;
}

int SentinelFactory::start()
{
	//wait the sentinel publish message and handle it.
	return sentinel_state_handle();
}

void SentinelFactory::stop()
{
	_is_state_stop = true;
	_cmd_thread_stop = true;
	_sentinel->stop_subscribe_event();
	for (int i = 0; i < _th_group.size(); ++i)
	{
		_th_group[i]->join();
	}
}

int SentinelFactory::sentinel_state_handle()
{
	//持续接收对哨兵节点的订阅事件，直到主动stop或者连接断开后才会返回
	//与哨兵节点断开后并不意味着master连接不可用
	_sentinel->subscribe_event();
	while (!_is_state_stop)
	{
		_state_handler(SentinelEventType::SentinelSubscribeReturn, _master_node);
		//stop because of something wrong, try to recover it.
		std::this_thread::sleep_for(std::chrono::seconds(5));

		int count = _sentinel->check();
		if (count < 1)
		{
			continue;
		}
		auto node = _sentinel->get_master_by_name(_master_name);
		if (!node)
		{
			//unrecoverable error[fatal error], notify the upper...
			return 1;
		}
		if (_master_node == nullptr
			|| _master_node->ip != node->ip
			|| _master_node->port != node->port)
		{
			{
				std::lock_guard<std::mutex> lock(_master_mutex);
				_master_node = node;
			}
			//new master node...
			auto task = std::make_shared<SfTask>();
			task->type = etMasterChange;
			//将消息发送到每一个线程
			for (int i = 0; i < _qu_group.size(); ++i)
			{
				std::lock_guard<std::mutex> lock(_qu_group[i]->mutex);
				_qu_group[i]->queue.push(task);
			}
		}
	}
	return 0;
}

void SentinelFactory::sentinel_event_handle(SentinelEventType evtype, std::shared_ptr<RedisNode> name, std::shared_ptr<RedisNode> master_name)
{
	//only handle the master change.
	if (name->role != RedisRoleType::Master)
	{
		//to do...
		return;
	}
	//回调函数，通知外部
	if (evtype == SentinelEventType::SwitchMaster)
	{
		std::lock_guard<std::mutex> lock(_master_mutex);
		_master_node = master_name;
		
	}
	_state_handler(evtype, _master_node);

	auto task = std::make_shared<SfTask>();
	task->type = etInvalidTaskType;
	//发送命令到内部的工作线程...
	if (evtype == SentinelEventType::PsDown)
	{
		//发送命令到cmd线程，测试master的连通性
		task->type = etCheckMaster;
	}
	else if (evtype == SentinelEventType::SsDown)
	{
		task->type = etCheckMaster;
	}
	else if (evtype == SentinelEventType::PoDown)
	{
		//暂停命令的发送，直到新的master可用
		task->type = etMasterObjDown;
	}
	else if (evtype == SentinelEventType::SoDown)
	{
		task->type = etMasterObjUp;
	}
	else if (evtype == SentinelEventType::FailoverEnd)
	{
	}
	else if (evtype == SentinelEventType::SwitchMaster)
	{
		//检查到新的master服务器
		task->type = etMasterChange;
	}
	//将消息发送到每一个线程
	for (int i = 0; i < _qu_group.size(); ++i)
	{
		std::lock_guard<std::mutex> lock(_qu_group[i]->mutex);
		_qu_group[i]->queue.push(task);
	}
}

SentinelFactory::~SentinelFactory()
{
}

NS_2
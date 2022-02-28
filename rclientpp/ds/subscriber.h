#ifndef RCPP_SUBSCRIBER_H
#define RCPP_SUBSCRIBER_H

#include "../core/rclient.h"
#include "rd.h"
#include <initializer_list>
#include <functional>
#include <type_traits>

NS_1

class Subscriber : public Rd
{
public:
	using SubsHandleFunc = std::function<void(std::shared_ptr<std::string>, std::shared_ptr<std::string>)>;
	using SubsHandleFuncP = std::function<void(std::shared_ptr<std::string>, std::shared_ptr<std::string>, std::shared_ptr<std::string>)>;
	Subscriber(std::shared_ptr<RClient> connection);

	int subscribe(std::initializer_list<std::string> channels);
	int p_subscribe(std::initializer_list<std::string> channels);

	void regist_p_handle(SubsHandleFuncP);
	void regist_handle(SubsHandleFunc);

	int recv_msg();
	int recv_p_msg();
	//int recv_msg(SubsHandleFuncP func);
	void stop_recv();

private:
	int handle_publish(std::shared_ptr<BaseValue> ptr, std::shared_ptr<std::string> channel, std::shared_ptr<std::string> message);
	int handle_p_publish(std::shared_ptr<BaseValue> ptr, std::shared_ptr<std::string> pattern, std::shared_ptr<std::string> channel, std::shared_ptr<std::string> message);

	int handle_subscribe_res(const std::string& cmd);

	int do_recv_msg(bool pattern);

private:
	bool _is_stop{true};
	SubsHandleFunc _func;
	SubsHandleFuncP _funcp;
};


NS_2

#endif
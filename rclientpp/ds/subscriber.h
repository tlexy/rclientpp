#ifndef RCPP_SUBSCRIBER_H
#define RCPP_SUBSCRIBER_H

#include "rd.h"
#include <initializer_list>
#include <functional>

NS_1

class Subscriber : public Rd
{
public:
	using SubsHandleFunc = std::function<void(std::shared_ptr<std::string>, std::shared_ptr<std::string>)>;
	Subscriber(std::shared_ptr<RClient> connection);

	int subscribe(std::initializer_list<std::string> channels);

	int recv_msg(SubsHandleFunc func);
	void stop_recv();

private:
	int handle_publish(std::shared_ptr<BaseValue>ptr, std::shared_ptr<std::string> channel, std::shared_ptr<std::string> message);

private:
	bool _is_stop{true};
	SubsHandleFunc _func;
};

NS_2

#endif
#ifndef RCPP_RD_PUBLISH_H
#define RCPP_RD_PUBLISH_H

#include "rd.h"
#include <initializer_list>

NS_1

class Publisher : public Rd
{
public:
	Publisher(std::shared_ptr<RClient> connection);

	int publish(const std::string& chl, const std::string& msg);

	~Publisher();

};

NS_2

#endif
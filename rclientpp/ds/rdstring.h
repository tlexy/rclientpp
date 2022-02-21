#ifndef RCPP_RD_STRING_H
#define RCPP_RD_STRING_H

#include "rd.h"
#include <memory>

NS_1

class RdString : public Rd
{
public:
    explicit RdString(std::shared_ptr<RClient> connection);

	//retval: 0 for succeed
	//millisec is privilege
	int set(const std::string& key, const std::string& value,
		uint64_t seconds = 0, uint64_t millisec = 0, const std::string& option = "") const;

	std::shared_ptr<BaseValue> get(const std::string& key) const;

private:

};

NS_2

#endif
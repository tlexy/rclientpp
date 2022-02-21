#ifndef RCPP_RD_HASH_H
#define RCPP_RD_HASH_H

#include "rd.h"
#include <memory>
#include <initializer_list>

NS_1

class RdHash : public Rd
{
public:
	explicit RdHash(const std::shared_ptr<RClient>& connection);

	std::size_t hset(const std::string& key, const std::string& field, const std::string& val) const;
	std::shared_ptr<RedisValue> hget(const std::string& key, const std::string& field) const;
	std::shared_ptr<RedisValue> hmget(const std::string& key, std::initializer_list<std::string> fields) const;

    static bool hexists(const std::string& key, const std::string& field);
    static int hdel(const std::string& key, const std::string& field);
    static int hlen(const std::string& key, const std::string& field = "");


private:

};

NS_2

#endif
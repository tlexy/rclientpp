#ifndef RCPP_BASE_PARSER_H
#define RCPP_BASE_PARSER_H

#include "../core/rclient_def.h"
#include "../core/rclient_buffer.h"
#include <memory>
#include <string>
#include <list>

NS_1

class BaseParser
{
public:
	explicit BaseParser(ParserType type);
	BaseParser(const BaseParser&) = delete;
	BaseParser(BaseParser&&) = delete;
	BaseParser& operator=(const BaseParser&) = delete;
	BaseParser& operator=(BaseParser&&) = delete;
    virtual ~BaseParser() = default;
    
	virtual ParserType type2();

	/*
	* 0: successfully
	* < 0: error 
	*/
	virtual int parse(std::shared_ptr<RClientBuffer> raw_resp, std::shared_ptr<BaseValue>& result) = 0;

	int get_aggregate_len(const std::shared_ptr<RClientBuffer>&) const;
	int get_len_string(const std::shared_ptr<RClientBuffer>&, std::size_t len, std::string& outstr) const;

	int get_normal_string(const std::shared_ptr<RClientBuffer>& bufptr, std::string& out) const;

	int process_blob_string(const std::shared_ptr<RClientBuffer>&, std::string& outstr);
	int process_simple_string(const std::shared_ptr<RClientBuffer>&, std::string& outstr) const;
	int process_simple_error(const std::shared_ptr<RClientBuffer>&, std::string& outstr) const;
	int process_blob_error(const std::shared_ptr<RClientBuffer>&, std::string& outstr);
	int process_number(const std::shared_ptr<RClientBuffer>&, int64_t&) const;
	int process_big_number(const std::shared_ptr<RClientBuffer>&, std::string& outstr) const;
    static int process_nil(const std::shared_ptr<RClientBuffer>&);
	int process_double(const std::shared_ptr<RClientBuffer>&, long double&) const;
	int process_bool(const std::shared_ptr<RClientBuffer>&, bool&) const;
	int process_verbatim_string(const std::shared_ptr<RClientBuffer>&, std::string& outstr);

private:
	inline void move_item(const std::shared_ptr<RClientBuffer>&, std::size_t itemlen) const;

protected:
	int parse_array(const std::shared_ptr<RClientBuffer>& bufptr, int amounts, std::shared_ptr<RedisComplexValue>& result);

//public:
//	std::list<std::shared_ptr<RedisValue>> results;

protected:
	ParserType _type;
	std::string _scrlf;
};

NS_2

#endif
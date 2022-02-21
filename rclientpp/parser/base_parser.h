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
	BaseParser(ParserType type);

	virtual ParserType type2();

	/*
	* 0: successfully
	* < 0: error 
	*/
	virtual int parse(std::shared_ptr<RClientBuffer> raw_resp, std::shared_ptr<BaseValue>& result) = 0;

	int get_aggregate_len(std::shared_ptr<RClientBuffer>);
	int get_len_string(std::shared_ptr<RClientBuffer>, int len, std::string& outstr);

	int get_normal_string(std::shared_ptr<RClientBuffer> bufptr, std::string& out);

	int process_blob_string(std::shared_ptr<RClientBuffer>, std::string& outstr);
	int process_simple_string(std::shared_ptr<RClientBuffer>, std::string& outstr);
	int process_simple_error(std::shared_ptr<RClientBuffer>, std::string& outstr);
	int process_blob_error(std::shared_ptr<RClientBuffer>, std::string& outstr);
	int process_number(std::shared_ptr<RClientBuffer>, int64_t&);
	int process_big_number(std::shared_ptr<RClientBuffer>, std::string& outstr);
	int process_nil(std::shared_ptr<RClientBuffer>);
	int process_double(std::shared_ptr<RClientBuffer>, long double&);
	int process_bool(std::shared_ptr<RClientBuffer>, bool&);
	int process_verbatim_string(std::shared_ptr<RClientBuffer>, std::string& outstr);

private:
	inline void move_item(std::shared_ptr<RClientBuffer>, int itemlen);

protected:
	int parse_array(std::shared_ptr<RClientBuffer> bufptr, int amounts, std::shared_ptr<RedisComplexValue>& result);

//public:
//	std::list<std::shared_ptr<RedisValue>> results;

protected:
	ParserType _type;
	std::string _scrlf;
};

NS_2

#endif
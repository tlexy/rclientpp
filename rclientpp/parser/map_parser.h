#ifndef RCPP_MAP_PARSER_H
#define RCPP_MAP_PARSER_H

#include "base_parser.h"


NS_1

class MapParser : public BaseParser
{
public:
	MapParser();

	/*
	* 0: successfully
	* < 0: error 
	*/
	virtual int parse(std::shared_ptr<RClientBuffer> raw_resp, std::shared_ptr<BaseValue>& result);

};

NS_2

#endif
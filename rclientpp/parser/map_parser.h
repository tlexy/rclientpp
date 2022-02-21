#ifndef RCPP_MAP_PARSER_H
#define RCPP_MAP_PARSER_H

#include "base_parser.h"


NS_1

class MapParser final : public BaseParser
{
public:
	MapParser();

	/*
	* 0: successfully
	* < 0: error 
	*/
    int parse(std::shared_ptr<RClientBuffer> raw_resp, std::shared_ptr<BaseValue>& result) override;

};

NS_2

#endif
#ifndef RCPP_ARRAY_PARSER_H
#define RCPP_ARRAY_PARSER_H

#include "base_parser.h"


NS_1

class ArrayParser final : public BaseParser
{
public:
	ArrayParser();

	/*
	* 0: successfully
	* < 0: error 
	*/
    int parse(std::shared_ptr<RClientBuffer> raw_resp, std::shared_ptr<BaseValue>& result) override;

};

NS_2

#endif
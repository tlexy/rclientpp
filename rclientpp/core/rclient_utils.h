#ifndef RCPP_UTILS_H
#define RCPP_UTILS_H

#include <vector>
#include <string>

namespace rcpp {

	bool is_num(const char* str, int len);
	bool is_digit(const char* str, int len);
	void split(const std::string& text, const std::string& splitter, std::vector<std::string>& vecs);
}

#endif
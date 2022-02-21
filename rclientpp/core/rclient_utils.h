#ifndef RCPP_UTILS_H
#define RCPP_UTILS_H

namespace rcpp {

	bool is_num(const char* str, int len);
	bool is_digit(const char* str, int len);
}

#endif
#include "rclient.h"

namespace rcpp {

	bool is_num(const char* str, const std::size_t len)
	{
		if (len < 1 || (len > 1 &&str[0] == '0'))
		{
			return false;
		}
		for (auto i = 0; i < len; ++i)
		{
			if (str[i] < '0' || str[i] > '9')
			{
				return false;
			}
		}
		return true;
	}

	bool is_digit(const char* str, const std::size_t len)
	{
		bool dot = false;
		if (len < 1 || str[0] == '0')
		{
			return false;
		}
		for (auto i = 0; i < len; ++i)
		{
			if (str[i] == '.')
			{
				if (!dot)
				{
					dot = true;
				}
				else
				{
					return false;
				}
			}
			else if (str[i] < '0' || str[i] > '9')
			{
				return false;
			}
		}
		return true;
	}
}
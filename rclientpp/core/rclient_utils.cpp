#include "rclient.h"

namespace rcpp {

	bool is_num(const char* str, int len)
	{
		if (len < 1 || (len > 1 &&str[0] == '0'))
		{
			return false;
		}
		for (int i = 0; i < len; ++i)
		{
			if (str[i] < '0' || str[i] > '9')
			{
				return false;
			}
		}
		return true;
	}

	bool is_digit(const char* str, int len)
	{
		bool dot = false;
		if (len < 1 || str[0] == '0')
		{
			return false;
		}
		for (int i = 0; i < len; ++i)
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
	
	void split(const std::string& text, const std::string& splitter, std::vector<std::string>& vecs)
	{
		size_t offset = 0;
		size_t pos = text.find(splitter);
		while (pos != std::string::npos)
		{
			if (pos - offset > 0)
			{
				vecs.push_back(text.substr(offset, pos - offset));
			}
			offset = pos + splitter.size();
			pos = text.find(splitter, offset);
		}
		if (offset < text.size())
		{
			vecs.push_back(text.substr(offset));
		}
	}
}
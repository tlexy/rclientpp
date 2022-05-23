
#include "base_parser.h"
#include <algorithm>
#include "../core/rclient_utils.h"

NS_1


BaseParser::BaseParser(ParserType type)
	:_type(type),
	_scrlf(std::string("\r\n"))
{}

ParserType BaseParser::type2()
{
	return _type;
}

int BaseParser::get_aggregate_len(std::shared_ptr<RClientBuffer> bufptr)
{
	std::string str_len;
	int ret = get_normal_string(bufptr, str_len);
	if (ret < 0)
	{
		return ret;
	}
	if (!is_num(str_len.c_str(), str_len.size()))
	{
		return NOT_A_NUMBER;
	}
	int len = std::stoi(str_len);
	//move_item(bufptr, str_len.size());
	return len;
}

inline void BaseParser::move_item(std::shared_ptr<RClientBuffer> bufptr, int itemlen)
{
	bufptr->has_read(itemlen + _scrlf.size());
}

//will return NEED_MORE_DATA
int BaseParser::get_len_string(std::shared_ptr<RClientBuffer> bufptr, int len, std::string& outstr)
{
	if (len == 0)
	{
		outstr = "";
		return 0;
	}
	if (bufptr->readable_size() > len + 1)
	{
		char* text = bufptr->read_ptr();
		if (text[len] == '\r')
		{
			outstr = std::string(bufptr->read_ptr(), len);
			move_item(bufptr, len);
			return 0;
		}
		return PARSE_FORMAT_ERROR;
	}
	else
	{
		return NEED_MORE_DATA;
	}

}

int BaseParser::process_blob_string(std::shared_ptr<RClientBuffer> bufptr, std::string& outstr)
{
	char* text = bufptr->read_ptr();
	if (text[0] != '$')
	{
		return PARSE_FORMAT_ERROR;
	}
	if (text[1] == '-')
	{
		//RESP 2 NIL_VALUE
		outstr = std::string("-");
		move_item(bufptr, 3);
		return NIL_VALUE;
	}
	bufptr->has_read(1);//symbol '$'
	int len = get_aggregate_len(bufptr);
	if (len < 0)
	{
		//bufptr->has_read(-1);
		return len;
	}
	int ret = get_len_string(bufptr, len, outstr);
	return ret;
}

//will return NEED_MORE_DATA
int BaseParser::get_normal_string(std::shared_ptr<RClientBuffer> bufptr, std::string& outstr)
{
	char* pos = std::search(bufptr->read_ptr(), bufptr->read_ptr() + bufptr->readable_size(),
		_scrlf.c_str(), _scrlf.c_str() + _scrlf.size());
	if (pos == bufptr->read_ptr() + bufptr->readable_size())
	{
		return NEED_MORE_DATA;
	}
	int len = pos - bufptr->read_ptr();
	int ret = get_len_string(bufptr, len, outstr);
	return ret;
}

int BaseParser::process_simple_string(std::shared_ptr<RClientBuffer> bufptr, std::string& outstr)
{
	char* text = bufptr->read_ptr();
	if (text[0] != '+')
	{
		return PARSE_FORMAT_ERROR;
	}
	bufptr->has_read(1);
	int ret = get_normal_string(bufptr, outstr);
	/*if (ret < 0)
	{
		bufptr->has_read(-1);
	}*/
	return ret;
}

int BaseParser::process_simple_error(std::shared_ptr<RClientBuffer> bufptr, std::string& outstr)
{
	char* text = bufptr->read_ptr();
	if (text[0] != '-')
	{
		return PARSE_FORMAT_ERROR;
	}
	bufptr->has_read(1);
	int ret = get_normal_string(bufptr, outstr);
	/*if (ret < 0)
	{
		bufptr->has_read(-1);
	}*/
	return ret;
}

int BaseParser::process_big_number(std::shared_ptr<RClientBuffer> bufptr, std::string& outstr)
{
	char* text = bufptr->read_ptr();
	if (text[0] != '(')
	{
		return PARSE_FORMAT_ERROR;
	}
	bufptr->has_read(1);
	int ret = get_normal_string(bufptr, outstr);
	/*if (ret < 0)
	{
		bufptr->has_read(-1);
	}*/
	return ret;
}

int BaseParser::process_blob_error(std::shared_ptr<RClientBuffer> bufptr, std::string& outstr)
{
	char* text = bufptr->read_ptr();
	if (text[0] != '!')
	{
		return PARSE_FORMAT_ERROR;
	}
	bufptr->has_read(1);
	int len = get_aggregate_len(bufptr);
	if (len < 0)
	{
		//bufptr->has_read(-1);
		return len;
	}
	int ret = get_len_string(bufptr, len, outstr);
	
	return ret;
}

int BaseParser::process_number(std::shared_ptr<RClientBuffer> bufptr, int64_t& number)
{
	char* text = bufptr->read_ptr();
	if (text[0] != ':')
	{
		return PARSE_FORMAT_ERROR;
	}
	bufptr->has_read(1);
	std::string strnum;
	int ret = get_normal_string(bufptr, strnum);
	if (ret == 0 && strnum.size() > 0)
	{
		bool flag = false;
		if (strnum[0] == '-')
		{
			flag = is_num(strnum.c_str() + 1, strnum.size() - 1);
		}
		else
		{
			flag = is_num(strnum.c_str(), strnum.size());
		}
		if (flag)
		{
			number = std::stoll(strnum);
			return 0;
		}
	}
	/*else
	{
		bufptr->has_read(-1);
	}*/
	return PARSE_FORMAT_ERROR;
}

int BaseParser::process_nil(std::shared_ptr<RClientBuffer> bufptr)
{
	char* text = bufptr->read_ptr();
	if (text[0] != '_')
	{
		return PARSE_FORMAT_ERROR;
	}
	if (bufptr->readable_size() >= 3)
	{
		if (text[1] == '\r' && text[2] == '\n')
		{
			bufptr->has_read(3);
			return 0;
		}
		return PARSE_FORMAT_ERROR;
	}
	else
	{
		return NEED_MORE_DATA;
	}
	return 0;
}

int BaseParser::process_bool(std::shared_ptr<RClientBuffer> bufptr, bool& flag)
{
	char* text = bufptr->read_ptr();
	if (text[0] != '#')
	{
		return PARSE_FORMAT_ERROR;
	}
	bufptr->has_read(1);
	std::string str;
	int ret = get_normal_string(bufptr, str);
	if (ret == 0 && str.size() == 1)
	{
		return str[0] == 't';
	}
	/*else
	{
		bufptr->has_read(-1);
	}*/
	return PARSE_FORMAT_ERROR;
}

int BaseParser::process_verbatim_string(std::shared_ptr<RClientBuffer> bufptr, /*std::string& type,*/ std::string& outstr)
{
	char* text = bufptr->read_ptr();
	if (text[0] != '=')
	{
		return PARSE_FORMAT_ERROR;
	}
	bufptr->has_read(1);
	int len = get_aggregate_len(bufptr);
	if (len < 0)
	{
		//bufptr->has_read(-1);
		return len;
	}
	
	if (bufptr->readable_size() < 4)
	{
		return PARSE_FORMAT_ERROR;
	}
	text = bufptr->read_ptr();
	if (text[3] != ':')
	{
		return PARSE_FORMAT_ERROR;
	}
	/*type = std::string(text, 3);
	bufptr->has_read(4);*/

	int ret = get_len_string(bufptr, len, outstr);
	/*if (ret < 0)
	{
		bufptr->has_read(-1);
	}*/
	return ret;

}

int BaseParser::process_double(std::shared_ptr<RClientBuffer> bufptr, long double& digit)
{
	char* text = bufptr->read_ptr();
	if (text[0] != ':')
	{
		return PARSE_FORMAT_ERROR;
	}
	bufptr->has_read(1);
	std::string strnum;
	int ret = get_normal_string(bufptr, strnum);
	if (ret == 0 && strnum.size() > 0)
	{
		bool flag = false;
		if (strnum[0] == '-')
		{
			flag = is_digit(strnum.c_str() + 1, strnum.size() - 1);
		}
		else
		{
			flag = is_digit(strnum.c_str(), strnum.size());
		}
		if (flag)
		{
			digit = std::stold(strnum);
			return 0;
		}
	}
	/*else
	{
		bufptr->has_read(-1);
	}*/
	return PARSE_FORMAT_ERROR;
}

int BaseParser::parse_array(std::shared_ptr<RClientBuffer> bufptr, int amounts, std::shared_ptr<RedisComplexValue>& result)
{
	std::string out_str;
	long double out_d;
	int64_t out_digit;
	bool out_b;
	int ret = 0;

	while (amounts > result->count)
	{
		if (bufptr->readable_size() <= 3)
		{
			return NEED_MORE_DATA;
		}
		char* text = bufptr->read_ptr();
		char c = text[0];
		switch (c)
		{
		case '$':
		{
			ret = process_blob_string(bufptr, out_str);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(std::move(out_str), ParserType::BlobString));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case ':':
		{
			ret = process_number(bufptr, out_digit);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(out_digit));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case '+':
		{
			ret = process_simple_string(bufptr, out_str);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(std::move(out_str), ParserType::SimpleString));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case '-':
		{
			ret = process_simple_error(bufptr, out_str);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(std::move(out_str), ParserType::SimpleError));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case ',':
		{
			ret = process_double(bufptr, out_d);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(out_d));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case '#':
		{
			ret = process_bool(bufptr, out_b);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(out_b));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case '=':
		{
			ret = process_verbatim_string(bufptr, out_str);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(out_str, ParserType::Verbatim));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case '_':
		{
			ret = process_nil(bufptr);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(ParserType::NilValue));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case '(':
		{
			ret = process_big_number(bufptr, out_str);
			if (ret == 0)
			{
				result->results.push_back(std::make_shared<RedisValue>(std::move(out_str), ParserType::BigNumber));
				++result->count;
			}
			else
			{
				return ret;
			}
		}
		break;
		case '*':
		{
			auto new_array = std::make_shared<RedisComplexValue>(ParserType::Array);
			bufptr->has_read(1);
			int new_arr_len = get_aggregate_len(bufptr);
			if (new_arr_len < 0)
			{
				return new_arr_len;
			}
			else if (new_arr_len > 0)
			{
				ret = parse_array(bufptr, new_arr_len, new_array);
				if (ret == 0)
				{
					result->results.push_back(new_array);
					++result->count;
				}
				else
				{
					return ret;
				}
			}
			else
			{
				result->results.push_back(new_array);
				++result->count;
			}
			
		}
		break;
		case '%':
		{
			auto new_map = std::make_shared<RedisComplexValue>(ParserType::Map);
			bufptr->has_read(1);
			int new_len = get_aggregate_len(bufptr);
			if (new_len < 0)
			{
				return new_len;
			}
			else if (new_len > 0)
			{
				ret = parse_array(bufptr, new_len * 2, new_map);
				if (ret == 0)
				{
					result->results.push_back(new_map);
					++result->count;
				}
				else
				{
					return ret;
				}
			}
			else
			{
				result->results.push_back(new_map);
				++result->count;
			}

		}
		break;
		case '~':
		{
			auto new_set = std::make_shared<RedisComplexValue>(ParserType::Set);
			bufptr->has_read(1);
			int new_len = get_aggregate_len(bufptr);
			if (new_len < 0)
			{
				return new_len;
			}
			else if (new_len > 0)
			{
				ret = parse_array(bufptr, new_len, new_set);
				if (ret == 0)
				{
					result->results.push_back(new_set);
					++result->count;
				}
				else
				{
					return ret;
				}
			}
			else
			{
				result->results.push_back(new_set);
				++result->count;
			}

		}
		break;
		case '>':
		{
			auto new_push = std::make_shared<RedisComplexValue>(ParserType::Push);
			bufptr->has_read(1);
			int new_len = get_aggregate_len(bufptr);
			if (new_len < 0)
			{
				return new_len;
			}
			else if (new_len > 0)
			{
				ret = parse_array(bufptr, new_len, new_push);
				if (ret == 0)
				{
					result->results.push_back(new_push);
					++result->count;
				}
				else
				{
					return ret;
				}
			}
			else
			{
				result->results.push_back(new_push);
				++result->count;
			}
		}
		break;
		default:
			return PARSE_FORMAT_ERROR;
			;
		}
	}
	return 0;
}

NS_2
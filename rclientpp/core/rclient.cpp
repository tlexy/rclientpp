#include "rclient.h"
#include "sock_utils.h"
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "async_socket_client.h"

NS_1

const static std::string CRLF = "\r\n";

int RClient::_read_timeout = 30000;

RClient::RClient(std::string ipstr, const int port)
	: _aclient(std::make_shared<AsyncSocketClient>()),
      _ipstr(std::move(ipstr)),
      _port(port),
      _bufptr(std::make_shared<RClientBuffer>(1024)),
      _map_parser(std::make_shared<MapParser>()),
      _array_parser(std::make_shared<ArrayParser>())
{
	_strerr.resize(128, '\0');
}

int RClient::connect(const std::string& username, const std::string& auth, int timeout_ms)
{
	const bool flag = _aclient->connect(_ipstr.c_str(), _port, timeout_ms);
	if (!flag)
	{
		return -1;
	}
	_sockfd = _aclient->sockfd();
	/*_sockfd = sockets::ConnectTcp(_ipstr.c_str(), _port, timeout_ms);
	if (_sockfd <= 0)
	{
		return TCP_CONNECTED_FAILED;
	}*/
	const int ret = _sock_param();
	if (ret != 0)
	{
		return ret;
	}
	std::string cmd = "AUTH ";
	cmd = cmd + username + " " + auth + CRLF;
	return do_connect(cmd);
}

int RClient::connect(const std::string& auth, const int timeout_ms)
{
	const bool flag = _aclient->connect(_ipstr.c_str(), _port, timeout_ms);
	if (!flag)
	{
		return -1;
	}
	_sockfd = _aclient->sockfd();
	/*_sockfd = sockets::ConnectTcp(_ipstr.c_str(), _port, timeout_ms);
	if (_sockfd <= 0)
	{
		return TCP_CONNECTED_FAILED;
	}*/
	const int ret = _sock_param();
	if (ret != 0)
	{
		return ret;
	}
	std::string cmd = "AUTH ";
	cmd += auth + CRLF;
	return do_connect(cmd);
}

int RClient::connect(const int timeout_ms)
{
	const bool flag = _aclient->connect(_ipstr.c_str(), _port, timeout_ms);
	if (!flag)
	{
		return -1;
	}
	_sockfd = _aclient->sockfd();
	/*_sockfd = sockets::ConnectTcp(_ipstr.c_str(), _port, timeout_ms);
	if (_sockfd <= 0)
	{
		return TCP_CONNECTED_FAILED;
	}*/
	return _sock_param();
}

int RClient::_sock_param() const
{
	/*int ret = sockets::SetRecvTimeout(_sockfd, _read_timeout);
	if (ret != 0)
	{
		return ret;
	}*/
	int ret = sockets::KeepAlive(_sockfd);
	if (ret != 0)
	{
		return ret;
	}
	ret = sockets::setNoDelay(_sockfd);
	return ret;
}

int RClient::do_connect(const std::string& cmd)
{
	printf("do connect with AUTH.\n");
	auto ret = command(cmd.c_str(), cmd.size());
	if (ret == cmd.size())
	{
		const auto ptr = get_results(ret);
		if (ret != 0)
		{
			return AUTH_FAILED;
		}
		if (ptr && ptr->is_ok())
		{
			printf("do connect with AUTH successfully\n");
			return 0;
		}

	    _get_error(ptr);
        return AUTH_FAILED;
        /*int len = sockets::Read(_sockfd, (void*)_strerr.c_str(), _strerr.size());
		if (len > 0)
		{
			if (_strerr[0] == '-')
			{
				return AUTH_FAILED;
			}
			return 0;
		}
		else
		{
			return TCP_TIMEOUT;
		}*/
	}
    return TCP_SEND_FAILED;
}

int RClient::get_error(std::string& errstr) const
{
	errstr = _strerr;
	return _err_code;
}

std::string RClient::strerror()
{
	return _strerr;
}

void RClient::set_read_timeout(const int millisec)
{
	_read_timeout = millisec;
}

size_t RClient::command(const char* cmd, const std::size_t len)
{
	if (len < 3)
	{
		_err_code = SYNTAX_ERROR;
		return SYNTAX_ERROR;
	}
	_bufptr->reset();
	size_t rest = _aclient->read(_bufptr->write_ptr(), _bufptr->writable_size());
	while (rest > 0)
	{
		rest = _aclient->read(_bufptr->write_ptr(), _bufptr->writable_size());
	}
	if (cmd[len - 2] == '\r' && cmd[len - 1] == '\n')
	{
		//_err_code = sockets::Write(_sockfd, cmd, len);;
		_err_code = static_cast<int>(_aclient->write(cmd, len, _read_timeout));
		return _err_code;
	}
	_err_code = SYNTAX_ERROR;
	return SYNTAX_ERROR;
}

size_t RClient::recv(void* buf, const int len) const
{
	//return sockets::Read(_sockfd, buf, len);
	return _aclient->read(static_cast<char*>(buf), len, _read_timeout);
}

size_t RClient::use_resp3()
{
	_err_code = 0;
	static std::string hell_cmd3 = "HELLO 3\r\n";
	auto ret = command(hell_cmd3.c_str(), hell_cmd3.size());
	if (ret == hell_cmd3.size())
	{
		memset((void*)_strerr.c_str(), 0x0, _strerr.size());
		_bufptr->reset();

		const auto ptr = get_results(ret);
		if (ret != 0)
		{
			return ret;
		}
		if (!ptr || ptr->value_type() != ParserType::Map)
		{
			int a = 0;
			_get_error(ptr);
		}
		return _err_code;
	}
	return TCP_SEND_FAILED;
}

void RClient::_get_error(const std::shared_ptr<BaseValue> ptr) const
{
	memset((void*)_strerr.c_str(), 0x0, _strerr.size());
	if (ptr && ptr->value_type() == ParserType::SimpleError)
	{
		const auto sptr = std::dynamic_pointer_cast<RedisValue>(ptr);
		const size_t min_size = sptr->str_val_.size() > _strerr.size() ? _strerr.size() : sptr->str_val_.size();
		memcpy((void*)_strerr.c_str(), sptr->str_val_.c_str(), min_size);
	}
}

std::shared_ptr<BaseValue> RClient::get_results(size_t& ret_code)
{
	ret_code = PARSE_FORMAT_ERROR;
	_bufptr->reset();
READ_DATA:
	//int len = sockets::Read(_sockfd, (void*)_bufptr->write_ptr(), _bufptr->writable_size());
	const auto len = _aclient->read(_bufptr->write_ptr(), _bufptr->writable_size(), _read_timeout);
	if (len <= 0)
	{
		ret_code = TCP_CONNECTION_ERROR;
		return nullptr;
	}
	_bufptr->has_written(len);
	const char* text = _bufptr->read_ptr();
	const size_t old_pos = _bufptr->get_read_off();
	std::shared_ptr<BaseValue> result = nullptr;
	int ret = 0;
	std::string outstr;
	if (text[0] == '$')
	{
		ret = _array_parser->process_blob_string(_bufptr, outstr);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(outstr, ParserType::BlobString);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '+')
	{
		ret = _array_parser->process_simple_string(_bufptr, outstr);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(outstr, ParserType::SimpleString);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '-')
	{
		ret = _array_parser->process_simple_error(_bufptr, outstr);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(outstr, ParserType::SimpleError);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == ':')
	{
		int64_t num;
		ret = _array_parser->process_number(_bufptr, num);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(num);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '_')
	{
		ret = _array_parser->process_nil(_bufptr);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(ParserType::NilValue);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == ',')
	{
		long double dl = 0;
		ret = _array_parser->process_double(_bufptr, dl);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(dl);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '#')
	{
		bool flag = false;
		ret = _array_parser->process_bool(_bufptr, flag);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(flag);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '!')
	{
		ret = _array_parser->process_blob_error(_bufptr, outstr);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(outstr, ParserType::BlobError);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '=')
	{
		ret = _array_parser->process_verbatim_string(_bufptr, outstr);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(outstr, ParserType::Verbatim);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '(')
	{
		ret = _array_parser->process_big_number(_bufptr, outstr);
		if (ret == 0)
		{
			result = std::make_shared<RedisValue>(outstr, ParserType::BigNumber);
		}
		else
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '*')
	{
		_bufptr->has_read(1);
		result = std::make_shared<RedisComplexValue>(ParserType::Array);
		ret = _array_parser->parse(_bufptr, result);
		if (ret != 0)
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '%')
	{
		_bufptr->has_read(1);
		result = std::make_shared<RedisComplexValue>(ParserType::Map);
		ret = _map_parser->parse(_bufptr, result);
		if (ret != 0)
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else if (text[0] == '~')
	{
		_bufptr->has_read(1);
		result = std::make_shared<RedisComplexValue>(ParserType::Set);
		ret = _array_parser->parse(_bufptr, result);
		if (ret != 0)
		{
			//ret_code = PARSE_FORMAT_ERROR;
		}
	}
	else
	{
		return nullptr;
	}
	//error
	_err_code = ret;
	if (ret == NEED_MORE_DATA)
	{
		_bufptr->resize(_bufptr->size() * 2);
		_bufptr->set_read_off(old_pos);
		goto READ_DATA;
	}

    if (ret < 0)
    {
        return nullptr;
    }

    ret_code = 0;
    return result;
}

int RClient::use_resp2()
{
	static std::string hell_cmd2 = "HELLO 2\r\n";
	return 0;
}

void RClient::close()
{
	if (_sockfd > 0)
	{
		sockets::Close(_sockfd);
		_sockfd = -1;
	}
}

NS_2
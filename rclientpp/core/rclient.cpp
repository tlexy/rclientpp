#include "rclient.h"
#include "sock_utils.h"
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "async_socket_client.h"
#include "rclient_utils.h"

NS_1

const static std::string CRLF = "\r\n";

const static std::string strConnectError = "connect to server failed";
const static std::string strSockParamError = "set tcp socket parameters failed";
const static std::string strAuthError = "AUTH failed";
const static std::string strTcpSendError = "tcp send failed";

RClient::RClient(const std::string& ipstr, int port)
	:_ipstr(ipstr),
	_port(port),
	_bufptr(std::make_shared<RClientBuffer>(1024*1024)),
	_map_parser(std::make_shared<MapParser>()),
	_array_parser(std::make_shared<ArrayParser>()),
	_aclient(std::make_shared<AsyncSocketClient>()),
	_is_connect(false)
{
	_strerr.resize(256, '\0');
}

int RClient::connect(const std::string& username, const std::string& auth, int timeoutms)
{
	_user = username;
	_pass = auth;
	/*bool flag = _aclient->connect(_ipstr.c_str(), _port, timeoutms);
	if (!flag)
	{
		return -1;
	}
	_is_connect = true;
	_sockfd = _aclient->sockfd();*/
		/*_sockfd = rcppsockets::ConnectTcp(_ipstr.c_str(), _port, timeoutms);
		if (_sockfd <= 0)
		{
			return TCP_CONNECTED_FAILED;
		}*/
	//int ret = _sock_param();
	int ret = connect(timeoutms);
	if (ret != 0)
	{
		return ret;
	}
	std::string cmd = "AUTH ";
	cmd = cmd + username + " " + auth + CRLF;
	return do_connect(cmd);
}

int RClient::connect(const std::string& auth, int timeoutms)
{
	_pass = auth;
	/*bool flag = _aclient->connect(_ipstr.c_str(), _port, timeoutms);
	if (!flag)
	{
		_strerr = strConnectError;
		return -1;
	}
	_is_connect = true;
	_sockfd = _aclient->sockfd();*/
			/*_sockfd = rcppsockets::ConnectTcp(_ipstr.c_str(), _port, timeoutms);
			if (_sockfd <= 0)
			{
				return TCP_CONNECTED_FAILED;
			}*/
	//int ret = _sock_param();
	int ret = connect(timeoutms);
	if (auth.size() == 0)
	{
		return ret;
	}
	if (ret != 0)
	{
		return ret;
	}
	std::string cmd = "AUTH ";
	cmd += auth + CRLF;
	return do_connect(cmd);
}

int RClient::connect(int timeoutms)
{
	bool flag = _aclient->connect(_ipstr.c_str(), _port, timeoutms);
	if (!flag)
	{
		_strerr = strConnectError;
		return -1;
	}
	_is_connect = true;
	_sockfd = _aclient->sockfd();
	/*_sockfd = rcppsockets::ConnectTcp(_ipstr.c_str(), _port, timeoutms);
	if (_sockfd <= 0)
	{
		return TCP_CONNECTED_FAILED;
	}*/
	return _sock_param();
}

int RClient::check_connection()
{
	if (!_is_connect)
	{
		return TCP_CONNECTION_ERROR;
	}
	int old_timeout = _read_timeout;
	_read_timeout = 1000;
	int ret = command("PING\r\n", 6);
	if (ret == 6)
	{
		auto ptr = get_results(ret);
		_read_timeout = old_timeout;
		if (ret == 0 && ptr)
		{
			return 0;
		}
		return -1;
	}
	return -1;
}

int RClient::reconnect(int timeoutms)
{
	_aclient = std::make_shared<AsyncSocketClient>();
	if (_user.size() == 0 && _pass.size() == 0)
	{
		return connect(timeoutms);
	}
	if (_pass.size() > 0 && _user.size() > 0)
	{
		return connect(_user, _pass, timeoutms);
	}
	return connect(_pass, timeoutms);
}

int RClient::_sock_param()
{
	/*int ret = rcppsockets::SetRecvTimeout(_sockfd, _read_timeout);
	if (ret != 0)
	{
		return ret;
	}*/
	int ret = rcppsockets::KeepAlive(_sockfd);
	if (ret != 0)
	{
		_strerr = strSockParamError;
		return ret;
	}
	ret = rcppsockets::setNoDelay(_sockfd);
	if (ret != 0)
	{
		_strerr = strSockParamError;
	}
	return ret;
}

int RClient::do_connect(const std::string& cmd)
{
	printf("do connect with AUTH.\n");
	int ret = command(cmd.c_str(), cmd.size());
	if (ret == cmd.size())
	{
		auto ptr = get_results(ret);
		if (ret != 0)
		{
			_strerr = strAuthError;
			return AUTH_FAILED;
		}
		if (ptr && ptr->is_ok())
		{
			printf("do connect with AUTH successfully\n");
			return 0;
		}
		else
		{
			_get_error(ptr);
			return AUTH_FAILED;
		}
		/*int len = rcppsockets::Read(_sockfd, (void*)_strerr.c_str(), _strerr.size());
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
	else
	{
		_strerr = strTcpSendError;
		return TCP_SEND_FAILED;
	}
}

int RClient::get_error(std::string& errstr)
{
	errstr = _strerr;
	return _err_code;
}

std::string RClient::strerror()
{
	return _strerr;
}

void RClient::set_error_str(const std::string& str)
{
	_strerr = str;
}

void RClient::set_read_timeout(int millisec)
{
	_read_timeout = millisec;
}

int RClient::command(const char* cmd, int len)
{
	if (!_is_connect)
	{
		return TCP_CONNECTION_ERROR;
	}
	if (len < 3)
	{
		_err_code = SYNTAX_ERROR;
		return SYNTAX_ERROR;
	}
	_bufptr->reset();
	int rest = _aclient->read(_bufptr->write_ptr(), _bufptr->writable_size());
	while (rest > 0)
	{
		rest = _aclient->read(_bufptr->write_ptr(), _bufptr->writable_size());
	}
	if (cmd[len - 2] == '\r' && cmd[len - 1] == '\n')
	{
		//_err_code = rcppsockets::Write(_sockfd, cmd, len);;
		_err_code = _aclient->write(cmd, len, _read_timeout);
		return _err_code;
	}
	_err_code = SYNTAX_ERROR;
	return SYNTAX_ERROR;
}

int RClient::recv(void* buf, int len)
{
	//return rcppsockets::Read(_sockfd, buf, len);
	return _aclient->read((char*)buf, len, _read_timeout);
}

RedisRoleType RClient::get_role()
{
	std::string cmd("INFO REPLICATION\r\n");
	int ret = command(cmd.c_str(), cmd.size());
	if (ret == cmd.size())
	{
		auto ptr = get_results(ret);
		if (ret != 0 || ptr == nullptr)
		{
			return RedisRoleType::Unknown;
		}
		if (ptr->value_type() != ParserType::Verbatim && ptr->value_type() != ParserType::BlobString)
		{
			return RedisRoleType::Unknown;
		}
		std::string str = ptr->get_string();
		if (str.find("# Replication") == std::string::npos)
		{
			return RedisRoleType::Unknown;
		}
		std::vector<std::string> vecs;
		rcpp::split(str, "\r\n", vecs);
		if (vecs.size() > 0)
		{
			for (int i = 0; i < vecs.size(); ++i)
			{
				if (vecs[i].find("role:") != std::string::npos)
				{
					if (vecs[i].find("master") != std::string::npos)
					{
						return RedisRoleType::Master;
					}
					else if (vecs[i].find("slave") != std::string::npos)
					{
						return RedisRoleType::Slave;
					}
				}
			}
		}
	}
	return RedisRoleType::Unknown;
}

int RClient::use_resp3()
{
	_err_code = 0;
	static std::string hell_cmd3 = "HELLO 3\r\n";
	int ret = command(hell_cmd3.c_str(), hell_cmd3.size());
	if (ret == hell_cmd3.size())
	{
		memset((void*)_strerr.c_str(), 0x0, _strerr.size());
		_bufptr->reset();

		auto ptr = get_results(ret);
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

void RClient::_get_error(std::shared_ptr<BaseValue> ptr)
{
	memset((void*)_strerr.c_str(), 0x0, _strerr.size());
	if (ptr && ptr->value_type() == ParserType::SimpleError)
	{
		auto sptr = std::dynamic_pointer_cast<RedisValue>(ptr);
		size_t min_size = sptr->str_val_.size() > _strerr.size() ? _strerr.size() : sptr->str_val_.size();
		memcpy((void*)_strerr.c_str(), sptr->str_val_.c_str(), min_size);
	}
}

int RClient::has_more_data() const
{
	return _bufptr->readable_size();
}

std::shared_ptr<BaseValue> RClient::do_parse(int& ret_code)
{
	ret_code = 0;
	if (_bufptr->readable_size() < 1)
	{
		return nullptr;
	}
	//parse data 
	char* text = _bufptr->read_ptr();
	size_t old_pos = _bufptr->get_read_off();
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
			if (ret == RCPPError::NIL_VALUE)
			{
				result = std::make_shared<RedisValue>(outstr, ParserType::NilValue);
			}
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
	else if (text[0] == '>')
	{
		_bufptr->has_read(1);
		result = std::make_shared<RedisComplexValue>(ParserType::Push);
		ret = _array_parser->parse(_bufptr, result);
		text = _bufptr->read_ptr();
		if (ret != 0)
		{
			int a = 1;
		}
	}
	else
	{
	ret_code = RESP_FORMAT_UNKNOWN;
		return nullptr;
	}
	//error
	_err_code = ret;
	ret_code = ret;
	if (ret == NEED_MORE_DATA)
	{
		_bufptr->set_read_off(old_pos);
	}
	return result;
}

std::shared_ptr<BaseValue> RClient::get_results(int& ret_code)
{
	ret_code = PARSE_FORMAT_ERROR;
	//_bufptr->reset();
	_bufptr->rearrange();
READ_DATA:
	//int len = rcppsockets::Read(_sockfd, (void*)_bufptr->write_ptr(), _bufptr->writable_size());
	int len = _aclient->read(_bufptr->write_ptr(), _bufptr->writable_size(), _read_timeout);
	if (len == 0)
	{
		ret_code = TCP_CONNECTION_ERROR;
		return nullptr;
	}
	else if (len == -1)
	{
		ret_code = TCP_TIMEOUT;
		return nullptr;
	}
	_bufptr->has_written(len);
	auto result = do_parse(ret_code);
	if (ret_code == NEED_MORE_DATA)
	{
		_bufptr->resize(_bufptr->size() * 2);
		goto READ_DATA;
	}
	else
	{
		return result;
	}
	/*
	char* text = _bufptr->read_ptr();
	size_t old_pos = _bufptr->get_read_off();
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
			ret_code = PARSE_FORMAT_ERROR;
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
	else if (text[0] == '>')
	{
		_bufptr->has_read(1);
		result = std::make_shared<RedisComplexValue>(ParserType::Push);
		ret = _array_parser->parse(_bufptr, result);
		text = _bufptr->read_ptr();
		if (ret != 0)
		{
			int a = 1;
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
	else if (ret < 0)
	{
		return nullptr;
	}
	else
	{
		ret_code = 0;
	}
	return result;
	*/
}

int RClient::use_resp2()
{
	static std::string hell_cmd2 = "HELLO 2\r\n";
	return 0;
}

bool RClient::is_connected()
{
	if (_sockfd > 0 && _is_connect)
	{
		return true;
	}
	return false;
}

void RClient::close()
{
	if (_sockfd > 0)
	{
		_is_connect = false;
		rcppsockets::Close(_sockfd);
		_sockfd = -1;
	}
}

NS_2
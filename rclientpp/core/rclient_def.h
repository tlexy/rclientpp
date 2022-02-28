#ifndef RCLIENT_DEF_H
#define RCLIENT_DEF_H

#include <stdint.h>
#include <string>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

/*
* RESP 2
* Types and Descriptions:
* Simple Strings: +
* Errors: - example "-Error message\r\n"
* Integers: :
* Bulk Strings: $
* Arrays: *
* Null elements in Arrays: 
		*3\r\n
		$3\r\n
		foo\r\n
		$-1\r\n null element
		$3\r\n
		bar\r\n
* Sending commands to a Redis Server: a RESP Array consisting of just Bulk Strings or Inline Commands
* Multiple commands and pipelining
*/

/*
* RESP 3
* Types and Descriptions:
* Blob string: "$11\r\nhelloworld\r\n" empty strig "$0\r\n\r\n"
* Simple string: "+hello world\r\n"
* Simple error: -ERR this is the error description<CR><LF>
* Number: :1234<CR><LF>
* Null: _\r\n
* Double: ",1.23\r\n" positive or negative infinity  ",inf\r\n" ",-inf\r\n"
* Boolean: #t\r\n and #f\r\n
* Blob error: !<length>\r\n<bytes>\r\n example "!21\r\nSYNTAX invalid syntax\r\n"
* Verbatim string: =15<CR><LF>txt:Some string<CR><LF> The fourth byte is always :
* Big number: "(3492890328409238509324850943850943825024385\r\n"
* Aggregate data types: <aggregate-type-char><numelements><CR><LF> ... numelements other types ...
* Array: start with "*" example "*3\r\n:1\r\n:2\r\n:3\r\n"
* or	*3<CR><LF>
		:1<CR><LF>
		:2<CR><LF>
		:3<CR><LF>
*
* Map type: with a % byte
		%2<CR><LF>
		+first<CR><LF>
		:1<CR><LF>
		+second<CR><LF>
		:2<CR><LF>
*
* Set reply: with a ~ byte
		~5<CR><LF>
		+orange<CR><LF>
		+apple<CR><LF>
		#t<CR><LF>
		:100<CR><LF>
		:999<CR><LF>
* 
* Attribute type:  the ability to report. How to do: When a client reads a reply and encounters an attribute type, it should read the attribute, and continue reading the reply
		|1<CR><LF>
			+key-popularity<CR><LF>
			%2<CR><LF>
				$1<CR><LF>
				a<CR><LF>
				,0.1923<CR><LF>
				$1<CR><LF>
				b<CR><LF>
				,0.0012<CR><LF>
		*2<CR><LF>
			:2039123<CR><LF>
			:9543892<CR><LF>
Attributes can appear anywhere before a valid part of the protocol identifying a given type, like in the following example:
		*3<CR><LF>
			:1<CR><LF>
			:2<CR><LF>
			|1<CR><LF>
				+ttl
				:3600
			:3<CR><LF>
* 
* Push type:  exactly like the Array type. However the first byte is > instead of *, and the first element of the array is always a String item(in RESP3 by the push types pubsub and monitor)
		>4<CR><LF>
		+pubsub<CR><LF> --push type
		+message<CR><LF> --sub message type (other sub types may be subscribe, unsubscribe, and so forth)
		+somechannel<CR><LF> -- channel
		+this is the message<CR><LF> --message
* 
*/

#define NS_1 namespace rcpp{
#define NS_2 }

NS_1

enum RCPPError
{
	RCPPERROR_BEGIN = 0,
	TCP_CONNECTED_FAILED = -1,
	AUTH_FAILED = -2,
	SYNTAX_ERROR = -3,
	TCP_SEND_FAILED = -4,
	TCP_TIMEOUT = -5,
	TCP_INTERNAL_ERROR = -6,
	HELLO_RESP_ERROR = -7,
	PARSE_FORMAT_ERROR = -8,
	RESP_FORMAT_UNKNOWN = -9,
	NOT_A_NUMBER = -10,
	PARSE_FORMAT_LEN_ERROR = -10,
	NIL_VALUE = -11,
	NEED_MORE_DATA = -12,
	TCP_CONNECTION_ERROR = -13,
	RCPPERROR_END
};

enum class ParserType
{
	NilValue,
	SimpleString,
	BlobString,
	Map,
	Array,
	Set,
	SimpleError,
	Number,
	Double,
	Boolean,
	Null,
	BlobError,
	Verbatim,
	BigNumber,
	Push,

};

enum class RedisRoleType
{
	Master,
	Slave,
	Sentinel,
	Unknown,
};

enum class SentinelEventType
{
	SsDown,
	PsDown,
	SoDown,
	PoDown,
	FailoverEnd,
	SwitchMaster,
	PSlave,
	SentinelSubscribeReturn, //订阅返回了，可能是出错
	ConnectNewMasterFailed,
	ConnectNewMasterSucceed,
	MasterObjDown,
};

class BaseValue
{
public:
	BaseValue(ParserType type)
		:type_(type)
	{}

	virtual int64_t get_number() = 0;
	virtual std::string get_string() = 0;

	ParserType value_type() const
	{
		return type_;
	}

	bool is_string() const
	{
		if (type_ == ParserType::SimpleString
			|| type_ == ParserType::BlobString
			|| type_ == ParserType::SimpleError
			|| type_ == ParserType::BlobError
			|| type_ == ParserType::Verbatim
			|| type_ == ParserType::BigNumber)
		{
			return true;
		}
		return false;
	}

	bool is_nil() const
	{
		if (type_ == ParserType::NilValue)
		{
			return true;
		}
		return false;
	}

	bool is_number() const
	{
		if (type_ == ParserType::Number)
		{
			return true;
		}
		return false;
	}

	bool is_digit() const
	{
		if (type_ == ParserType::Number
			|| type_ == ParserType::Double)
		{
			return true;
		}
		return false;
	}

	bool is_boolean()
	{
		if (type_ == ParserType::Boolean)
		{
			return true;
		}
		return false;
	}

	virtual bool is_ok() { return false; }

public:
	ParserType type_;
};

class RedisValue : public BaseValue
{
public:
	RedisValue(ParserType type)
		:BaseValue(type)
	{
	}
	RedisValue(int64_t val)
		:BaseValue(ParserType::Number)
	{
		u.int_val_ = val;
	}
	RedisValue(long double val)
		:BaseValue(ParserType::Double)
	{
		u.d_val_ = val;
	}

	RedisValue(bool val)
		:BaseValue(ParserType::Boolean)
	{
		u.boolean_val_ = val;
	}

	RedisValue(const std::string& val, ParserType type)
		:BaseValue(type),
		str_val_(val)
	{
	}

	RedisValue(const std::string&& val, ParserType type)
		:BaseValue(type),
		str_val_(val)
	{
	}

	virtual int64_t get_number()
	{
		return u.int_val_;
	}

	virtual std::string get_string()
	{
		return str_val_;
	}

	virtual bool is_ok()
	{
		if (value_type() == ParserType::SimpleString)
		{
			if (str_val_ == std::string("OK"))
			{
				return true;
			}
		}
		return false;
	}

public:
	union
	{
		int64_t int_val_;
		long double d_val_;
		bool boolean_val_;
	} u;
	std::string str_val_;
};

class RedisComplexValue : public BaseValue
{
public:
	RedisComplexValue(ParserType type)
		:BaseValue(type)
	{
	}
	virtual int64_t get_number()
	{
		return 0;
	}

	virtual std::string get_string()
	{
		return std::string("");
	}
	std::list<std::shared_ptr<BaseValue>> results;
	int count{0};
};

class Attributes
{
public:
	void add(const std::string& key, const std::string& val)
	{
		_maps[key] = val;
	}
	bool query(const std::string& key, std::string& value)
	{
		auto it = _maps.find(key);
		if (it != _maps.end())
		{
			value = it->second;
			return true;
		}
		return false;
	}
	//std::string get(const std::string& key);
	size_t size() const
	{
		return _maps.size();
	}
private:
	std::unordered_map<std::string, std::string> _maps;
};

std::shared_ptr<Attributes> to_attrs(std::shared_ptr<RedisComplexValue> ptr);
std::vector<std::shared_ptr<Attributes>> to_bulk_attrs(std::shared_ptr<RedisComplexValue> ptr);
std::string get_string(std::shared_ptr<BaseValue>);
int64_t get_number(std::shared_ptr<BaseValue>);

NS_2

#endif
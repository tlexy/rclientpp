#include "rclient_def.h"
#include "rclient_utils.h"

NS_1

std::shared_ptr<Attributes> to_attrs(std::shared_ptr<RedisComplexValue> ptr)
{
	if (ptr->count % 2 != 0)
	{
		return nullptr;
	}

	auto attr = std::make_shared<Attributes>();
	auto it = ptr->results.begin();
	int i = 0;
	for (; it != ptr->results.end(); ++it)
	{
		auto key_ptr = *it;
		if (++it != ptr->results.end())
		{
			// only simple type is allow
			auto val_ptr = *it;
			if (key_ptr->value_type() != ParserType::BlobString)
			{
				return nullptr;
			}
			if (val_ptr->value_type() == ParserType::Set
				|| val_ptr->value_type() == ParserType::Map
				|| val_ptr->value_type() == ParserType::Array)
			{
				return nullptr;
			}
			auto key1 = std::dynamic_pointer_cast<RedisValue>(key_ptr);
			auto val1 = std::dynamic_pointer_cast<RedisValue>(val_ptr);
			std::string value;
			if (val1->is_string())
			{
				value = val1->str_val_;
			}
			else if (val1->value_type() == ParserType::Number)
			{
				value = std::to_string(val1->u.int_val_);
			}
			else if (val1->value_type() == ParserType::Double)
			{
				value = std::to_string(val1->u.d_val_);
			}
			else if (val1->value_type() == ParserType::Boolean)
			{
				value = std::to_string(val1->u.boolean_val_ ? 1 : 0);
			}

			attr->add(key1->str_val_, value);
		}
	}
	return attr;
}

std::vector<std::shared_ptr<Attributes>> to_bulk_attrs(std::shared_ptr<RedisComplexValue> ptr)
{
	std::vector<std::shared_ptr<Attributes>> attris;
	auto it = ptr->results.begin();
	for (; it != ptr->results.end(); ++it)
	{
		if ((*it)->value_type() != ParserType::Array
			&& (*it)->value_type() != ParserType::Map)
		{
			//something error...
			return attris;
		}
		auto master_ptr = std::dynamic_pointer_cast<RedisComplexValue>(*it);
		auto attr = to_attrs(master_ptr);
		if (attr)
		{
			attris.push_back(attr);
		}
	}
	return attris;
}

std::string get_string(std::shared_ptr<BaseValue> ptr)
{
	if (ptr)
	{
		auto vptr = std::dynamic_pointer_cast<RedisValue>(ptr);
		if (vptr)
		{
			return vptr->str_val_;
		}
	}
	return "";
}

int64_t get_number(std::shared_ptr<BaseValue> ptr)
{
	if (!ptr)
	{
		return 0;
	}
	if (ptr->is_string())
	{
		std::string str_num = get_string(ptr);
		if (is_num(str_num.c_str(), str_num.size()))
		{
			return std::atoll(str_num.c_str());
		}
		return 0;
	}
	auto vptr = std::dynamic_pointer_cast<RedisValue>(ptr);
	if (vptr)
	{
		return vptr->u.int_val_;
	}
	return 0;
}

NS_2

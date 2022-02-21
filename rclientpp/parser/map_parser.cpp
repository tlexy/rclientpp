
#include "map_parser.h"
#include <algorithm>

NS_1

MapParser::MapParser()
	:BaseParser(ParserType::Map)
{}

int MapParser::parse(std::shared_ptr<RClientBuffer> raw_resp, std::shared_ptr<BaseValue>& result)
{
	//size_t old_pos = raw_resp->get_read_off();
	int len = get_aggregate_len(raw_resp);
	if (len < 0)
	{
		//raw_resp->set_read_off(old_pos);
		return len;
	}
	//auto result = std::make_shared<RedisComplexValue>(ParserType::Map);
	auto complex_result = std::dynamic_pointer_cast<RedisComplexValue>(result);
	int ret = parse_array(raw_resp, len*2, complex_result);
	if (ret < 0)
	{
		//raw_resp->set_read_off(old_pos);
		return ret;
	}
	
	return 0;
}

NS_2
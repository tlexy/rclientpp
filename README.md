Supported platform: Windows and Linux
--------------

What is rclientpp?
--------------

rclientpp is a C++ client for redis. It doesn't rely on any other c or c++ redis client.

Specifications

* Easy to use.
* C++ 11 require only.
* Use select() to support connect/read/write timeout interface. Both windows and linux
* RESP 2 and RESP 3.
* Attribute type is not supported now.

Building rclientpp
--------------

### Windows
Use CMake with Visual Studio (2019 or higher is recommended)
Two files will be generated, librclientpp and rclienpp-maind.
rclienpp-maind will be located in the bin directory and the librclientpp will located in libs directory.

### Linux
Use CMake. follow the steps:
```
    > git clone xxx
    > cd rclientpp
    > mkdir build
    > cd build
    > cmake ..
    > make
```
Two files will be generated, librclientpp and rclienpp-maind.
rclienpp-maind will be located in the bin directory and the librclientpp will located in libs directory.

Tutorial
--------------
### Connect to redis server
```
std::string ip("81.71.72.73");
int port = 6379;

RClient* client = new RClient(ip, port);
int ret = client->connect("123456");
if (ret != 0)
{
	std::cout << "Err: " << client->strerror() << std::endl;
}
//transfer to RESP 3
ret = client->use_resp3();
if (ret != 0)
{
	return;
}
```
### Send command and wait for responses
```
std::string ip("81.71.72.73");
int port = 6379;

auto client = std::make_shared<RClient>(ip, port);
int ret = client->connect("123456");
if (ret != 0)
{
	std::cout << "Err: " << client->strerror() << std::endl;
}
//transfer to RESP 3
ret = client->use_resp3();
if (ret != 0)
{
	return;
}

std::string cmd = "sadd test_set 101 102\r\n"
int written_len = _client->command(cmd.c_str(), cmd.size());
if (written_len != cmd.size())
{
    //send error
	return;
}
int ret_code = 0;
auto result_ptr = _client->get_results(ret_code);
```
* In the code above, ret_code means whether an unexpected error is happening, such recv timeout or the responese data format is error. ret_code == 0 means no error.
If result_ptr is not empty and the ret_code is not zero, it also will be and error because of your cmd syntax error, you have to estimate what kind of result has been returned.

* result_ptr will be one of those[RedisValue or RedisComplexValue].
```
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
}
```

* RedisValue and RedisComplexValue have common super class: BaseValue

### Use redis structures
Right now, only a few ops are supported

```
std::string ip("81.71.72.73");
int port = 6379;

auto client = std::make_shared<RClient>(ip, port);
int ret = client->connect("123456");
if (ret != 0)
{
	std::cout << "Err: " << client->strerror() << std::endl;
}

ret = client->use_resp3();
if (ret != 0)
{
	return;
}
auto set_client = std::make_shared<RdSet>(client);
int count = set_client->sadd("test_set", { 10, 9, 8 });
std::cout << "set count: " << count << std::endl;
auto results = set_client->smembers("test_set");
if (results)
{
	if (results->value_type() != ParserType::Set
		&& results->value_type() != ParserType::Array)
	{
		return;
	}
	std::cout << "values:";
	auto ptr = std::dynamic_pointer_cast<RedisComplexValue>(results);
	auto it = ptr->results.begin();
	for (; it != ptr->results.end(); ++it)
	{
		if ((*it)->is_string())
		{
			auto ptr = std::dynamic_pointer_cast<RedisValue>(*it);
			std::cout << " " << ptr->str_val_;
		}
	}
	std::cout << std::endl;
}
bool flag = set_client->is_member("test_set", 2);
std::cout << "flag:" << flag << std::endl;

```

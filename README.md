Supported platform: Windows and Linux
--------------

What is rclientpp?
--------------

rclientpp is C++ client for redis. It didn't rely on any other c or c++ redis client.

Specifications

* Easy to use.
* C++ 11 require only.
* Use select() to support connect/read/write timeout interface. Both windows and linux
* Unix system maybe unsupported, because of the author never been use the any of unix system.
* RESP 2 and RESP 3.
* Attribute type is not supported now.

Building rclientpp
--------------

### Windows
Just use visual studio to open the rclientpp.sln, and then to compile the projects. visual stdio 2019 community is recommended.

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
else
{
	std::cout << "connect successfully..." << std::endl;
}

//transfer to RESP 3
ret = client->use_resp3();
if (ret != 0)
{
	return 1;
}
std::string cmd = "sadd test_set 101 102\r\n";
int written_len = client->command(cmd.c_str(), cmd.size());
if (written_len != cmd.size())
{
	//send error
	return 1;
}	
int ret_code = 0;
auto result_ptr = client->get_results(ret_code);
if (result_ptr)
{
	if (result_ptr->value_type() == ParserType::Number)
	{
		 auto ptr = std::dynamic_pointer_cast<RedisValue>(result_ptr);
		std::cout << "result : " << ptr->u.int_val_ << std::endl;
	}
}	
else
{
	std::cout << "get results error: " << ret_code << std::endl;
}

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

And so for, you have learn all that how to use rclientpp to connect and communicate with redis server. 
Also, you could use the advanced data structure the communicate to the redis server. But only few data structure and few operations are supported now, we welcome you to add more data structures and operations to this module.

### Use redis advanced structures
Right now, only few op is supported

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



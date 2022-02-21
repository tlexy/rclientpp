#ifndef RCPP_CIRCLE_BUFFER_H
#define RCPP_CIRCLE_BUFFER_H

#include <stdint.h>
#include <stddef.h>


/// <summary>
/// a buffer for redis client
/// </summary>
class RClientBuffer
{
public:
	RClientBuffer(size_t init_size);

	char* enable_size(size_t size);
	char* write_ptr();
	char* read_ptr();
	void reset();
	void resize(size_t newcap);

	size_t get_read_off();
	size_t get_write_off();

	void set_read_off(size_t);
	void set_write_off(size_t);

	void has_written(size_t size);
	void has_read(size_t size);

	size_t write(const char* buf, size_t len);
	size_t read(char* buf, size_t len);

	size_t size();

	size_t get_step();
	void set_step(size_t);

	size_t usable_size();
	size_t writable_size();
	size_t readable_size();

	~RClientBuffer();

private:
	void rearrange();

private:
	char* _buffer{NULL};
	size_t _read_pos{0};
	size_t _write_pos{0};
	size_t _step_pos{ 0 };
	size_t _size{0};
};

#endif

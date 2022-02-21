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
	RClientBuffer(const RClientBuffer&) = delete;
	RClientBuffer(RClientBuffer&&) = delete;
	RClientBuffer& operator=(const RClientBuffer&) = delete;
	RClientBuffer& operator=(RClientBuffer&&) = delete;
	~RClientBuffer();

	char* enable_size(size_t size);
	char* write_ptr() const;
	char* read_ptr() const;
	void reset();
	void resize(size_t newcap);

	size_t get_read_off() const;
	size_t get_write_off() const;

	void set_read_off(size_t);
	void set_write_off(size_t);

	void has_written(size_t size);
	void has_read(size_t size);

	size_t write(const char* buf, size_t len);
	size_t read(char* buf, size_t len) const;

	size_t size() const;

	size_t get_step() const;
	void set_step(size_t);

	size_t usable_size() const;
	size_t writable_size() const;
	size_t readable_size() const;

private:
	void rearrange();
	
	char* _buffer{nullptr};
	size_t _read_pos{0};
	size_t _write_pos{0};
	size_t _step_pos{ 0 };
	size_t _size{0};
};

#endif

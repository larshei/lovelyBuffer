FIFO and ring buffer in C
===

![Tests](https://github.com/larshei/ttdelay/workflows/Ruby/badge.svg?branch=master)

This is an implementation of a buffer in C, with the following attributes:

- Buffer with user defined data type.
- Manages read/write and mutliple buffer instances.
- FIFO or ring buffer mode for each buffer instance.
- Buffer instances / slots can be claimed and returned for repeated use.
- The buffer struct is hidden to the user.
- The buffer struct is managing data, not storing tha data itself.
- Memory usage and execution time not known yet, both expected to be very low.
  
Unit tests in Ceedling are included.

The Buffer System
===

The buffer system is designed to provide multiple FIFO-buffers and allow
asynchronous data collection / data processing.

After initialization, a buffer handle for each buffer is put on a stack and can
be claimed by the users program. The buffer can later be returned to the
system and claimed again.

Example Usage
===

## Configuration

Lets assume you have an SPI driver, that requires a buffer for both input and
output, and communication is based on exchanging bytes (= 8 bit length).

Configure the buffer system in `buf_buffer_config.h` like so:
```
#define DATA_TYPE          uint8_t
#define BUF_BUFFER_COUNT   2
```

## System Init

At startup, the buffer system needs to be initialized, using
```
if (buf_init_system() != BUF_OK) {
    // handle error
}
```

## Buffer Init

System init prepares the buffer system itself, not a specific buffer.

Each buffer must be initialized by itself. The buffers only manage data and
dont contain the data themselves, so the user needs to provide memory for
storage. Buffers will be used in the application based on their handle/pointer.

```
DATA_TYPE     buffer_array[100];
buf_buffer_t  buffer_handle;
```

To give the data storage information to the buffer system, create a
`buf_data_info_t` structure:
```
buf_data_info_t             buffer_config;
buffer_config.array         = (DATA_TYPE*) buffer_array;
buffer_config.element_count = BUFFER_SIZE;
buffer_config.element_size  = sizeof(DATA_TYPE);
```
and pass it to the system to retrieve and initialize a buffer:
```
buffer_handle = buf_claim_and_init_buffer((buf_data_info_t*)&buffer_config);
```
Make sure to not claim more buffers than you have configured in
`BUF_BUFFER_COUNT`, otherwise this function will return NULL instead of a buffer
handle.

## Buffer Mode

After buffer initialization, a buffer is used in FIFO mode. Users might want to
switch to ring buffer mode. The difference is:
- FIFO: A FIFO blocks further write access when the buffer is full, making sure
  all data that was put in the buffer is taken out.
- Ring Buffer: A ring buffer always allows write access, overwriting old data
  after a wrap around. Oftentimes, a ring buffer does not have a dedicated read
  pointer (but we do!)
```
uint8_t  result1 = buf_fifo_buffer_mode(buffer_handle);
uint8_t  result2 = buf_ring_buffer_mode(buffer_handle);
```
The functions change the internal state of the buffer. It may be changed at any
time. Returns `BUF_OK`.

Get the current buffer mode of either `BUF_RINGBUF` or `BUF_FIFOBUF` using
```
uint8_t  mode =  buf_get_buffer_mode(buffer_handle);
```

## Buffer Usage

Using a buffer is pretty straight forward adding and reading values, using
```
uint8_t    result = buf_add_element (buffer_handle, value);
DATA_TYPE  value  = buf_read_element(buffer_handle);
```

`buf_add_element` returns `BUF_OK` or `BUF_FULL`.
`buf_read_element` returns `value` or `NULL`.

The functions can be guarded by using
```
uint8_t  is_empty = buf_is_empty(buffer_handle);
uint8_t  is_full  = buf_is_full (buffer_handle);
```

## Returning Buffers that are no longer used

In a scenario with continuous data collection and asynchronous data processing
it may be a good idea to have the data collection task claim a buffer, fill it
and then pass it on to one of many other tasks. These tasks can then return the
buffer once its content has been processed.
```
uint8_t  result = buf_return_buffer(buffer_handle);
```
The function automtically checks, if the buffer_handle is a valid buffer
address and if the buffer is already on the stack or not. The return values are
`BUF_OK`, `BUF_DUPLICATE` (buffer already on stack), `BUF_NULL` (invalid handle).

# Changelog

2020-05-12: Initial Version

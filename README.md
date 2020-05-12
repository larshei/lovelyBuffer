Ring buffer in C
===

This is an implementation of a ring buffer in C, with the following attributes:

- Ring buffer with user defined data type.
- Manages read/write and mutliple buffer instances.
- Buffer instances / slots can be claimed and returned for repeated use.
- The buffer struct is hidden to the user.
- Data has to be provided by the user.
- Memory usage and execution time not known yet, both expected to be very low.
  
Unit tests in Ceedling are planned.

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
output, and communication is based on excahnging bytes (= 8 bit length).

Configure the buffer system in `ring_buffer_config.h` like so:
```
#define DATA_TYPE           uint8_t
#define RB_BUFFER_COUNT     2
```

## System Init

At startup, the buffer system needs to be initialized, using
```
if (rb_init_system() != RB_OK) {
    // handle error
}
```

## Buffer Init

System init prepares the buffer system itself, not a specific buffer.

Each buffer must be initialized by itself. The buffers only manage data and
dont contain the data themselves, so the user needs to provide memory for
storage. Buffers will be used in the application based on their handle/pointer.

```
uint8_t     buffer_array[100];
rb_buffer_t buffer_handle;
```

To give the data storage information to the buffer system, create a
`rb_data_info_t` structure:
```
rb_data_info_t buffer_config;
buffer_config.array = (DATA_TYPE*) buffer_array;
buffer_config.element_count = BUFFER_SIZE;
buffer_config.element_size  = sizeof(DATA_TYPE);
```
and pass it to the system to retrieve and initialize a buffer:
```
buffer_handle = rb_claim_and_init_buffer((rb_data_info_t*)&buffer_config);
```
Make sure to not claim more buffers than you have configured in
`RB_BUFFER_COUNT`, otherwise this function will return NULL instead of a buffer
handle.

## Buffer Usage

Using a buffer is pretty straight forward adding and reading values, using
```
result = rb_add_element(buffer_handle, value);
value  = rb_read_element(buffer_handle);
```

`rb_add_element` returns `RB_OK` or `RB_FULL`.
`rb_read_element` returns `value` or `NULL`.

The functions can be guarded by using
```
uint8_t is_empty = rb_is_empty(buffer_handle);
uint8_t is_full  = rb_is_full (buffer_handle);
```

## Returning Buffers that are no longer used

In a scenario with continuous data collection and asynchronous data processing
it may be a good idea to have the data collection task claim a buffer, fill it
and then pass it on to one of many other tasks. These tasks can then return the
buffer once its content has been processed.
```
result = rb_return_buffer(buffer_handle);
```
The function automtically checks, if the buffer_handle is a valid buffer
address and if the buffer is already on the stack or not. The return values are
`RB_OK`, `RB_DUPLICATE` (buffer already on stack), `RB_NULL` (invalid handle).

# Changelog

2020-05-12: Initial Version

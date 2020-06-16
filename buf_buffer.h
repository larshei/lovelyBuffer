#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include <stdint.h>
#include "buf_default_config.h"

// expose a pointer to buf_buffer_t, but hide its contents.
typedef struct buf_internal_buffer_t* buf_buffer_t;

typedef struct buf_data_info_t {
    DATA_TYPE* array;
    int element_size;
    int element_count;
} buf_data_info_t;

// === BUFFER SYSTEM ===
// --- System init, run before using any other function ---
uint8_t buf_init_system();

// === BUFFER SLOT MANAGEMENT ===
// --- Grab a buffer slot for your application. Returns buffer or NULL ---
buf_buffer_t buf_claim_buffer();

// --- Give a buffer slot back from your application. Returns BUF_OK or BUF_NULL ---
uint8_t buf_return_buffer(buf_buffer_t buffer);

// --- init a claimed buffer slot. Returns BUF_OK or BUF_NULL. ---
uint8_t buf_init_buffer(buf_buffer_t buffer, buf_data_info_t* data);

// --- claim a buffer slot and init, returns buffer or NULL. ---
buf_buffer_t buf_claim_and_init_buffer(buf_data_info_t* data);

// --- number of used buffer slots ---
uint8_t buf_get_used_buffer_slot_count();

// --- number of elements that can fit in the buffer ---
unsigned int buf_get_buffer_size(buf_buffer_t buffer);

unsigned int buf_claim_count();
unsigned int buf_return_count();


// === BUFFER CONTENT MANAGEMENT ===
uint8_t buf_reset_read(buf_buffer_t buffer);
uint8_t buf_get_buffer_mode(buf_buffer_t buffer);
uint8_t buf_fifo_buffer_mode(buf_buffer_t buffer);
uint8_t buf_ring_buffer_mode(buf_buffer_t buffer);

uint8_t buf_add_element (buf_buffer_t buffer, DATA_TYPE element);
DATA_TYPE buf_read_element (buf_buffer_t buffer);
uint8_t buf_is_empty(buf_buffer_t buffer);
uint8_t buf_is_full(buf_buffer_t buffer);
uint8_t buf_was_filled_once(buf_buffer_t buffer);

// === TEST FUNCTIONS ===
#ifdef TEST
void print_buffer(buf_buffer_t buffer);
#endif

enum {
    BUF_OK,
    BUF_FULL,
    BUF_EMPTY,
    BUF_NULL,
    BUF_DUPLICATE,
    BUF_RINGBUF,
    BUF_FIFOBUF
};

#ifndef NULL
#define NULL (void*)0
#endif

#endif // __RING_BUFFER_H

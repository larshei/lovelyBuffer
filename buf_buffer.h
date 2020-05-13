#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include <stdint.h>
#include "fifo_buffer_config.h"

// expose a pointer to fifo_buffer_t, but hide its contents.
typedef struct fifo_internal_buffer_t* fifo_buffer_t;

typedef struct fifo_data_info_t {
    DATA_TYPE* array;
    int element_size;
    int element_count;
} fifo_data_info_t;

// === BUFFER SYSTEM ===
// --- System init, run before using any other function ---
uint8_t fifo_init_system();

// === BUFFER SLOT MANAGEMENT ===
// --- Grab a buffer slot for your application. Returns buffer or NULL ---
fifo_buffer_t fifo_claim_buffer();

// --- Give a buffer slot back from your application. Returns FIFO_OK or FIFO_NULL ---
uint8_t fifo_return_buffer(fifo_buffer_t buffer);

// --- init a claimed buffer slot. Returns FIFO_OK or FIFO_NULL. ---
uint8_t fifo_init_buffer(fifo_buffer_t buffer, fifo_data_info_t* data);

// --- claim a buffer slot and init, returns buffer or NULL. ---
fifo_buffer_t fifo_claim_and_init_buffer(fifo_data_info_t* data);

// --- number of used buffer slots ---
uint8_t fifo_get_used_buffer_slot_count();

// --- number of elements that can fit in the buffer ---
unsigned int fifo_get_buffer_size(fifo_buffer_t buffer);

unsigned int fifo_claim_count();
unsigned int fifo_return_count();


// === BUFFER CONTENT MANAGEMENT ===
uint8_t fifo_add_element (fifo_buffer_t buffer, DATA_TYPE element);
DATA_TYPE* fifo_read_element (fifo_buffer_t buffer);
uint8_t fifo_is_empty(fifo_buffer_t buffer);
uint8_t fifo_is_full(fifo_buffer_t buffer);

// === TEST FUNCTIONS ===
#ifdef TEST
void print_buffer(fifo_buffer_t buffer);
#endif

enum {
    FIFO_OK,
    FIFO_FULL,
    FIFO_EMPTY,
    FIFO_NULL,
    FIFO_DUPLICATE
};

#ifndef NULL
#define NULL (void*)0
#endif

#endif // __RING_BUFFER_H

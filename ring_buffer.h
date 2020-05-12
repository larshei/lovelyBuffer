#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include <stdint.h>
#include "ring_buffer_config.h"

// expose a pointer to rb_buffer_t, but hide its contents.
typedef struct ring_buffer_t* rb_buffer_t;

typedef struct rb_data_info_t {
    DATA_TYPE* array;
    int element_size;
    int element_count;
} rb_data_info_t;

// === BUFFER SYSTEM ===
// --- System init, run before using any other function ---
uint8_t rb_init_system();

// === BUFFER SLOT MANAGEMENT ===
// --- Grab a buffer slot for your application. Returns buffer or NULL ---
rb_buffer_t rb_claim_buffer();

// --- Give a buffer slot back from your application. Returns RB_OK or RB_NULL ---
uint8_t rb_return_buffer(rb_buffer_t buffer);

// --- init a claimed buffer slot. Returns RB_OK or RB_NULL. ---
uint8_t rb_init_buffer(rb_buffer_t buffer, rb_data_info_t* data);

// --- claim a buffer slot and init, returns buffer or NULL. ---
rb_buffer_t rb_claim_and_init_buffer(rb_data_info_t* data);

// --- number of used buffer slots ---
uint8_t rb_get_used_buffer_slot_count();

// --- number of elements that can fit in the buffer ---
unsigned int rb_get_buffer_size(rb_buffer_t buffer);

unsigned int rb_claim_count();
unsigned int rb_return_count();


// === BUFFER CONTENT MANAGEMENT ===
uint8_t rb_add_element (rb_buffer_t buffer, DATA_TYPE element);
DATA_TYPE* rb_read_element (rb_buffer_t buffer);
uint8_t rb_is_empty(rb_buffer_t buffer);
uint8_t rb_is_full(rb_buffer_t buffer);

// === TEST FUNCTIONS ===
#ifdef TEST
void print_buffer(rb_buffer_t buffer);
#endif

enum {
    RB_OK,
    RB_FULL,
    RB_EMPTY,
    RB_NULL,
    RB_DUPLICATE
};

#ifndef NULL
#define NULL (void*)0
#endif

#endif // __RING_BUFFER_H

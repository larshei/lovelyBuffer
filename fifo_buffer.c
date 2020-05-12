#include "ring_buffer.h"
#include "string.h"

// --- PRIVATE STRUCTURE FOR A SINGLE BUFFER ---
typedef struct ring_buffer_t {
    uint8_t is_empty;
    uint8_t is_full;
    DATA_TYPE* data_array_first;
    DATA_TYPE* data_array_last;
    DATA_TYPE* read;
    DATA_TYPE* write;
} ring_buffer_t;

// --- HOLD MULTIPLE BUFFERS, organized by pointers on a stack ---
typedef struct buffer_system_t {
    unsigned int buffer_claim_count;
    unsigned int buffer_return_count;
    unsigned int buffer_stack_size;
    ring_buffer_t* buffer_stack[RB_BUFFER_COUNT];
    ring_buffer_t  buffers[RB_BUFFER_COUNT];
} buffer_system_t;
buffer_system_t rb_system;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * HELPER FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t ouf_of_bounds(rb_buffer_t buffer, DATA_TYPE* pointer) {
    if (pointer > buffer->data_array_last ) return 1;
    if (pointer < buffer->data_array_first) return 1;
    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MANAGE BUFFER INSTANCES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t rb_init_system() {
    rb_system.buffer_stack_size = RB_BUFFER_COUNT;
    rb_system.buffer_claim_count = 0;
    rb_system.buffer_return_count = 0;
    for (int i = 0 ; i < RB_BUFFER_COUNT ; i++) {
        rb_system.buffer_stack[i] = &rb_system.buffers[i];
    }
    return RB_OK;
}

rb_buffer_t rb_claim_buffer() {
    uint8_t stack_not_empty = rb_system.buffer_stack_size > 0;
    if (stack_not_empty){
        rb_system.buffer_claim_count++;
        return rb_system.buffer_stack[--rb_system.buffer_stack_size];
    }
    return NULL;
}

uint8_t rb_buffer_on_stack(rb_buffer_t buffer) {
    for (int i = 0; i < rb_system.buffer_stack_size ; i++) {
        if (&rb_system.buffers[i] == buffer)
            return RB_DUPLICATE;
    }
    rb_system.buffer_return_count++;
    return RB_OK;
}

uint8_t rb_return_buffer(rb_buffer_t buffer) {
    rb_buffer_t buffer_slot_address = &rb_system.buffers[0];
    if (rb_buffer_on_stack(buffer) == RB_DUPLICATE)
        return RB_DUPLICATE;

    for (int i = 0 ; i < RB_BUFFER_COUNT ; i++, buffer_slot_address++) {
        if (buffer_slot_address == buffer) {
            rb_system.buffer_stack[rb_system.buffer_stack_size++] = buffer;
            return RB_OK;
        }
    }
    return RB_NULL;
}

uint8_t rb_init_buffer(rb_buffer_t buffer, rb_data_info_t* data) {
    if (buffer == NULL) return RB_NULL;
    memset(buffer, 0, sizeof(ring_buffer_t));
    buffer->data_array_first = data->array;
    buffer->data_array_last  = data->array + (data->element_count - 1);
    buffer->read             = buffer->data_array_first;
    buffer->write            = buffer->data_array_first;
    buffer->is_empty         = 1;
    return RB_OK;
}

rb_buffer_t rb_claim_and_init_buffer(rb_data_info_t* data) {
    rb_buffer_t buffer;
    buffer = rb_claim_buffer();
    if (rb_init_buffer(buffer, data) == RB_NULL) {
        return NULL;
    } else {
        return buffer;
    }
}

uint8_t rb_get_used_buffer_slot_count() {
    return RB_BUFFER_COUNT - rb_system.buffer_stack_size;
}

// --- purely informational for debug / stats / tests ---
unsigned int rb_claim_count() {
    return rb_system.buffer_claim_count;
}

// --- purely informational for debug / stats / tests ---
unsigned int rb_return_count() {
    return rb_system.buffer_return_count;

}

// --- purely informational for debug / stats / tests ---
unsigned int rb_get_buffer_size(rb_buffer_t buffer) {
    unsigned int buffer_size = (*(buffer->data_array_last)  \
                             - *(buffer->data_array_first)) \
                             + 1;
    return buffer_size;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MANAGE BUFFER CONTENT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t rb_add_element (rb_buffer_t buffer, DATA_TYPE element) {
    if (buffer->is_full) return RB_FULL;

    *(buffer->write) = element;
    buffer->write++;
    buffer->is_empty = 0;

    if (ouf_of_bounds(buffer, buffer->write))
        buffer->write = buffer->data_array_first;

    if (buffer->write == buffer->read) buffer->is_full = 1;

    return RB_OK;
}

DATA_TYPE* rb_read_element (rb_buffer_t buffer) {
    if (buffer->is_empty) return NULL;

    DATA_TYPE* element = *(buffer->read);
    buffer->read++;
    buffer->is_full = 0;

    if (ouf_of_bounds(buffer, buffer->read))
        buffer->read = buffer->data_array_first;

    if (buffer->write == buffer->read) buffer->is_empty = 1;

    return element;
}

uint8_t rb_is_empty(rb_buffer_t buffer) {
    return buffer->is_empty;
}

uint8_t rb_is_full(rb_buffer_t buffer) {
    return buffer->is_full;
}

#ifdef TEST
void print_buffer(rb_buffer_t buffer) {
    unsigned int buffer_size = rb_get_buffer_size(buffer);
    for (int i = 0; i < buffer_size ; i++) {
        printf("%d ", buffer->data_array_first[i]);
        if (!(i+1 % 0))
            printf("\n");
    }
}
#endif

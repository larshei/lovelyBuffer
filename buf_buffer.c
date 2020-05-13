#include "buf_buffer.h"
#include "string.h"

// --- PRIVATE STRUCTURE FOR A SINGLE BUFFER ---
typedef struct buf_internal_buffer_t {
    uint8_t is_empty;
    uint8_t is_full;
    uint8_t allow_overwrite;
    uint8_t was_filled_once;
    DATA_TYPE* first_element;
    DATA_TYPE* last_element;
    DATA_TYPE* read;
    DATA_TYPE* write;
} buf_internal_buffer_t;

// --- HOLD MULTIPLE BUFFERS, organized by pointers on a stack ---
typedef struct buffer_system_t {
    unsigned int claim_count;
    unsigned int return_count;
    unsigned int stack_size;
    buf_internal_buffer_t* buffer_stack[BUF_BUFFER_COUNT];
    buf_internal_buffer_t  buffers[BUF_BUFFER_COUNT];
} buffer_system_t;
buffer_system_t buf_system;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * HELPER FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t ouf_of_bounds(buf_buffer_t buffer, DATA_TYPE* pointer) {
    if (pointer > buffer->last_element ) return 1;
    if (pointer < buffer->first_element) return 1;
    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MANAGE BUFFER INSTANCES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t buf_init_system() {
    buf_system.stack_size = BUF_BUFFER_COUNT;
    buf_system.claim_count = 0;
    buf_system.return_count = 0;
    for (int i = 0 ; i < BUF_BUFFER_COUNT ; i++) {
        buf_system.buffer_stack[i] = &buf_system.buffers[i];
    }
    return BUF_OK;
}

buf_buffer_t buf_claim_buffer() {
    uint8_t stack_not_empty = buf_system.stack_size > 0;
    if (stack_not_empty){
        buf_system.claim_count++;
        return buf_system.buffer_stack[--buf_system.stack_size];
    }
    return NULL;
}

uint8_t buf_buffer_on_stack(buf_buffer_t buffer) {
    for (int i = 0; i < buf_system.stack_size ; i++) {
        if (&buf_system.buffers[i] == buffer)
            return BUF_DUPLICATE;
    }
    buf_system.return_count++;
    return BUF_OK;
}

uint8_t buf_return_buffer(buf_buffer_t buffer) {
    buf_buffer_t buffer_slot_address = &buf_system.buffers[0];
    if (buf_buffer_on_stack(buffer) == BUF_DUPLICATE)
        return BUF_DUPLICATE;

    for (int i = 0 ; i < BUF_BUFFER_COUNT ; i++, buffer_slot_address++) {
        if (buffer_slot_address == buffer) {
            buf_system.buffer_stack[buf_system.stack_size++] = buffer;
            return BUF_OK;
        }
    }
    return BUF_NULL;
}

uint8_t buf_init_buffer(buf_buffer_t buffer, buf_data_info_t* data) {
    if (buffer == NULL) return BUF_NULL;
    memset(buffer, 0, sizeof(buf_internal_buffer_t));
    buffer->first_element = data->array;
    buffer->last_element  = data->array + (data->element_count - 1);
    buffer->read             = buffer->first_element;
    buffer->write            = buffer->first_element;
    buffer->is_empty         = 1;
    return BUF_OK;
}

buf_buffer_t buf_claim_and_init_buffer(buf_data_info_t* data) {
    buf_buffer_t buffer;
    buffer = buf_claim_buffer();
    if (buf_init_buffer(buffer, data) == BUF_NULL) {
        return NULL;
    } else {
        return buffer;
    }
}

uint8_t buf_get_used_buffer_slot_count() {
    return BUF_BUFFER_COUNT - buf_system.stack_size;
}

// --- purely informational for debug / stats / tests ---
unsigned int buf_claim_count() {
    return buf_system.claim_count;
}

// --- purely informational for debug / stats / tests ---
unsigned int buf_return_count() {
    return buf_system.return_count;

}

// --- purely informational for debug / stats / tests ---
unsigned int buf_get_buffer_size(buf_buffer_t buffer) {
    unsigned int buffer_size = (*(buffer->last_element)  \
                             - *(buffer->first_element)) \
                             + 1;
    return buffer_size;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MANAGE BUFFER CONTENT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t buf_reset_read(buf_buffer_t buffer);


uint8_t buf_get_buffer_mode(buf_buffer_t buffer) {
    uint8_t is_ring_buffer = buffer->allow_overwrite;
    if (is_ring_buffer) {
        return BUF_RINGBUF;
    } else {
        return BUF_FIFOBUF;
    }
}

uint8_t buf_fifo_buffer_mode(buf_buffer_t buffer) {
    buffer->allow_overwrite = 0;
    return BUF_OK;
}

uint8_t buf_ring_buffer_mode(buf_buffer_t buffer) {
    buffer->allow_overwrite = 1;
    return BUF_OK;
}

uint8_t buf_add_element (buf_buffer_t buffer, DATA_TYPE element) {
    if (buffer->is_full) {
        if (!(buffer->allow_overwrite)) {
            return BUF_FULL;
        }
    }

    *(buffer->write) = element;
    buffer->write++;
    buffer->is_empty = 0;

    if (ouf_of_bounds(buffer, buffer->write))
        buffer->write = buffer->first_element;

    if (buffer->write == buffer->read) buffer->is_full = 1;

    return BUF_OK;
}

DATA_TYPE* buf_read_element (buf_buffer_t buffer) {
    if (buffer->is_empty) return NULL;

    DATA_TYPE* element = *(buffer->read);
    buffer->read++;
    buffer->is_full = 0;

    if (ouf_of_bounds(buffer, buffer->read))
        buffer->read = buffer->first_element;

    if (buffer->write == buffer->read) buffer->is_empty = 1;

    return element;
}



uint8_t buf_is_empty(buf_buffer_t buffer) {
    return buffer->is_empty;
}

uint8_t buf_is_full(buf_buffer_t buffer) {
    return buffer->is_full;
}

#ifdef TEST
void print_buffer(buf_buffer_t buffer) {
    unsigned int buffer_size = buf_get_buffer_size(buffer);
    for (int i = 0; i < buffer_size ; i++) {
        printf("%d ", buffer->first_element[i]);
        if (!(i+1 % 10))
            printf("\n");
    }
}
#endif

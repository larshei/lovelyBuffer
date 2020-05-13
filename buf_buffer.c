#include "fifo_buffer.h"
#include "string.h"

// --- PRIVATE STRUCTURE FOR A SINGLE BUFFER ---
typedef struct fifo_internal_buffer_t {
    uint8_t is_empty;
    uint8_t is_full;
    DATA_TYPE* first_element;
    DATA_TYPE* last_element;
    DATA_TYPE* read;
    DATA_TYPE* write;
} fifo_internal_buffer_t;

// --- HOLD MULTIPLE BUFFERS, organized by pointers on a stack ---
typedef struct buffer_system_t {
    unsigned int claim_count;
    unsigned int return_count;
    unsigned int stack_size;
    fifo_internal_buffer_t* buffer_stack[FIFO_BUFFER_COUNT];
    fifo_internal_buffer_t  buffers[FIFO_BUFFER_COUNT];
} buffer_system_t;
buffer_system_t fifo_system;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * HELPER FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t ouf_of_bounds(fifo_buffer_t buffer, DATA_TYPE* pointer) {
    if (pointer > buffer->last_element ) return 1;
    if (pointer < buffer->first_element) return 1;
    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MANAGE BUFFER INSTANCES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t fifo_init_system() {
    fifo_system.stack_size = FIFO_BUFFER_COUNT;
    fifo_system.claim_count = 0;
    fifo_system.return_count = 0;
    for (int i = 0 ; i < FIFO_BUFFER_COUNT ; i++) {
        fifo_system.buffer_stack[i] = &fifo_system.buffers[i];
    }
    return FIFO_OK;
}

fifo_buffer_t fifo_claim_buffer() {
    uint8_t stack_not_empty = fifo_system.stack_size > 0;
    if (stack_not_empty){
        fifo_system.claim_count++;
        return fifo_system.buffer_stack[--fifo_system.stack_size];
    }
    return NULL;
}

uint8_t fifo_buffer_on_stack(fifo_buffer_t buffer) {
    for (int i = 0; i < fifo_system.stack_size ; i++) {
        if (&fifo_system.buffers[i] == buffer)
            return FIFO_DUPLICATE;
    }
    fifo_system.return_count++;
    return FIFO_OK;
}

uint8_t fifo_return_buffer(fifo_buffer_t buffer) {
    fifo_buffer_t buffer_slot_address = &fifo_system.buffers[0];
    if (fifo_buffer_on_stack(buffer) == FIFO_DUPLICATE)
        return FIFO_DUPLICATE;

    for (int i = 0 ; i < FIFO_BUFFER_COUNT ; i++, buffer_slot_address++) {
        if (buffer_slot_address == buffer) {
            fifo_system.buffer_stack[fifo_system.stack_size++] = buffer;
            return FIFO_OK;
        }
    }
    return FIFO_NULL;
}

uint8_t fifo_init_buffer(fifo_buffer_t buffer, fifo_data_info_t* data) {
    if (buffer == NULL) return FIFO_NULL;
    memset(buffer, 0, sizeof(fifo_internal_buffer_t));
    buffer->first_element = data->array;
    buffer->last_element  = data->array + (data->element_count - 1);
    buffer->read             = buffer->first_element;
    buffer->write            = buffer->first_element;
    buffer->is_empty         = 1;
    return FIFO_OK;
}

fifo_buffer_t fifo_claim_and_init_buffer(fifo_data_info_t* data) {
    fifo_buffer_t buffer;
    buffer = fifo_claim_buffer();
    if (fifo_init_buffer(buffer, data) == FIFO_NULL) {
        return NULL;
    } else {
        return buffer;
    }
}

uint8_t fifo_get_used_buffer_slot_count() {
    return FIFO_BUFFER_COUNT - fifo_system.stack_size;
}

// --- purely informational for debug / stats / tests ---
unsigned int fifo_claim_count() {
    return fifo_system.claim_count;
}

// --- purely informational for debug / stats / tests ---
unsigned int fifo_return_count() {
    return fifo_system.return_count;

}

// --- purely informational for debug / stats / tests ---
unsigned int fifo_get_buffer_size(fifo_buffer_t buffer) {
    unsigned int buffer_size = (*(buffer->last_element)  \
                             - *(buffer->first_element)) \
                             + 1;
    return buffer_size;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MANAGE BUFFER CONTENT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint8_t fifo_add_element (fifo_buffer_t buffer, DATA_TYPE element) {
    if (buffer->is_full) return FIFO_FULL;

    *(buffer->write) = element;
    buffer->write++;
    buffer->is_empty = 0;

    if (ouf_of_bounds(buffer, buffer->write))
        buffer->write = buffer->first_element;

    if (buffer->write == buffer->read) buffer->is_full = 1;

    return FIFO_OK;
}

DATA_TYPE* fifo_read_element (fifo_buffer_t buffer) {
    if (buffer->is_empty) return NULL;

    DATA_TYPE* element = *(buffer->read);
    buffer->read++;
    buffer->is_full = 0;

    if (ouf_of_bounds(buffer, buffer->read))
        buffer->read = buffer->first_element;

    if (buffer->write == buffer->read) buffer->is_empty = 1;

    return element;
}

uint8_t fifo_is_empty(fifo_buffer_t buffer) {
    return buffer->is_empty;
}

uint8_t fifo_is_full(fifo_buffer_t buffer) {
    return buffer->is_full;
}

#ifdef TEST
void print_buffer(fifo_buffer_t buffer) {
    unsigned int buffer_size = fifo_get_buffer_size(buffer);
    for (int i = 0; i < buffer_size ; i++) {
        printf("%d ", buffer->first_element[i]);
        if (!(i+1 % 0))
            printf("\n");
    }
}
#endif

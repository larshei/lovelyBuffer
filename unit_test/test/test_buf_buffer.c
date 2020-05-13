#include "unity.h"
#include "../../fifo_buffer.h"
#include "../../fifo_buffer_config.h"

#define BUFFER_SIZE     20
DATA_TYPE buffer_array[BUFFER_SIZE];
fifo_data_info_t  buffer_info;
fifo_buffer_t*    buffer;

void setUp(void) {
    TEST_ASSERT_EQUAL(FIFO_OK, fifo_init_system() );
}

void tearDown(void) {

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TEST BUFFER SLOTS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void test_claim_all_buffers(void) {
    fifo_buffer_t buffer_slot[FIFO_BUFFER_COUNT];
    // initial buffer from init
    TEST_ASSERT_EQUAL(0, fifo_get_used_buffer_slot_count());

    for (int i = 0; i < FIFO_BUFFER_COUNT ; i++) {
        TEST_ASSERT_EQUAL(i, fifo_claim_count());
        buffer_slot[i] = fifo_claim_buffer();
        TEST_ASSERT_EQUAL(i+1, fifo_get_used_buffer_slot_count());
        TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot[i]);
    }
    TEST_ASSERT_EQUAL(FIFO_BUFFER_COUNT, fifo_claim_count());
    TEST_ASSERT_EQUAL(0, fifo_return_count());
}

void test_claim_too_many_buffers(void) {
    test_claim_all_buffers();

    fifo_buffer_t buffer_slot;
    buffer_slot = fifo_claim_buffer();
    TEST_ASSERT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(FIFO_BUFFER_COUNT, fifo_claim_count());

}

void test_return_valid_buffer (void) {
    fifo_buffer_t buffer_slot;
    buffer_slot = fifo_claim_buffer();
    TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(1, fifo_get_used_buffer_slot_count());

    uint8_t result = fifo_return_buffer(buffer_slot);
    TEST_ASSERT_EQUAL(FIFO_OK, result);
}

void test_return_valid_buffer_multiple_times (void) {
    fifo_buffer_t buffer_slot;
    buffer_slot = fifo_claim_buffer();
    TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(1, fifo_get_used_buffer_slot_count());

    uint8_t result = fifo_return_buffer(buffer_slot);
    TEST_ASSERT_EQUAL(FIFO_OK, result);
    TEST_ASSERT_EQUAL(0, fifo_get_used_buffer_slot_count());

    result = fifo_return_buffer(buffer_slot);
    TEST_ASSERT_EQUAL(FIFO_DUPLICATE, result);
    TEST_ASSERT_EQUAL(0, fifo_get_used_buffer_slot_count());

    TEST_ASSERT_EQUAL(1, fifo_claim_count());
    TEST_ASSERT_EQUAL(1, fifo_return_count());

}

void test_return_invalid_buffer (void) {
    fifo_buffer_t buffer_slot;
    buffer_slot = fifo_claim_buffer();
    TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(1, fifo_get_used_buffer_slot_count());

    fifo_buffer_t wrong_address = (fifo_buffer_t)((int)buffer_slot - 1111);
    uint8_t result = fifo_return_buffer(wrong_address);
    TEST_ASSERT_EQUAL(FIFO_NULL, result);
    TEST_ASSERT_EQUAL(1, fifo_get_used_buffer_slot_count());
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TEST BUFFER CONTENT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
fifo_data_info_t* create_buffer_config() {
    buffer_info.array = (DATA_TYPE*) buffer_array;
    buffer_info.element_count = BUFFER_SIZE;
    buffer_info.element_size  = sizeof(DATA_TYPE);
    return (fifo_data_info_t*)&buffer_info;
} 

void test_init_buffer(void) {
    fifo_buffer_t buffer_slot = fifo_claim_buffer();
    uint8_t result = fifo_init_buffer(buffer_slot, create_buffer_config());

    TEST_ASSERT_EQUAL(FIFO_OK, result);
    TEST_ASSERT_EQUAL(1, fifo_is_empty(buffer_slot));
    TEST_ASSERT_EQUAL(0, fifo_is_full (buffer_slot));
}

void test_claim_init_buffer(void) {
    fifo_buffer_t buffer_slot = fifo_claim_and_init_buffer(create_buffer_config());

    TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(1, fifo_is_empty(buffer_slot));
    TEST_ASSERT_EQUAL(0, fifo_is_full (buffer_slot));
}

void test_init_NULL_buffer(void) {
    uint8_t result = fifo_init_buffer(NULL, create_buffer_config());
    TEST_ASSERT_EQUAL(FIFO_NULL, result);
}

void test_buffer_add_element(void) {
    fifo_buffer_t buffer_slot = fifo_claim_buffer();
    fifo_init_buffer(buffer_slot, create_buffer_config());

    DATA_TYPE value = 1000;
    fifo_add_element(buffer_slot, value);
    TEST_ASSERT_EQUAL(0, fifo_is_empty(buffer_slot));
    TEST_ASSERT_EQUAL(0, fifo_is_full(buffer_slot));
}

void test_buffer_add_remove_element(void) {
    fifo_buffer_t buffer_slot = fifo_claim_buffer();
    fifo_init_buffer(buffer_slot, create_buffer_config());

    DATA_TYPE value = 1000;
    fifo_add_element(buffer_slot, value);
    value = 0;
    value = fifo_read_element(buffer_slot);
    TEST_ASSERT_EQUAL(1, fifo_is_empty(buffer_slot));
}

void test_buffer_fill(void) {
    fifo_buffer_t buffer_slot = fifo_claim_buffer();
    fifo_init_buffer(buffer_slot, create_buffer_config());

    uint8_t result;

    for (int i = 0; i < BUFFER_SIZE ; i++) {
        TEST_ASSERT_EQUAL(0, fifo_is_full(buffer_slot));
        result = fifo_add_element(buffer_slot, i);
        TEST_ASSERT_EQUAL(FIFO_OK, result);
        TEST_ASSERT_EQUAL(0, fifo_is_empty(buffer_slot));
    }
    TEST_ASSERT_EQUAL(1, fifo_is_full(buffer_slot));
    result = fifo_add_element(buffer_slot, 10);
    TEST_ASSERT_EQUAL(FIFO_FULL, result);
}

void test_buffer_fill_empty(void) {
    fifo_buffer_t buffer_slot = fifo_claim_buffer();
    fifo_init_buffer(buffer_slot, create_buffer_config());

    uint8_t result;
    DATA_TYPE read_data;

    for (int i = 0; i < BUFFER_SIZE ; i++) {
        TEST_ASSERT_EQUAL(0, fifo_is_full(buffer_slot));
        result = fifo_add_element(buffer_slot, i);
        TEST_ASSERT_EQUAL(FIFO_OK, result);
        TEST_ASSERT_EQUAL(0, fifo_is_empty(buffer_slot));
    }
    TEST_ASSERT_EQUAL(1, fifo_is_full(buffer_slot));
    result = fifo_add_element(buffer_slot, 10);
    TEST_ASSERT_EQUAL(FIFO_FULL, result);

    for (int i = 0; i < BUFFER_SIZE ; i++) {
        TEST_ASSERT_EQUAL(0, fifo_is_empty(buffer_slot));
        read_data = fifo_read_element(buffer_slot);
        TEST_ASSERT_EQUAL(i, read_data);
        TEST_ASSERT_EQUAL(0, fifo_is_full(buffer_slot));
    }
    TEST_ASSERT_EQUAL(1, fifo_is_empty(buffer_slot));
}

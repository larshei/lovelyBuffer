#include "unity.h"
#include "../../buf_buffer.h"
#include "../../buf_buffer_config.h"

#define BUFFER_SIZE     20
DATA_TYPE buffer_array[BUFFER_SIZE];
buf_data_info_t  buffer_info;
buf_buffer_t*    buffer;

void setUp(void) {
    TEST_ASSERT_EQUAL(BUF_OK, buf_init_system() );
}

void tearDown(void) {

}

// -------- HELPERS ----------
buf_data_info_t* create_buffer_config() {
    buffer_info.array = (DATA_TYPE*) buffer_array;
    buffer_info.element_count = BUFFER_SIZE;
    buffer_info.element_size  = sizeof(DATA_TYPE);
    return (buf_data_info_t*)&buffer_info;
} 

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TEST BUFFER CLAIM / INIT / RETURN
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void test_claim_all_buffers(void) {
    buf_buffer_t buffer_slot[BUF_BUFFER_COUNT];
    // initial buffer from init
    TEST_ASSERT_EQUAL(0, buf_get_used_buffer_slot_count());

    for (int i = 0; i < BUF_BUFFER_COUNT ; i++) {
        TEST_ASSERT_EQUAL(i, buf_claim_count());
        buffer_slot[i] = buf_claim_buffer();
        TEST_ASSERT_EQUAL(i+1, buf_get_used_buffer_slot_count());
        TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot[i]);
    }
    TEST_ASSERT_EQUAL(BUF_BUFFER_COUNT, buf_claim_count());
    TEST_ASSERT_EQUAL(0, buf_return_count());
}

void test_claim_too_many_buffers(void) {
    test_claim_all_buffers();

    buf_buffer_t buffer_slot;
    buffer_slot = buf_claim_buffer();
    TEST_ASSERT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(BUF_BUFFER_COUNT, buf_claim_count());

}
// -----------------------------------------
void test_init_buffer(void) {
    buf_buffer_t buffer_slot = buf_claim_buffer();
    uint8_t result = buf_init_buffer(buffer_slot, create_buffer_config());

    TEST_ASSERT_EQUAL(BUF_OK, result);
    TEST_ASSERT_EQUAL(1, buf_is_empty(buffer_slot));
    TEST_ASSERT_EQUAL(0, buf_is_full (buffer_slot));
}

void test_claim_init_buffer(void) {
    buf_buffer_t buffer_slot = buf_claim_and_init_buffer(create_buffer_config());

    TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(1, buf_is_empty(buffer_slot));
    TEST_ASSERT_EQUAL(0, buf_is_full (buffer_slot));
}

void test_init_NULL_buffer(void) {
    uint8_t result = buf_init_buffer(NULL, create_buffer_config());
    TEST_ASSERT_EQUAL(BUF_NULL, result);
}
// -----------------------------------------
void test_return_valid_buffer (void) {
    buf_buffer_t buffer_slot;
    buffer_slot = buf_claim_buffer();
    TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(1, buf_get_used_buffer_slot_count());

    uint8_t result = buf_return_buffer(buffer_slot);
    TEST_ASSERT_EQUAL(BUF_OK, result);
}

void test_return_valid_buffer_multiple_times (void) {
    buf_buffer_t buffer_slot;
    buffer_slot = buf_claim_buffer();
    TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(1, buf_get_used_buffer_slot_count());

    uint8_t result = buf_return_buffer(buffer_slot);
    TEST_ASSERT_EQUAL(BUF_OK, result);
    TEST_ASSERT_EQUAL(0, buf_get_used_buffer_slot_count());

    result = buf_return_buffer(buffer_slot);
    TEST_ASSERT_EQUAL(BUF_DUPLICATE, result);
    TEST_ASSERT_EQUAL(0, buf_get_used_buffer_slot_count());

    TEST_ASSERT_EQUAL(1, buf_claim_count());
    TEST_ASSERT_EQUAL(1, buf_return_count());

}

void test_return_invalid_buffer (void) {
    buf_buffer_t buffer_slot;
    buffer_slot = buf_claim_buffer();
    TEST_ASSERT_NOT_EQUAL(NULL, buffer_slot);
    TEST_ASSERT_EQUAL(1, buf_get_used_buffer_slot_count());

    buf_buffer_t wrong_address = (buf_buffer_t)((int)buffer_slot - 1111);
    uint8_t result = buf_return_buffer(wrong_address);
    TEST_ASSERT_EQUAL(BUF_NULL, result);
    TEST_ASSERT_EQUAL(1, buf_get_used_buffer_slot_count());
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TEST BUFFER MODES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void test_buffer_mode(void) {
    uint8_t buffer_mode;
    uint8_t result;

    buf_buffer_t buffer_slot = buf_claim_and_init_buffer(create_buffer_config());
    buffer_mode = buf_get_buffer_mode(buffer_slot);
    TEST_ASSERT_EQUAL(BUF_FIFOBUF, buffer_mode);

    result = buf_ring_buffer_mode(buffer_slot);
    TEST_ASSERT_EQUAL(BUF_OK, result);
    buffer_mode = buf_get_buffer_mode(buffer_slot);
    TEST_ASSERT_EQUAL(BUF_RINGBUF, buffer_mode);

    result = buf_fifo_buffer_mode(buffer_slot);
    TEST_ASSERT_EQUAL(BUF_OK, result);
    buffer_mode = buf_get_buffer_mode(buffer_slot);
    TEST_ASSERT_EQUAL(BUF_FIFOBUF, buffer_mode);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TEST BUFFER USAGE - FIFO
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void test_buffer_add_element(void) {
    buf_buffer_t buffer_slot = buf_claim_buffer();
    buf_init_buffer(buffer_slot, create_buffer_config());

    DATA_TYPE value = 1000;
    buf_add_element(buffer_slot, value);
    TEST_ASSERT_EQUAL(0, buf_is_empty(buffer_slot));
    TEST_ASSERT_EQUAL(0, buf_is_full(buffer_slot));
}

void test_buffer_add_remove_element(void) {
    buf_buffer_t buffer_slot = buf_claim_buffer();
    buf_init_buffer(buffer_slot, create_buffer_config());

    DATA_TYPE value = 1000;
    buf_add_element(buffer_slot, value);
    value = 0;
    value = *(buf_read_element(buffer_slot));
    TEST_ASSERT_EQUAL(1, buf_is_empty(buffer_slot));
}

void test_buffer_fill(void) {
    buf_buffer_t buffer_slot = buf_claim_buffer();
    buf_init_buffer(buffer_slot, create_buffer_config());

    uint8_t result;

    for (int i = 0; i < BUFFER_SIZE ; i++) {
        TEST_ASSERT_EQUAL(0, buf_is_full(buffer_slot));
        result = buf_add_element(buffer_slot, i);
        TEST_ASSERT_EQUAL(BUF_OK, result);
        TEST_ASSERT_EQUAL(0, buf_is_empty(buffer_slot));
    }
    TEST_ASSERT_EQUAL(1, buf_is_full(buffer_slot));
    result = buf_add_element(buffer_slot, 10);
    TEST_ASSERT_EQUAL(BUF_FULL, result);
}

void test_buffer_fill_empty(void) {
    buf_buffer_t buffer_slot = buf_claim_buffer();
    buf_init_buffer(buffer_slot, create_buffer_config());

    uint8_t result;
    DATA_TYPE read_data;

    for (int i = 0; i < BUFFER_SIZE ; i++) {
        TEST_ASSERT_EQUAL(0, buf_is_full(buffer_slot));
        result = buf_add_element(buffer_slot, i);
        TEST_ASSERT_EQUAL(BUF_OK, result);
        TEST_ASSERT_EQUAL(0, buf_is_empty(buffer_slot));
    }
    TEST_ASSERT_EQUAL(1, buf_is_full(buffer_slot));
    result = buf_add_element(buffer_slot, 10);
    TEST_ASSERT_EQUAL(BUF_FULL, result);

    for (int i = 0; i < BUFFER_SIZE ; i++) {
        TEST_ASSERT_EQUAL(0, buf_is_empty(buffer_slot));
        read_data = *(buf_read_element(buffer_slot));
        TEST_ASSERT_EQUAL(i, read_data);
        TEST_ASSERT_EQUAL(0, buf_is_full(buffer_slot));
    }
    TEST_ASSERT_EQUAL(1, buf_is_empty(buffer_slot));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TEST BUFFER USAGE - RING BUFFER
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// TODO add unit test for ring buffer mode

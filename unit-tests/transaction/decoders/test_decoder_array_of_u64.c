#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <cmocka.h>
#include "../unit-tests/mocks.h"
#include "transaction/parsers.h"
#include "types.h"
#include "globals.h"

static void test_decoder_array_of_u64(void **state) {
    (void) state;

    // clang-format off
    uint8_t data[] = {
        0x02, // two elements
        0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // 72057594037927953 LE
        0x05, 0xD1, 0xBC, 0x75, 0x00, 0x00, 0x00, 0x00, // 1975308549 LE
    };
    // clang-format on

    field_t field = {0};
    buffer_t buffer_valid = {.offset = 0, .ptr = data, .size = sizeof(data)};
    buffer_t buffer_invalid = {.offset = 0, .ptr = data, .size = 0};

    // invalid length
    assert_false(decoder_array_of_u64(&buffer_invalid, &field, false));

    assert_true(decoder_array_of_u64(&buffer_valid, &field, false));
    assert_string_equal(field.value, "[ 72057594037927953, 1975308549 ]");
}

static void test_decoder_array_of_u64_hashing(void **state) {
    (void) state;

    // clang-format off
    uint8_t data[] = {
        0x02, // two elements
        0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // 72057594037927953 LE
        0x05, 0xD1, 0xBC, 0x75, 0x00, 0x00, 0x00, 0x00, // 1975308549 LE
    };
    // clang-format on

    field_t field = {0};
    buffer_t buffer_valid = {.offset = 0, .ptr = data, .size = sizeof(data)};

    // both functions will be called once
    will_return(__wrap_cx_hash_no_throw, 0);
    will_return(__wrap_cx_hash_get_size, 0);

    // expect cx_hash to be called with number of elements
    expect_value(__wrap_cx_hash_no_throw, hash, &G_context.tx_info.sha);
    expect_value(__wrap_cx_hash_no_throw, mode, 0);
    expect_memory(__wrap_cx_hash_no_throw, in, data, sizeof(data));
    expect_value(__wrap_cx_hash_no_throw, len, sizeof(data));
    expect_value(__wrap_cx_hash_no_throw, out, NULL);
    expect_value(__wrap_cx_hash_no_throw, out_len, 0);
    // expect to success
    assert_true(decoder_array_of_u64(&buffer_valid, &field, true));

    // expect it to not modify the output field, just hash data
    assert_string_equal(field.value, "");
}

int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_decoder_array_of_u64), cmocka_unit_test(test_decoder_array_of_u64_hashing)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}

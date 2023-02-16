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

static void test_decoder_array_of_strings(void **state) {
    (void) state;

    // clang-format off
    uint8_t data[] = {
        0x02,                                       // two strings
        0x04,                                       // string length
        0x68, 0x69, 0x76, 0x65,                     // "hive"
        0x05,                                       // string length
        0x70, 0x6f, 0x77, 0x65, 0x72                // "power"
    };
    // clang-format on

    field_t field = {0};
    buffer_t buffer_valid = {.offset = 0, .ptr = data, .size = sizeof(data)};
    buffer_t buffer_invalid = {.offset = 0, .ptr = data, .size = 0};

    // invalid length
    assert_false(decoder_array_of_strings(&buffer_invalid, &field, false));

    // string length bigger than buffer length
    buffer_seek_set(&buffer_invalid, 0);
    buffer_invalid.size = sizeof(data) - 2;
    assert_false(decoder_array_of_strings(&buffer_invalid, &field, false));

    assert_true(decoder_array_of_strings(&buffer_valid, &field, false));
    assert_string_equal(field.value, "[ hive, power ]");
}

static void test_decoder_array_of_strings_hashing(void **state) {
    (void) state;

    // clang-format off
    uint8_t data[] = {
        0x02,                                       // two strings
        0x04,                                       // string length
        0x68, 0x69, 0x76, 0x65,                     // "hive"
        0x05,                                       // string length
        0x70, 0x6f, 0x77, 0x65, 0x72                // "power"
    };
    // clang-format on

    field_t field = {0};
    buffer_t buffer_valid = {.offset = 0, .ptr = data, .size = sizeof(data)};

    // both functions will be called once
    will_return(__wrap_cx_hash_no_throw, 0);
    will_return(__wrap_cx_hash_get_size, 0);

    // expect cx_hash to be called with buffer data
    expect_value(__wrap_cx_hash_no_throw, hash, &G_context.tx_info.sha);
    expect_value(__wrap_cx_hash_no_throw, mode, 0);
    expect_memory(__wrap_cx_hash_no_throw, in, data, sizeof(data));
    expect_value(__wrap_cx_hash_no_throw, len, sizeof(data));
    expect_value(__wrap_cx_hash_no_throw, out, NULL);
    expect_value(__wrap_cx_hash_no_throw, out_len, 0);

    // expect to success
    assert_true(decoder_array_of_strings(&buffer_valid, &field, true));

    // expect it to not modify the output field, just hash data
    assert_string_equal(field.value, "");
}

int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_decoder_array_of_strings), cmocka_unit_test(test_decoder_array_of_strings_hashing)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}

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

static void test_decoder_string(void **state) {
    (void) state;

    // clang-format off

    uint8_t data[] = {
        0x7,                                        // string length
        0x65, 0x6e, 0x67, 0x72, 0x61, 0x76, 0x65    // "engrave"
    };

    // clang-format on

    field_t field = {0};
    buffer_t buffer_valid = {.offset = 0, .ptr = data, .size = sizeof(data)};
    buffer_t buffer_invalid = {.offset = 0, .ptr = data, .size = 0};

    // invalid length
    assert_false(decoder_string(&buffer_invalid, &field, false));

    // string length bigger than buffer length
    buffer_seek_set(&buffer_invalid, 0);
    buffer_invalid.size = sizeof(data) - 2;
    assert_false(decoder_string(&buffer_invalid, &field, false));

    assert_true(decoder_string(&buffer_valid, &field, false));
    assert_string_equal(field.value, "engrave");
}

static void test_decoder_string_long(void **state) {
    (void) state;

    uint8_t data[] = {129, 1,   97,  120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
                      120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
                      120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
                      120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
                      120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 122};

    field_t field = {0};
    buffer_t buffer_valid = {.offset = 0, .ptr = data, .size = sizeof(data)};

    assert_true(decoder_string(&buffer_valid, &field, false));
    assert_string_equal(field.value,
                        "axxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxz");
}

static void test_decoder_string_hashing(void **state) {
    (void) state;

    // clang-format off

    uint8_t data[] = {
        0x7,                                        // string length
        0x65, 0x6e, 0x67, 0x72, 0x61, 0x76, 0x65,   // "engrave"
        0x00, 0x00                                  // fake values to increase size
    };

    // clang-format on

    field_t field = {0};
    buffer_t buffer_valid = {.offset = 0, .ptr = data, .size = sizeof(data)};

    will_return(__wrap_cx_hash_no_throw, 0);
    will_return(__wrap_cx_hash_get_size, 0);

    expect_value(__wrap_cx_hash_no_throw, hash, &G_context.tx_info.sha);
    expect_value(__wrap_cx_hash_no_throw, mode, 0);
    expect_memory(__wrap_cx_hash_no_throw, in, data, sizeof(data) - 2);
    expect_value(__wrap_cx_hash_no_throw, len, sizeof(data) - 2);
    expect_value(__wrap_cx_hash_no_throw, out, NULL);
    expect_value(__wrap_cx_hash_no_throw, out_len, 0);

    // expect to success
    assert_true(decoder_string(&buffer_valid, &field, true));

    // expect it to not modify the output field, just hash data
    assert_string_equal(field.value, "");
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_decoder_string), cmocka_unit_test(test_decoder_string_hashing), cmocka_unit_test(test_decoder_string_long)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}

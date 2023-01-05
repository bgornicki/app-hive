#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <cmocka.h>

#include "common/format.h"

static void test_format_timestamp(void **state) {
    (void) state;

    char out[20] = {0};

    // return false if not enough space to handle output string
    assert_false(format_timestamp(0, out, 19));

    // return false if NULL pointer passed as out buffer
    assert_false(format_timestamp(0, NULL, 20));

    // valid result for oldest timestmap
    memset(out, 0, sizeof(out));
    assert_true(format_timestamp(0, out, sizeof(out)));
    assert_string_equal(out, "1970-01-01T00:00:00");

    // valid result for the past
    memset(out, 0, sizeof(out));
    assert_true(format_timestamp(1616757759, out, sizeof(out)));
    assert_string_equal(out, "2021-03-26T11:22:39");

    // valid result for the future
    memset(out, 0, sizeof(out));
    assert_true(format_timestamp(2158029321, out, sizeof(out)));
    assert_string_equal(out, "2038-05-21T04:35:21");

    // valid result for leap year
    memset(out, 0, sizeof(out));
    assert_true(format_timestamp(1709164800, out, sizeof(out)));
    assert_string_equal(out, "2024-02-29T00:00:00");

    // valid for first day of the year
    memset(out, 0, sizeof(out));
    assert_true(format_timestamp(1577836800, out, sizeof(out)));
    assert_string_equal(out, "2020-01-01T00:00:00");

    // valid for last day of the year
    memset(out, 0, sizeof(out));
    assert_true(format_timestamp(1577836799, out, sizeof(out)));
    assert_string_equal(out, "2019-12-31T23:59:59");
}

static void test_format_i64(void **state) {
    (void) state;

    char out[21] = {0};

    int64_t value = 0;
    assert_true(format_i64(value, out, sizeof(out)));
    assert_string_equal(out, "0");

    value = (int64_t) 9223372036854775807ull;  // MAX_INT64
    memset(out, 0, sizeof(out));
    assert_true(format_i64(value, out, sizeof(out)));
    assert_string_equal(out, "9223372036854775807");

    // buffer too small
    assert_false(format_i64(value, out, sizeof(out) - 5));

    value = (int64_t) -9223372036854775808ull;  // MIN_INT64
    memset(out, 0, sizeof(out));
    assert_true(format_i64(value, out, sizeof(out)));
    assert_string_equal(out, "-9223372036854775808");
}

static void test_format_u64(void **state) {
    (void) state;

    char out[21] = {0};

    uint64_t value = 0;
    assert_true(format_u64(value, out, sizeof(out)));
    assert_string_equal(out, "0");

    value = (uint64_t) 18446744073709551615ull;  // MAX_UNT64
    memset(out, 0, sizeof(out));
    assert_true(format_u64(value, out, sizeof(out)));
    assert_string_equal(out, "18446744073709551615");

    // buffer too small
    assert_false(format_u64(value, out, sizeof(out) - 5));
}

static void test_format_asset(void **state) {
    (void) state;

    char out[29] = {0};

    asset_t asset_steem = {.amount = 1337, .precision = 3, .symbol = {0x53, 0x54, 0x45, 0x45, 0x4d, 0x00}};
    asset_t asset_sbd = {.amount = 1337, .precision = 3, .symbol = {0x53, 0x42, 0x44, 0x00}};
    asset_t asset_vests = {.amount = 13371337, .precision = 6, .symbol = {0x56, 0x45, 0x53, 0x54, 0x53, 0x00}};
    asset_t asset_steem_zero = {.amount = 0, .precision = 3, .symbol = {0x53, 0x54, 0x45, 0x45, 0x4d, 0x00, 0x00}};
    asset_t asset_testnet = {.amount = 0, .precision = 3, .symbol = {0x54, 0x45, 0x53, 0x54, 0x53, 0x00, 0x00}};

    // not enought space
    assert_false(format_asset(&asset_steem, out, sizeof(out) - 5));

    // should parse asset and replace STEEM with HIVE
    assert_true(format_asset(&asset_steem, out, sizeof(out)));
    assert_string_equal(out, "1.337 HIVE");

    // should parse asset and replace SBD with HBD
    assert_true(format_asset(&asset_sbd, out, sizeof(out)));
    assert_string_equal(out, "1.337 HBD");

    // should parse asset
    assert_true(format_asset(&asset_vests, out, sizeof(out)));
    assert_string_equal(out, "13.371337 VESTS");

    // should append and prepend required zeroes to match precision
    assert_true(format_asset(&asset_steem_zero, out, sizeof(out)));
    assert_string_equal(out, "0.000 HIVE");

    // should parse testnet asset
    assert_true(format_asset(&asset_testnet, out, sizeof(out)));
    assert_string_equal(out, "0.000 TESTS");
}

static void test_format_hash(void **state) {
    (void) state;

    uint8_t hash[] = {0xB2, 0xBF, 0x27, 0xF1, 0x05, 0xD0, 0xE0, 0xE1, 0x2F, 0x8B, 0xC9, 0x13, 0xC8, 0xE1, 0x24, 0xB2,
                      0x13, 0x8E, 0x71, 0x1A, 0xFA, 0xEA, 0xA7, 0xE8, 0x5F, 0x18, 0x6C, 0x2D, 0x83, 0x87, 0xF4, 0x46};

    char out[(sizeof(hash) * 2) + 1] = {0};

    // return false if not enough space to handle output string
    assert_false(format_hash(hash, sizeof(hash), out, 19));

    // return true for valid hash and output string
    memset(out, 0, sizeof(out));
    assert_true(format_hash(hash, sizeof(hash), out, sizeof(out)));
    assert_string_equal(out, "B2BF27F105D0E0E12F8BC913C8E124B2138E711AFAEAA7E85F186C2D8387F446");
}

int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_format_timestamp),
                                       cmocka_unit_test(test_format_i64),
                                       cmocka_unit_test(test_format_u64),
                                       cmocka_unit_test(test_format_asset),
                                       cmocka_unit_test(test_format_hash)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}

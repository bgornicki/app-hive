#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>

#include <cmocka.h>

#include "common/buffer.h"

static void test_buffer_can_read(void **state) {
    (void) state;

    uint8_t temp[20] = {0};
    buffer_t buf = {.ptr = temp, .size = sizeof(temp), .offset = 0};

    assert_true(buffer_can_read(&buf, 20));

    assert_true(buffer_seek_cur(&buf, 20));
    assert_false(buffer_can_read(&buf, 1));
}

static void test_buffer_seek(void **state) {
    (void) state;

    uint8_t temp[20] = {0};
    buffer_t buf = {.ptr = temp, .size = sizeof(temp), .offset = 0};

    assert_true(buffer_can_read(&buf, 20));

    assert_true(buffer_seek_cur(&buf, 20));  // seek at offset 20
    assert_false(buffer_can_read(&buf, 1));  // can't read 1 byte
    assert_false(buffer_seek_cur(&buf, 1));  // can't move at offset 21

    assert_true(buffer_seek_end(&buf, 19));
    assert_int_equal(buf.offset, 1);
    assert_false(buffer_seek_end(&buf, 21));  // can't seek at offset -1

    assert_true(buffer_seek_set(&buf, 10));
    assert_int_equal(buf.offset, 10);
    assert_false(buffer_seek_set(&buf, 21));  // can't seek at offset 21
}

static void test_buffer_read(void **state) {
    (void) state;

    // clang-format off
    uint8_t temp[15] = {
        0xFF,
        0x01, 0x02,
        0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E
    };
    
    buffer_t buf = {.ptr = temp, .size = sizeof(temp), .offset = 0};

    uint8_t first = 0;
    assert_true(buffer_read_u8(&buf, &first));
    assert_int_equal(first, 255);                // 0xFF
    assert_true(buffer_seek_end(&buf, 0));       // seek at offset 19
    assert_false(buffer_read_u8(&buf, &first));  // can't read 1 byte

    uint16_t second = 0;
    assert_true(buffer_seek_set(&buf, 1));             // set back to offset 1
    assert_true(buffer_read_u16(&buf, &second, BE));   // big endian
    assert_int_equal(second, 258);                     // 0x01 0x02
    assert_true(buffer_seek_set(&buf, 1));             // set back to offset 1
    assert_true(buffer_read_u16(&buf, &second, LE));   // little endian
    assert_int_equal(second, 513);                     // 0x02 0x01
    assert_true(buffer_seek_set(&buf, 14));            // seek at offset 14
    assert_false(buffer_read_u16(&buf, &second, BE));  // can't read 2 bytes

    uint32_t third = 0;
    assert_true(buffer_seek_set(&buf, 3));            // set back to offset 3
    assert_true(buffer_read_u32(&buf, &third, BE));   // big endian
    assert_int_equal(third, 50595078);                // 0x03 0x04 0x05 0x06
    assert_true(buffer_seek_set(&buf, 3));            // set back to offset 3
    assert_true(buffer_read_u32(&buf, &third, LE));   // little endian
    assert_int_equal(third, 100992003);               // 0x06 0x05 0x04 0x03
    assert_true(buffer_seek_set(&buf, 12));           // seek at offset 12
    assert_false(buffer_read_u32(&buf, &third, BE));  // can't read 4 bytes

    uint64_t fourth = 0;
    assert_true(buffer_seek_set(&buf, 7));             // set back to offset 7
    assert_true(buffer_read_u64(&buf, &fourth, BE));   // big endian
    assert_int_equal(fourth, 506664896818842894);      // 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E
    assert_true(buffer_seek_set(&buf, 7));             // set back to offset 7
    assert_true(buffer_read_u64(&buf, &fourth, LE));   // little endian
    assert_int_equal(fourth, 1012478732780767239);     // 0x0E 0x0D 0x0C 0x0B 0x0A 0x09 0x08 0x07
    assert_true(buffer_seek_set(&buf, 8));             // seek at offset 8
    assert_false(buffer_read_u64(&buf, &fourth, BE));  // can't read 8 bytes
}

static void test_buffer_read_varint(void **state) {

    // clang-format off
    uint8_t temp_varint[] = {
        0x01, // 1
        0x80, 0x01, // 128
        0x80, 0x80, 0x01, // 16384
        0x80, 0x80, 0x80, 0x01, // 2097152
        0x80, 0x80, 0x80, 0x80, 0x01, // 268435456
        0x80, 0x80, 0x80, 0x80, 0x80, 0x01, // 34359738368
        0x80 // fake varint, not enough bytes
    };

    buffer_t buf = {.ptr = temp_varint, .size = sizeof(temp_varint), .offset = 0};
    uint64_t varint = 0;
    
    assert_true(buffer_read_varint(&buf, &varint));
    assert_int_equal(varint, 1);
    assert_true(buffer_read_varint(&buf, &varint));
    assert_int_equal(varint, 128);
    assert_true(buffer_read_varint(&buf, &varint));
    assert_int_equal(varint, 16384);
    assert_true(buffer_read_varint(&buf, &varint));
    assert_int_equal(varint, 2097152);    
    assert_true(buffer_read_varint(&buf, &varint));
    assert_int_equal(varint, 268435456);
    assert_true(buffer_read_varint(&buf, &varint));
    assert_int_equal(varint, 34359738368);

    assert_false(buffer_read_varint(&buf, &varint)); // not enough bytes
}

static void test_buffer_copy(void **state) {
    (void) state;

    uint8_t output[5] = {0};
    uint8_t temp[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    buffer_t buf = {.ptr = temp, .size = sizeof(temp), .offset = 0};

    assert_true(buffer_copy(&buf, output, sizeof(output)));
    assert_memory_equal(output, temp, sizeof(output));

    uint8_t output2[3] = {0};
    assert_true(buffer_seek_set(&buf, 2));
    assert_true(buffer_copy(&buf, output2, sizeof(output2)));
    assert_memory_equal(output2, ((uint8_t[3]){0x03, 0x04, 0x05}), 3);
    assert_true(buffer_seek_set(&buf, 0));                      // seek at offset 0
    assert_false(buffer_copy(&buf, output2, sizeof(output2)));  // can't read 5 bytes
}

static void test_buffer_move(void **state) {
    (void) state;

    uint8_t output[5] = {0};
    uint8_t temp[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    buffer_t buf = {.ptr = temp, .size = sizeof(temp), .offset = 0};

    assert_true(buffer_move(&buf, output, sizeof(output)));
    assert_memory_equal(output, temp, sizeof(output));
    assert_int_equal(buf.offset, sizeof(output));

    uint8_t output2[3] = {0};
    assert_true(buffer_seek_set(&buf, 0));                      // seek at offset 0
    assert_false(buffer_move(&buf, output2, sizeof(output2)));  // can't read 5 bytes
}

static void test_buffer_move_partial(void **state) {
    (void) state;

    uint8_t output[5] = {0};
    uint8_t temp[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    buffer_t buf = {.ptr = temp, .size = sizeof(temp), .offset = 0};

    assert_true(buffer_move_partial(&buf, output, sizeof(output), 1));
    assert_memory_equal(output, temp, 1);
    assert_int_equal(buf.offset, 1);

    uint8_t output2[3] = {0};
    assert_true(buffer_seek_set(&buf, 0));                      // seek at offset 0
    assert_false(buffer_move_partial(&buf, output2, sizeof(output2), 5));  // can't read 5 bytes
}

static void test_buffer_move_partial_long(void **state) {
    (void) state;

    uint8_t output[132] = {0};
    uint8_t temp[132] = {1};
    buffer_t buf = {.ptr = temp, .size = sizeof(temp), .offset = 0};

    assert_true(buffer_move_partial(&buf, output, sizeof(output), 132));
    assert_memory_equal(output, temp, 132);
    assert_int_equal(buf.offset, 132);
}

static void test_buffer_read_tlv(void **state) {
    (void) state;

    uint8_t output[8] = {0};
    uint8_t temp[] = {
        0x00, // invalid tag
        0x04, 0x00, // empty field
        0x04, 0x85, // invalid length
        0x04, 0x07, 0x65, 0x6e, 0x67, 0x72, 0x61, 0x76, 0x65};

    buffer_t buf = {.ptr = temp, .size = sizeof(temp), .offset = 0};
    
    uint8_t tag = 0;
    uint32_t length = 0;

    // not enought space
    assert_false(buffer_read_tlv(&buf, output, 1, &tag, &length));

    // invalid tag
    assert_true(buffer_seek_set(&buf, 0));                      // seek at offset 0
    assert_false(buffer_read_tlv(&buf, output, sizeof(output), &tag, &length));

    // parse zero length field
    assert_true(buffer_seek_set(&buf, 1));                      // seek at offset 1
    assert_true(buffer_read_tlv(&buf, output, sizeof(output), &tag, &length));
    assert_int_equal(length, 0);
    assert_int_equal(tag, 0x04);

    // invalid length
    assert_true(buffer_seek_set(&buf, 3));                      // seek at offset 3
    assert_false(buffer_read_tlv(&buf, output, sizeof(output), &tag, &length));

    // parse valid string
    assert_true(buffer_seek_set(&buf, 5));                      // seek at offset 5
    assert_true(buffer_read_tlv(&buf, output, sizeof(output), &tag, &length));
    assert_int_equal(length, 0x07);
    assert_int_equal(tag, 0x04);
    assert_string_equal(output, "engrave");
}


static void test_buffer_read_bip32_path(void **state) {
    (void) state;
    
    uint32_t output[5] = {0};
    uint8_t input[20] = {
        0x80, 0x00, 0x00, 0x2C,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    buffer_t buf = {.ptr = input, .offset = 0, .size = sizeof(input)};

    // not enought space
    assert_false(buffer_read_bip32_path(&buf, output, 0));
    
    assert_true(buffer_seek_set(&buf, 0)); // seek at offset 0
    assert_true(buffer_read_bip32_path(&buf, output, sizeof(output) / sizeof(output[0])));

}

int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_buffer_can_read),
                                       cmocka_unit_test(test_buffer_seek),
                                       cmocka_unit_test(test_buffer_read),
                                       cmocka_unit_test(test_buffer_copy),
                                       cmocka_unit_test(test_buffer_move),
                                       cmocka_unit_test(test_buffer_move_partial), 
                                       cmocka_unit_test(test_buffer_move_partial_long), 
                                       cmocka_unit_test(test_buffer_read_tlv),
                                       cmocka_unit_test(test_buffer_read_varint),
                                       cmocka_unit_test(test_buffer_read_bip32_path)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
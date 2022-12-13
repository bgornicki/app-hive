#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

/**
 * Read a protobuf-syle-like varint from byte buffer.
 *
 * @param[in]  in
 *   Pointer to input byte buffer.
 * @param[in]  in_len
 *   Length of the input byte buffer.
 * @param[out] value
 *   Pointer to 64-bit unsigned integer to output varint.
 *
 * @return number of bytes read, -1 otherwise.
 *
 */
int8_t varint_read(const uint8_t *in, size_t in_len, uint64_t *value);

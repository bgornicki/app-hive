#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // int*_t, uint*_t
#include <stdbool.h>  // bool

// in format YYYY-MM-DDTHH:MM:SS
#define DATE_TIME_STR_LEN 20

// 2^64 = 18446744073709551615
#define MAX_U64_LEN 21

// 2^63 - 1 = 9223372036854775807 with sign
#define MAX_I64_LEN 21

// MAX_I64_LEN + dot (for precision) plus symbol_t length
#define MAX_HIVE_ASSET_LEN (MAX_I64_LEN + 1 + 7)

typedef char symbol_t[7];

typedef struct asset_t {
    int64_t amount;
    uint8_t precision;
    symbol_t symbol;
} asset_t;

/**
 * Format EPOCH timestamp as a user friendly datetime string
 *
 * @param[in] timestamp
 *  32-bit unsigned epoch
 * @param[out] out
 *  Pointer to output string
 * @param out_size
 *  Length of output buffer
 * @return true if success, false otherwise.
 */
bool format_timestamp(uint32_t timestamp, char *out, size_t out_size);

/**
 * Format 64-bit signed integer as string.
 *
 * @param[out] dst
 *   Pointer to output string.
 * @param[in]  dst_len
 *   Length of output string.
 * @param[in]  value
 *   64-bit signed integer to format.
 *
 * @return true if success, false otherwise.
 *
 */
bool format_i64(const int64_t i, char *out, uint8_t out_len);

/**
 * Format 64-bit unsigned integer as string.
 *
 * @param[out] dst
 *   Pointer to output string.
 * @param[in]  dst_len
 *   Length of output string.
 * @param[in]  value
 *   64-bit unsigned integer to format.
 *
 * @return true if success, false otherwise.
 *
 */
bool format_u64(const uint64_t i, char *out, uint8_t out_len);

/**
 * Convert serialized asset into readable form and replace STEEM/SBD with HIVE equivalents
 *
 * @param[in] asset
 *  Pointer to serialized asset buffer
 * @param[out] out
 *  Pointer to output buffer
 * @param[in] size
 *  Length of output buffer
 * @return true if success, false otherwise.
 */
bool format_asset(asset_t *asset, char *out, size_t size);

bool format_hash(const uint8_t *hash, size_t hash_len, char *out, size_t out_len);
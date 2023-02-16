/*******************************************************************************
 *   Ledger App Hive
 *   (c) 2021 Bartłomiej (@engrave) Górnicki
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "decoders.h"
#include "globals.h"

#include "common/macros.h"
#include "common/buffer.h"
#include "common/wif.h"
#include "common/read.h"
#include "common/format.h"

/* Hive specific decders to convert DER encoded data to user-friendly form */

#define MAX_OPERATION_NUMBER 50
#define MAX_OPERATION_NAME_LEN 29
#define EXT_TYPE_BENEFICIARIES 0
#define MAX_ACCOUNT_NAME_LEN 64

const char operation_names[MAX_OPERATION_NUMBER][MAX_OPERATION_NAME_LEN] = {
    "vote",                          // 0
    "comment",                       // 1
    "transfer",                      // 2
    "transfer_to_vesting",           // 3
    "withdraw_vesting",              // 4
    "limit_order_create",            // 5
    "limit_order_cancel",            // 6
    "feed_publish",                  // 7
    "convert",                       // 8
    "account_create",                // 9
    "account_update",                // 10
    "witness_update",                // 11
    "account_witness_vote",          // 12
    "account_witness_proxy",         // 13
    "",                              // 14
    "",                              // 15
    "",                              // 16
    "delete_comment",                // 17
    "custom_json",                   // 18
    "comment_options",               // 19
    "set_withdraw_vesting_route",    // 20
    "",                              // 21
    "claim_account",                 // 22
    "create_claimed_account",        // 23
    "request_account_recovery",      // 24
    "recover_account",               // 25
    "change_recovery_account",       // 26
    "",                              // 27
    "",                              // 28
    "",                              // 29
    "",                              // 30
    "",                              // 31
    "transfer_to_savings",           // 32
    "transfer_from_savings",         // 33
    "cancel_transfer_from_savings",  // 34
    "",                              // 35
    "decline_voting_rights",         // 36
    "reset_account",                 // 37
    "set_reset_account",             // 38
    "claim_reward_balance",          // 39
    "delegate_vesting_shares",       // 40
    "",                              // 41
    "",                              // 42
    "",                              // 43
    "create_proposal",               // 44
    "update_proposal_votes",         // 45
    "remove_proposal",               // 46
    "update_proposal",               // 47
    "collateralized_convert",        // 48
    "recurrent_transfer",            // 49
};

/** The only thing to do is to find the operation name in an array */
bool decoder_operation_name(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint8_t op_nr;

    if (!buffer_read_u8(buf, &op_nr)) {
        return false;
    }
    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, &op_nr, 1, NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", operation_names[op_nr]);
    }
    return true;
}

/**
 * Decode string which consist of [length] [n chars]
 */
bool decoder_string(buffer_t *buf, field_t *field, bool should_hash_only) {
    size_t initial_offset = buf->offset;

    uint64_t string_length;
    if (!buffer_read_varint(buf, &string_length)) {
        return false;
    }

    char str[string_length + 1];
    memset(str, 0, sizeof(str));

    if (!buffer_move_partial(buf, (uint8_t *) str, sizeof(str), string_length)) {
        return false;
    }

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, buf->ptr + initial_offset, buf->offset - initial_offset, NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", str);
    }
    return true;
}

bool decoder_array_of_strings(buffer_t *buf, field_t *field, bool should_hash_only) {
    size_t initial_offset = buf->offset;

    uint64_t number_of_items;
    if (!buffer_read_varint(buf, &number_of_items)) {
        return false;
    }

    uint32_t max_string_length = MEMBER_SIZE(field_t, value);

    char str[max_string_length];
    memset(str, 0, max_string_length);

    char tmp[50];

    snprintf(str, max_string_length, "[ ");

    for (uint8_t i = 0; i < number_of_items; i++) {
        memset(tmp, 0, sizeof(tmp));
        uint64_t string_length;
        if (!buffer_read_varint(buf, &string_length) || string_length >= sizeof(tmp) ||
            !buffer_move_partial(buf, (uint8_t *) tmp, sizeof(tmp), string_length)) {
            return false;
        }
        snprintf(str + strlen(str), sizeof(str) - strlen(str), i == number_of_items - 1 ? "%s" : "%s, ", tmp);
    }

    snprintf(str + strlen(str), sizeof(str) - strlen(str), " ]");

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, buf->ptr + initial_offset, buf->offset - initial_offset, NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", str);
    }

    return true;
}

bool decoder_array_of_u64(buffer_t *buf, field_t *field, bool should_hash_only) {
    size_t initial_offset = buf->offset;
    uint32_t max_string_length = MEMBER_SIZE(field_t, value);
    char u64_str[MAX_U64_LEN];
    char str[max_string_length];
    uint64_t number_of_items;
    uint64_t proposal_id;

    memset(str, 0, max_string_length);

    if (!buffer_read_varint(buf, &number_of_items)) {
        return false;
    }

    snprintf(str, max_string_length, "[ ");

    for (uint8_t i = 0; i < number_of_items; i++) {
        memset(u64_str, 0, ARRAYLEN(u64_str));
        if (!buffer_read_u64(buf, &proposal_id, LE) || !format_u64(proposal_id, u64_str, ARRAYLEN(u64_str))) {
            return false;
        }
        snprintf(str + strlen(str), max_string_length - strlen(str), i == number_of_items - 1 ? "%s" : "%s, ", u64_str);
    }

    snprintf(str + strlen(str), max_string_length - strlen(str), " ]");

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, buf->ptr + initial_offset, buf->offset - initial_offset, NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", str);
    }

    return true;
}

/**
 * Decode boolean value
 */
bool decoder_boolean(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint8_t value;
    if (!buffer_read_u8(buf, &value)) {
        return false;
    }

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, &value, 1, NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", value ? "true" : "false");
    }
    return true;
}

bool decoder_date_time(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint32_t timestamp;
    if (!buffer_read_u32(buf, &timestamp, LE)) {
        return false;
    }

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, (uint8_t *) &timestamp, sizeof(timestamp), NULL, 0);
    } else {
        if (!format_timestamp(timestamp, field->value, MEMBER_SIZE(field_t, value))) {
            return false;
        }
    }
    return true;
}

bool decoder_public_key(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint8_t value[PUBKEY_COMPRESSED_LEN] = {0};
    char wif[PUBKEY_WIF_STR_LEN] = {0};

    if (!buffer_move_partial(buf, value, PUBKEY_COMPRESSED_LEN, PUBKEY_COMPRESSED_LEN) ||
        !wif_from_compressed_public_key((uint8_t *) value, PUBKEY_COMPRESSED_LEN, wif, PUBKEY_WIF_STR_LEN)) {
        return false;
    }

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, (uint8_t *) value, PUBKEY_COMPRESSED_LEN, NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", wif);
    }
    return true;
}

bool decoder_asset(buffer_t *buf, field_t *field, bool should_hash_only) {
    asset_t asset = {0};
    if (!buffer_move_partial(buf, (uint8_t *) &asset, sizeof(asset_t), sizeof(asset_t))) {
        return false;
    }
    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, (uint8_t *) &asset, sizeof(asset_t), NULL, 0);
    } else {
        if (!format_asset(&asset, field->value, MEMBER_SIZE(field_t, value))) {
            return false;
        }
    }
    return true;
}

bool decoder_weight(buffer_t *buf, field_t *field, bool should_hash_only) {
    int16_t weight;
    if (!buffer_read_u16(buf, (uint16_t *) &weight, LE)) {
        return false;
    }
    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, (uint8_t *) &weight, sizeof(weight), NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%d.%02d%%", weight / 100, abs(weight) % 100);
    }
    return true;
}

bool decoder_uint32(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint32_t value;
    if (!buffer_read_u32(buf, &value, LE)) {
        return false;
    }
    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, (uint8_t *) &value, sizeof(value), NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%d", value);
    }
    return true;
}

bool decoder_uint64(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint64_t value;
    if (!buffer_read_u64(buf, &value, LE)) {
        return false;
    }
    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, (uint8_t *) &value, sizeof(value), NULL, 0);
    } else {
        char u64_str[MAX_U64_LEN];
        format_u64(value, u64_str, ARRAYLEN(u64_str));
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", u64_str);
    }
    return true;
}

bool decoder_uint16(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint16_t value;
    if (!buffer_read_u16(buf, &value, LE)) {
        return false;
    }
    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, (uint8_t *) &value, sizeof(value), NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%d", value);
    }
    return true;
}

bool decoder_uint8(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint8_t value;
    if (!buffer_read_u8(buf, &value)) {
        return false;
    }
    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, &value, sizeof(value), NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%d", value);
    }
    return true;
}

bool decoder_authority_type(buffer_t *buf, field_t *field, bool should_hash_only) {
    size_t initial_offset = buf->offset;

    uint64_t count;
    uint16_t threshold;
    uint32_t weight;

    uint8_t tmp[MAX_ACCOUNT_NAME_LEN] = {0};
    char value[MEMBER_SIZE(field_t, value)] = {0};
    char wif[PUBKEY_WIF_STR_LEN + 1] = {0};

    // weight_threshold
    if (!buffer_read_u32(buf, &weight, LE) || !buffer_read_varint(buf, &count)) {
        return false;
    }

    snprintf(value, sizeof(value), "Weight: %d, [ ", weight);

    // account_auths count
    for (uint8_t i = 0; i < count; i++) {
        memset(tmp, 0, sizeof(tmp));

        uint64_t string_length;

        // clang-format off
    if (!buffer_read_varint(buf, &string_length) || 
        string_length >= sizeof(tmp) || 
        !buffer_move_partial(buf, tmp, sizeof(tmp), string_length) ||
        !buffer_read_u16(buf, &threshold, LE)) {
        return false;
    }
        // clang-format on

        snprintf(value + strlen(value), sizeof(value) - strlen(value), i == count - 1 ? "[ %s, %d ]" : "[ %s, %d ], ", tmp, threshold);
    }

    snprintf(value + strlen(value), sizeof(value) - strlen(value), " ], [ ");

    // key_auths
    if (!buffer_read_varint(buf, &count)) {
        return false;
    }

    for (uint8_t i = 0; i < count; i++) {
        memset(tmp, 0, sizeof(tmp));
        memset(wif, 0, sizeof(wif));

        if (!buffer_move_partial(buf, tmp, sizeof(tmp), PUBKEY_COMPRESSED_LEN) || !buffer_read_u16(buf, &threshold, LE) ||
            !wif_from_compressed_public_key((uint8_t *) tmp, PUBKEY_COMPRESSED_LEN, wif, PUBKEY_WIF_STR_LEN)) {
            return false;
        }

        snprintf(value + strlen(value), sizeof(value) - strlen(value), i == count - 1 ? "[ %s, %d ]" : "[ %s, %d ], ", wif, threshold);
    }

    snprintf(value + strlen(value), sizeof(value) - strlen(value), " ]");

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, buf->ptr + initial_offset, buf->offset - initial_offset, NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", value);
    }
    return true;
}

bool decoder_optional_authority_type(buffer_t *buf, field_t *field, bool should_hash_only) {
    size_t initial_offset = buf->offset;

    uint8_t count;
    uint16_t threshold;
    uint32_t weight;

    uint8_t tmp[MAX_ACCOUNT_NAME_LEN] = {0};
    char value[MEMBER_SIZE(field_t, value)] = {0};
    char wif[PUBKEY_WIF_STR_LEN + 1] = {0};

    // this field may be optional so we need to check first byte
    if (!buffer_read_u8(buf, &count)) {
        return false;
    }

    if (count != 0) {
        // weight_threshold
        if (!buffer_read_u32(buf, &weight, LE) || !buffer_read_u8(buf, &count)) {
            return false;
        }

        snprintf(value, sizeof(value), "Weight: %d, [ ", weight);

        // account_auths count
        for (uint8_t i = 0; i < count; i++) {
            memset(tmp, 0, sizeof(tmp));

            uint8_t string_length;

            // clang-format off
        if (!buffer_read_u8(buf, &string_length) || 
            string_length >= sizeof(tmp) || 
            !buffer_move_partial(buf, tmp, sizeof(tmp), string_length) ||
            !buffer_read_u16(buf, &threshold, LE)) {
            return false;
        }
            // clang-format on

            snprintf(value + strlen(value), sizeof(value) - strlen(value), i == count - 1 ? "[ %s, %d ]" : "[ %s, %d ], ", tmp, threshold);
        }

        snprintf(value + strlen(value), sizeof(value) - strlen(value), " ], [ ");

        // key_auths
        if (!buffer_read_u8(buf, &count)) {
            return false;
        }

        for (uint8_t i = 0; i < count; i++) {
            memset(tmp, 0, sizeof(tmp));
            memset(wif, 0, sizeof(wif));

            if (!buffer_move_partial(buf, tmp, sizeof(tmp), PUBKEY_COMPRESSED_LEN) || !buffer_read_u16(buf, &threshold, LE) ||
                !wif_from_compressed_public_key((uint8_t *) tmp, PUBKEY_COMPRESSED_LEN, wif, PUBKEY_WIF_STR_LEN)) {
                return false;
            }

            snprintf(value + strlen(value), sizeof(value) - strlen(value), i == count - 1 ? "[ %s, %d ]" : "[ %s, %d ], ", wif, threshold);
        }

        snprintf(value + strlen(value), sizeof(value) - strlen(value), " ]");
    } else {
        snprintf(value, sizeof(value), "no changes");
    }

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, buf->ptr + initial_offset, buf->offset - initial_offset, NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", value);
    }
    return true;
}

bool decoder_empty_extensions(buffer_t *buf, field_t *field, bool should_hash_only) {
    uint8_t size;
    if (!buffer_read_u8(buf, &size) || size != 0) {
        return false;
    }

    if (should_hash_only) {
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, &size, sizeof(size), NULL, 0);
    } else {
        snprintf(field->value, MEMBER_SIZE(field_t, value), "[ ]");
    }
    return true;
}

bool decoder_beneficiaries_extensions(buffer_t *buf, field_t *field, bool should_hash_only) {
    size_t initial_offset = buf->offset;
    uint64_t size, account_name_len, type, beneficiaries;
    uint16_t weight;
    char account_name[MAX_ACCOUNT_NAME_LEN + 1] = {0};
    char value[MEMBER_SIZE(field_t, value)] = {0};

    if (!buffer_read_varint(buf, &size)) {
        return false;
    }

    if (size == 0) {
        if (should_hash_only) {
            cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, buf->ptr + initial_offset, buf->offset - initial_offset, NULL, 0);
        } else {
            snprintf(field->value, MEMBER_SIZE(field_t, value), "[ ]");
        }
    } else if (size == 1) {
        if (!buffer_read_varint(buf, &type) || type != EXT_TYPE_BENEFICIARIES) {  // only allow beneficiaries extension
            return false;
        }

        if (!buffer_read_varint(buf, &beneficiaries)) {
            return false;
        }

        snprintf(value, sizeof(value), "Beneficiaries: [");

        for (uint64_t i = 0; i < beneficiaries; i++) {
            // clang-format off
            if (!buffer_read_varint(buf, &account_name_len) || 
                account_name_len >= sizeof(value) || 
                !buffer_move_partial(buf, (uint8_t *)account_name, sizeof(account_name) - 1, account_name_len) ||
                !buffer_read_u16(buf, &weight, LE)) {
                return false;
            }
            // clang-format on

            snprintf(value + strlen(value),
                     sizeof(value) - strlen(value),
                     i == beneficiaries - 1 ? "%s: %d.%02d%%" : "%s: %d.%02d%%, ",
                     account_name,
                     weight / 100,
                     weight % 100);
        }

        snprintf(value + strlen(value), sizeof(value) - strlen(value), "]");

        if (should_hash_only) {
            cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, buf->ptr + initial_offset, buf->offset - initial_offset, NULL, 0);
        } else {
            snprintf(field->value, MEMBER_SIZE(field_t, value), "%s", value);
        }

    } else {
        // not supported
        return false;
    }

    return true;
}
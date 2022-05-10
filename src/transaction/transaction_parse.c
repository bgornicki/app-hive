/*****************************************************************************
 *   Ledger App Hive.
 *   (c) 2020 Ledger SAS.
 *   Modifications (c) 2021 Bartłomiej (@engrave) Górnicki
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
 *****************************************************************************/

#include "transaction_parse.h"
#include "parsers.h"
#include "constants.h"
#include "decoders.h"
#include "globals.h"
#include "common/buffer.h"
#include "common/asn1.h"
#include "common/bip32.h"

/**
 * Parse DER encoded transacion, validate and hash
 * */
parser_status_e transaction_parse(buffer_t *buf) {
    uint8_t data[MAX_TRANSACTION_LEN];
    uint8_t tag;
    uint32_t length;

    if (buf->size > MAX_TRANSACTION_LEN) {
        return WRONG_LENGTH_ERROR;
    }

    /* Parse:
     *  - BIP32 path
     */
    if (!buffer_read_u8(buf, &G_context.bip32_path_len) || !buffer_read_bip32_path(buf, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return BIP32_PATH_PARSING_ERROR;
    }

    /* Parse and hash:
     *  - chain id
     *  - ref_block_num
     *  - ref_block_prefix
     *  - expiration
     *  - operations_count
     */
    for (uint8_t i = 0; i < 5; i++) {
        memset(data, 0, sizeof(data));
        if (!buffer_read_tlv(buf, data, sizeof(data), &tag, &length)) {
            return FIELD_PARSING_ERROR;
        }
        cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, data, length, NULL, 0);
    }

    if (data[0] != 1) {
        // Only one operation per tx is supported for now
        return OPERATION_COUNT_PARSING_ERROR;
    }

    /* Parse and hash:
     *  - DER encoded operations
     */
    memset(data, 0, sizeof(data));
    if (!buffer_read_tlv(buf, data, sizeof(data), &tag, &length)) {
        return FIELD_PARSING_ERROR;
    }

    G_context.tx_info.operation = (buffer_t){.ptr = G_context.tx_info.raw_tx + buf->offset - length, .size = length, .offset = 0};
    G_context.tx_info.parser = get_operation_parser(G_context.tx_info.operation.ptr[0]);

    // Hash operation
    for (uint8_t i = 0; i < G_context.tx_info.parser->size; i++) {
        decoder_t *fun = (decoder_t *) PIC(G_context.tx_info.parser->decoders[i]);

        // check if it's correct
        bool result = (*fun)(&G_context.tx_info.operation, NULL, true);
        if (!result) {
            return FIELD_PARSING_ERROR;
        }
    }

    G_context.tx_info.operation.offset = 0;

    /* Parse:
     *  - extensions
     */
    memset(data, 0, sizeof(data));
    if (!buffer_read_tlv(buf, data, sizeof(data), &tag, &length)) {
        return FIELD_PARSING_ERROR;
    }

    cx_hash((cx_hash_t *) &G_context.tx_info.sha, 0, data, 1, NULL, 0);

    return (buf->offset == buf->size) ? PARSING_OK : WRONG_LENGTH_ERROR;
}

/**
 * Parse transaction hash and path
 * */
parser_status_e hash_parse(buffer_t *buf) {
    /* Parse:
     *  - BIP32 path
     */
    if (!buffer_read_u8(buf, &G_context.bip32_path_len) || !buffer_read_bip32_path(buf, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return BIP32_PATH_PARSING_ERROR;
    }
    /* Parse:
     *  - sha256 hash
     */
    if (!buffer_move(buf, G_context.hash_info.hash, MEMBER_SIZE(hash_ctx_t, hash))) {
        return WRONG_LENGTH_ERROR;
    }

    return PARSING_OK;
}

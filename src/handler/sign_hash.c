/*****************************************************************************
 *   Ledger App Hive.
 *   (c) 2022 Bartłomiej (@engrave) Górnicki
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

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"

#include "sign_tx.h"
#include "sw.h"
#include "globals.h"
#include "crypto.h"
#include "ui/screens/review_transaction.h"
#include "common/buffer.h"
#include "apdu/dispatcher.h"

int handler_sign_hash(buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_HASH;
    G_context.state = STATE_NONE;

    cx_sha256_init(&G_context.tx_info.sha);

    // TODO parse hash, check length etc

    G_context.state = STATE_PARSED;
    ////

    if (chunk == P1_FIRST_CHUNK) {  // first chunk

        if (!buffer_move(cdata, G_context.tx_info.raw_tx, MAX_TRANSACTION_LEN)) {
            return io_send_sw(SW_WRONG_TX_LENGTH);
        }

        G_context.tx_info.raw_tx_len += cdata->size;

        if (!more) {
            buffer_t tx = {0};

            tx.offset = 0;
            tx.ptr = G_context.tx_info.raw_tx;
            tx.size = cdata->size;
            const parser_status_e status = transaction_parse(&tx);

            if (status != PARSING_OK) {
                return io_send_sw(SW_TX_PARSING_FAIL);
            }

            G_context.state = STATE_PARSED;

            return ui_display_transaction();
        } else {
            G_context.state = STATE_TX_RECEIVING;
            // will be more, just return OK
            return io_send_sw(SW_OK);
        }

    } else {
        if (G_context.state != STATE_TX_RECEIVING) {
            return io_send_sw(SW_BAD_STATE);
        }

        // get subsequent chunk
        if (!buffer_move(cdata, G_context.tx_info.raw_tx + G_context.tx_info.raw_tx_len, MAX_TRANSACTION_LEN - G_context.tx_info.raw_tx_len)) {
            return io_send_sw(SW_WRONG_TX_LENGTH);
        }

        G_context.tx_info.raw_tx_len += cdata->size;

        if (!more) {
            buffer_t tx = {0};
            tx.offset = 0;
            tx.ptr = G_context.tx_info.raw_tx;
            tx.size = G_context.tx_info.raw_tx_len;

            const parser_status_e status = transaction_parse(&tx);

            if (status != PARSING_OK) {
                return io_send_sw(SW_TX_PARSING_FAIL);
            }

            G_context.state = STATE_PARSED;

            return ui_display_transaction();
        } else {
            G_context.state = STATE_TX_RECEIVING;
            // will be more, just return OK
            return io_send_sw(SW_OK);
        }
    }

    return 0;
}

/*****************************************************************************
 *   Ledger App Hive.
 *   (c) 2023 Bartłomiej (@engrave) Górnicki
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

#include "sign_message.h"
#include "sw.h"
#include "globals.h"
#include "crypto.h"
#include "ui/screens/review_message.h"
#include "common/buffer.h"
#include "apdu/dispatcher.h"

int handler_sign_message(buffer_t *cdata, uint8_t chunk, bool more) {
    if (chunk == P1_FIRST_CHUNK) {  // first chunk

        if (G_context.state != STATE_NONE) {
            return io_send_sw(SW_BAD_STATE);
        }

        explicit_bzero(&G_context, sizeof(G_context));
        G_context.req_type = CONFIRM_MESSAGE;
        G_context.state = STATE_NONE;
        cx_sha256_init(&G_context.message_info.sha);

        if (!buffer_move(cdata, G_context.message_info.data, MAX_RAW_MESSAGE_LEN)) {
            return io_send_sw(SW_WRONG_MESSAGE_LENGTH);
        }

        G_context.message_info.data_len += cdata->size;

        if (!more) {
            buffer_t msg = {0};

            msg.offset = 0;
            msg.ptr = G_context.message_info.data;
            msg.size = cdata->size;

            const parser_status_e status = message_parse(&msg);

            if (status != PARSING_OK) {
                return io_send_sw(SW_MESSAGE_PARSING_FAIL);
            }

            G_context.state = STATE_PARSED;

            return ui_display_message();
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
        if (!buffer_move(cdata, G_context.message_info.data + G_context.message_info.data_len, MAX_RAW_MESSAGE_LEN - G_context.message_info.data_len)) {
            return io_send_sw(SW_WRONG_MESSAGE_LENGTH);
        }

        G_context.message_info.data_len += cdata->size;

        if (!more) {
            buffer_t msg = {0};

            msg.offset = 0;
            msg.ptr = G_context.message_info.data;
            msg.size = G_context.message_info.data_len;

            const parser_status_e status = message_parse(&msg);

            if (status != PARSING_OK) {
                return io_send_sw(SW_MESSAGE_PARSING_FAIL);
            }

            G_context.state = STATE_PARSED;

            return ui_display_message();
        } else {
            G_context.state = STATE_TX_RECEIVING;
            // will be more, just return OK
            return io_send_sw(SW_OK);
        }
    }

    return 0;
}

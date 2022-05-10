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
#include "ui/screens/review_hash.h"
#include "ui/screens/settings.h"
#include "common/buffer.h"
#include "apdu/dispatcher.h"

int handler_sign_hash(buffer_t *cdata) {
    if (G_context.state != STATE_NONE) {
        return io_send_sw(SW_BAD_STATE);
    }

    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_HASH;
    G_context.state = STATE_NONE;

    // test APDU D41000000401
    // test APDU d41000003505800000308000000d8000000080000000800000005fd924625f6ab16a19cc9807c7c506ae1813490e4ba675f843d5a10e0baacdb8

    if (N_settings.sign_hash_policy == DISABLED) {
        ui_display_hash_signing_disabled_warning();
        return io_send_sw(SW_HASH_SIGNING_DISABLED);
    }

    const parser_status_e status = hash_parse(cdata);

    if (status != PARSING_OK) {
        return io_send_sw(SW_HASH_PARSING_FAIL);
    }

    G_context.state = STATE_PARSED;

    return ui_display_hash();
}

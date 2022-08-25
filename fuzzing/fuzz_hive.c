#include "transaction/transaction_parse.h"

#include <stddef.h>
#include <stdint.h>

#include "globals.h"
/*
#include "apdu/parser.h"
#include "apdu/dispatcher.h"
#include "cx.h"
*/

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 5) {
        //mimic buf_len < OFFSET_CDATA in apdu_parser()
        return 1;
    }
    BEGIN_TRY {
        TRY {
            uint8_t lc;

            lc = Data[0];

            explicit_bzero(&G_context, sizeof(G_context));
            G_context.req_type = CONFIRM_TRANSACTION;
            G_context.state = STATE_NONE;

            G_context.tx_info.raw_tx_len = Size - 1 > MAX_TRANSACTION_LEN ? MAX_TRANSACTION_LEN : Size - 1;
            memcpy(G_context.tx_info.raw_tx, Data + 1, G_context.tx_info.raw_tx_len);
            buffer_t tx_buffer = {.offset = 0, .ptr = &G_context.tx_info.raw_tx, .size = lc};

            transaction_parse(&tx_buffer);

            lc = Data[0];

            explicit_bzero(&G_context, sizeof(G_context));
            G_context.req_type = CONFIRM_HASH;
            G_context.state = STATE_NONE;

            G_context.tx_info.raw_tx_len = Size - 1 > MAX_TRANSACTION_LEN ? MAX_TRANSACTION_LEN : Size - 1;
            memcpy(G_context.tx_info.raw_tx, Data + 1, G_context.tx_info.raw_tx_len);
            buffer_t hash_buffer = {.offset = 0, .ptr = &G_context.tx_info.raw_tx, .size = lc};

            hash_parse(&hash_buffer);
        }
        CATCH_OTHER(e) {
            return 0;
        }
        FINALLY {
        }
        END_TRY;
    }

    return 0;
}

/*
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    command_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    if (!apdu_parser(&cmd, Data, Size)) {
        return 0;
    }

    // Dispatch structured APDU command to handler
    if (apdu_dispatcher(&cmd) < 0) {
        return 0;
    }
    return 0;
}
*/

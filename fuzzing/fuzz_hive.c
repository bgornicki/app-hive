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
    BEGIN_TRY {
        TRY {
            explicit_bzero(&G_context, sizeof(G_context));
            G_context.req_type = CONFIRM_TRANSACTION;
            G_context.state = STATE_NONE;

            G_context.tx_info.raw_tx_len = Size > MAX_TRANSACTION_LEN ? MAX_TRANSACTION_LEN : Size;
            memcpy(G_context.tx_info.raw_tx, Data, G_context.tx_info.raw_tx_len);

            buffer_t buf = {.offset = 0, .ptr = &G_context.tx_info.raw_tx, .size = G_context.tx_info.raw_tx_len};

            transaction_parse(&buf);
        }
        CATCH(EXCEPTION_IO_RESET) {
            return 0;
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

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

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "ui/screens/review_transaction.h"

static action_validate_cb g_validate_callback;
static char g_bip32_path[60];
static enum e_state g_current_state;
static int8_t g_tx_field_position;
static field_t g_tx_field_parsed;

// This is a special function you must call for bn_paging to work properly in an edgecase.
// It does some weird stuff with the `G_ux` global which is defined by the SDK.
// No need to dig deeper into the code, a simple copy paste will do.
void bn_paging_edgecase() {
    G_ux.flow_stack[G_ux.stack_count - 1].prev_index = G_ux.flow_stack[G_ux.stack_count - 1].index - 2;
    G_ux.flow_stack[G_ux.stack_count - 1].index--;
    ux_flow_relayout();
}

#ifdef TARGET_NANOS

// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_tx_path_step,
             bn_paging,
             {
                 .title = "Signing key path",
                 .text = g_bip32_path,
             });

// Step with title/text for transaction field
UX_STEP_NOCB(ux_display_tx_field_step,
             bn_paging,
             {
                 .title = g_tx_field_parsed.title,
                 .text = g_tx_field_parsed.value,
             });

#else

// For Nano X utilize all three lines of text

// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_tx_path_step,
             bnnn_paging,
             {
                 .title = "Signing key path",
                 .text = g_bip32_path,
             });

// Step with title/text for transaction field
UX_STEP_NOCB(ux_display_tx_field_step,
             bnnn_paging,
             {
                 .title = g_tx_field_parsed.title,
                 .text = g_tx_field_parsed.value,
             });

#endif

// Step with approve button
UX_STEP_CB(ux_display_tx_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_tx_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

// Note we're using UX_STEP_INIT because this step won't display anything.
UX_STEP_INIT(step_upper_delimiter, NULL, NULL, {
    // This function will be detailed later on.
    display_next_state(true);
});

// Note we're using UX_STEP_INIT because this step won't display anything.
UX_STEP_INIT(step_lower_delimiter, NULL, NULL, {
    // This function will be detailed later on.
    display_next_state(false);
});

// Step with icon and text
UX_STEP_NOCB(ux_display_review_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "Transaction",
             });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 n screens : display transaction field
// #3 screen : approve button
// #4 screen : reject button
UX_FLOW(ux_display_transaction_flow,
        &ux_display_review_step,
        &ux_display_tx_path_step,
        &step_upper_delimiter,
        &ux_display_tx_field_step,
        &step_lower_delimiter,
        &ux_display_tx_approve_step,
        &ux_display_tx_reject_step,
        FLOW_LOOP);

// Transaction signing message step
UX_STEP_NOCB(ux_display_signing_step,
             pnn,
             {
                 &C_icon_processing,
                 "Signing",
                 "Transaction",
             });

// FLOW to display transaction signing message:
// #1 screen : eye processing + "Signing Transaction"
UX_FLOW(ux_display_signing_tx_flow, &ux_display_signing_step);

void ui_display_signing_message() {
    ux_flow_init(0, ux_display_signing_tx_flow, NULL);
}

int ui_display_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_bip32_path, 0, sizeof(g_bip32_path));
    if (!bip32_path_format(G_context.bip32_path, G_context.bip32_path_len, g_bip32_path, sizeof(g_bip32_path))) {
        return io_send_sw(SW_WRONG_BIP32_PATH);
    }

    memset(&g_tx_field_parsed, 0, sizeof(field_t));

    g_tx_field_position = -1;
    g_validate_callback = &ui_action_validate_transaction;
    g_current_state = STATIC_SCREEN;

    ux_flow_init(0, ux_display_transaction_flow, NULL);

    return 0;
}

void display_next_state(bool is_upper_delimiter) {
    if (is_upper_delimiter) {
        if (g_current_state == STATIC_SCREEN) {
            // make sure we start from the first field
            g_tx_field_position = -1;
            bool more_data = parse_field(&g_tx_field_parsed, false, false);
            if (more_data) {
                // We found some data to display so we now enter in dynamic mode.
                g_current_state = DYNAMIC_SCREEN;
            }
            // Move to the next step, which will display the screen.
            ux_flow_next();
        } else {
            // The previous screen was NOT a static screen, so we were already in a dynamic screen.
            // Fetch new data.
            bool more_data = parse_field(&g_tx_field_parsed, true, false);
            if (more_data) {
                // We found some data so simply display it.
                ux_flow_next();
            } else {
                // There's no more dynamic data to display, so
                // update the current state accordingly.
                g_current_state = STATIC_SCREEN;

                // Display the previous screen which should be a static one.
                ux_flow_prev();
            }
        }
    } else {
        // We're called from the lower delimiter.
        if (g_current_state == STATIC_SCREEN) {
            // Fetch new data
            bool more_data = parse_field(&g_tx_field_parsed, true, true);
            if (more_data) {
                // We found some data to display so enter in dynamic mode.
                g_current_state = DYNAMIC_SCREEN;
            }

            // Display the data.
            ux_flow_prev();
        } else {
            // We're being called from a dynamic screen, so the user was already browsing the array.
            bool more_data = parse_field(&g_tx_field_parsed, false, false);
            if (more_data) {
                // We found some data, so display it.
                // Similar to `ux_flow_prev()` but updates layout to account for `bn_paging`'s
                // weird behaviour.
                bn_paging_edgecase();
            } else {
                // We found no data so make sure we update the state accordingly.
                g_current_state = STATIC_SCREEN;

                // Display the next screen
                ux_flow_next();
            }
        }
    }
}

bool parse_field(field_t *field, bool reverse_order, bool start_from_last_operation) {
    // Because encoded fields have various lenght and we want to be able to parse fields in reverse
    // order, we need to iterate decoders from the very beginning up to the
    // g_tx_field_position

    // Edge case, when we want to start from the last operation
    if (start_from_last_operation) {
        g_tx_field_position = G_context.tx_info.parser->size;
    }

    if (reverse_order) {
        if (g_tx_field_position >= 0) {
            g_tx_field_position--;
        } else {
            return false;
        }
    } else {
        if (g_tx_field_position < G_context.tx_info.parser->size - 1) {
            g_tx_field_position++;
        } else {
            return false;
        }
    }

    // Always start parsing the buffer from the beginning
    G_context.tx_info.operation.offset = 0;

    // Display field value
    for (uint8_t i = 0; i < g_tx_field_position + 1; i++) {
        /* Use PIC macro to access const functions (stored in .text area) */
        decoder_t *decoder = (decoder_t *) PIC(G_context.tx_info.parser->decoders[i]);
        // We dont need to validate the return code because at this point we're already sure the transaction parses correctly
        (*decoder)(&G_context.tx_info.operation, field, false);
    }

    // Display field name
    snprintf(field->title, MEMBER_SIZE(field_t, title), "%s", G_context.tx_info.parser->names[g_tx_field_position]);

    if (reverse_order) {
        if (g_tx_field_position == -1) {
            // no more data
            return false;
        }
    } else {
        if (g_tx_field_position == G_context.tx_info.parser->size) {
            // no more data
            return false;
        }
    }

    return true;
}
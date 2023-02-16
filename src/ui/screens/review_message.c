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

#include <stdbool.h>
#include <string.h>

#include "ui/screens/review_message.h"
#include "ui/screens/variables.h"

#ifdef TARGET_NANOS
// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_message_path_step,
             bn_paging,
             {
                 .title = "Signing key path",
                 .text = g_bip32_path,
             });

// For Nano X and S+ utilize all three lines of text
#else
// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_message_path_step,
             bnnn_paging,
             {
                 .title = "Signing key path",
                 .text = g_bip32_path,
             });
#endif

#ifdef TARGET_NANOS
// Step with message content
UX_STEP_NOCB(ux_display_message_step,
             bn_paging,
             {
                 .title = "Message",
                 .text = g_screen_text.value,
             });

// For Nano X and S+ utilize all three lines of text
#else
// Step with message content
UX_STEP_NOCB(ux_display_message_step,
             bnnn_paging,
             {
                 .title = "Message",
                 .text = g_screen_text.value,
             });
#endif

// Step with approve button
UX_STEP_CB(ux_display_message_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });

// Step with reject button
UX_STEP_CB(ux_display_message_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

// Step with icon and text
UX_STEP_NOCB(ux_display_review_message_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "message",
             });

// FLOW to display message:
// #1 screen : eye icon + "Review message"
// #2 screen : display BIP32 path
// #3 n screens : display message content
// #4 screen : approve button
// #5 screen : reject button
UX_FLOW(ux_display_message_flow,
        &ux_display_review_message_step,
        &ux_display_message_path_step,
        &ux_display_message_step,
        &ux_display_message_approve_step,
        &ux_display_message_reject_step,
        FLOW_LOOP);

// Message signing message step
UX_STEP_NOCB(ux_display_signing_message_step,
             pnn,
             {
                 &C_icon_processing,
                 "Signing",
                 "message",
             });

// FLOW to display message signing message:
// #1 screen : eye processing + "Signing message"
UX_FLOW(ux_display_signing_message_flow, &ux_display_signing_message_step);

void ui_display_signing_message_message() {
    ux_flow_init(0, ux_display_signing_message_flow, NULL);
}

int ui_display_message() {
    if (G_context.req_type != CONFIRM_MESSAGE || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_bip32_path, 0, sizeof(g_bip32_path));
    if (!bip32_path_format(G_context.bip32_path, G_context.bip32_path_len, g_bip32_path, sizeof(g_bip32_path))) {
        return io_send_sw(SW_WRONG_BIP32_PATH);
    }

    memset(&g_screen_text, 0, sizeof(field_t));
    snprintf(g_screen_text.value, sizeof(g_screen_text.value), "%s", G_context.message_info.string.ptr + G_context.message_info.string.offset);

    g_validate_callback = &ui_action_validate_message;

    ux_flow_init(0, ux_display_message_flow, NULL);

    return 0;
}
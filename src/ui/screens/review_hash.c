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

#include <stdbool.h>
#include <string.h>

#include "ui/screens/review_hash.h"

static action_validate_cb g_validate_callback;
static char g_bip32_path[60];
static char g_hash[DIGEST_LEN * 2 + 1];

#ifdef TARGET_NANOS
// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_hash_path_step,
             bn_paging,
             {
                 .title = "Signing key path",
                 .text = g_bip32_path,
             });

// For Nano X and S+ utilize all three lines of text
#else
// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_hash_path_step,
             bnnn_paging,
             {
                 .title = "Signing key path",
                 .text = g_bip32_path,
             });
#endif

#ifdef TARGET_NANOS
// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_hash_value_step,
             bn_paging,
             {
                 .title = "Hash",
                 .text = g_hash,
             });

// For Nano X and S+ utilize all three lines of text
#else
// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_hash_value_step,
             bnnn_paging,
             {
                 .title = "Hash",
                 .text = g_hash,
             });
#endif

// Step with approve button
UX_STEP_CB(ux_display_hash_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_hash_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

// Step with icon and text
UX_STEP_NOCB(ux_display_review_hash_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "Hash",
             });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review hash"
// #2 n screens : display transaction field
// #3 screen : approve button
// #4 screen : reject button
UX_FLOW(ux_display_hash_flow,
        &ux_display_review_hash_step,
        &ux_display_hash_path_step,
        &ux_display_hash_value_step,
        &ux_display_hash_approve_step,
        &ux_display_hash_reject_step,
        FLOW_LOOP);

// Transaction signing message step
UX_STEP_NOCB(ux_display_signing_hash_step,
             pnn,
             {
                 &C_icon_processing,
                 "Signing",
                 "hash",
             });

// FLOW to display transaction signing message:
// #1 screen : eye processing + "Signing hash"
UX_FLOW(ux_display_signing_hash_flow, &ux_display_signing_hash_step);

void ui_display_signing_hash_message() {
    ux_flow_init(0, ux_display_signing_hash_flow, NULL);
}

int ui_display_hash() {
    if (G_context.req_type != CONFIRM_HASH || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_bip32_path, 0, sizeof(g_bip32_path));
    if (!bip32_path_format(G_context.bip32_path, G_context.bip32_path_len, g_bip32_path, sizeof(g_bip32_path))) {
        return io_send_sw(SW_WRONG_BIP32_PATH);
    }

    if (!format_hash(G_context.hash_info.hash, MEMBER_SIZE(hash_ctx_t, hash), g_hash, sizeof(g_hash))) {
        return io_send_sw(SW_WRONG_HASH_LENGTH);
    }

    g_validate_callback = &ui_action_validate_hash;

    ux_flow_init(0, ux_display_hash_flow, NULL);

    return 0;
}
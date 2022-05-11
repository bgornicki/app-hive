/*****************************************************************************
 *   Ledger App Hive.
 *   (c) 2020 Ledger SAS.
 *   Modifications (c) Bartłomiej @engrave Górnicki
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

#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "ui/screens/confirm_public_key.h"

static action_validate_cb g_validate_callback;
static char g_bip32_path[60];

// Step with icon and text
UX_STEP_NOCB(ux_display_confirm_public_key_step, pn, {&C_icon_eye, "Confirm Public key"});

// For Nano X utilize all three lines of text
#ifdef TARGET_NANOS

// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_path_step,
             bn_paging,
             {
                 .title = "Path",
                 .text = g_bip32_path,
             });
// Step with title/text for address
UX_STEP_NOCB(ux_display_public_key_step,
             bn_paging,
             {
                 .title = "Public key",
                 .text = G_context.pk_info.wif,
             });

#else

// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_path_step,
             bnnn_paging,
             {
                 .title = "Path",
                 .text = g_bip32_path,
             });
// Step with title/text for address
UX_STEP_NOCB(ux_display_public_key_step,
             bnnn_paging,
             {
                 .title = "Public key",
                 .text = G_context.pk_info.wif,
             });
#endif

// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

// FLOW to display public key and BIP32 path:
// #1 screen: eye icon + "Confirm Public key"
// #2 screen: display BIP32 Path
// #3 screen: display public key
// #4 screen: approve button
// #5 screen: reject button
UX_FLOW(ux_display_pubkey_flow,
        &ux_display_confirm_public_key_step,
        &ux_display_path_step,
        &ux_display_public_key_step,
        &ux_display_approve_step,
        &ux_display_reject_step,
        FLOW_LOOP);

int ui_display_public_key() {
    if (G_context.req_type != CONFIRM_PUBLIC_KEY || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_bip32_path, 0, sizeof(g_bip32_path));
    if (!bip32_path_format(G_context.bip32_path, G_context.bip32_path_len, g_bip32_path, sizeof(g_bip32_path))) {
        return io_send_sw(SW_WRONG_BIP32_PATH);
    }

    g_validate_callback = &ui_action_validate_pubkey;

    ux_flow_init(0, ux_display_pubkey_flow, NULL);

    return 0;
}

/*****************************************************************************
 *   Ledger App Hive.
 *   (c) 2022 Bartłomiej @engrave Górnicki
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

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "globals.h"
#include "menu.h"
#include "settings.h"

static char sign_hash_policy_prompt[9];  // max size of longest settings name which is "Disabled"

UX_STEP_CB(ux_settings_hash_sign_step, bn_paging, switch_settings_hash_signing(), {.title = "Hash signing", .text = sign_hash_policy_prompt});
UX_STEP_VALID(ux_settings_back_step, pb, ui_menu_main(NULL), {&C_icon_back, "Back"});  // TODO make it back to ux_menu_settings_step

// FLOW for the settings submenu:
// #1 screen: blind signing
// #3 screen: back button to main menu
UX_FLOW(ux_settings_flow, &ux_settings_hash_sign_step, &ux_settings_back_step, FLOW_LOOP);

void ui_display_settings(const ux_flow_step_t* const start_step) {
    strcpy(sign_hash_policy_prompt, N_settings.sign_hash_policy == ENABLED ? "Enabled" : "Disabled");
    ux_flow_init(0, ux_settings_flow, start_step);
}

void switch_settings_hash_signing(void) {
    uint8_t value = N_settings.sign_hash_policy == ENABLED ? DISABLED : ENABLED;
    nvm_write((void*) &N_settings.sign_hash_policy, (void*) &value, sizeof(MEMBER_SIZE(settings_t, sign_hash_policy)));
    ui_display_settings(&ux_settings_hash_sign_step);
}

// clang-format off
#if defined(TARGET_NANOS)
UX_STEP_CB(
    ux_warning_hash_signing_step,
    bnnn_paging,
    ui_menu_main(NULL),
    {
      "Error",
      "Blind signing must be enabled in Settings",
    });
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
UX_STEP_CB(
    ux_warning_hash_signing_step,
    pnn,
    ui_menu_main(NULL),
    {
      &C_icon_crossmark,
      "Blind signing must be",
      "enabled in Settings",
    });
#endif
// clang-format on

UX_FLOW(ux_warning_hash_signing_disabled_flow, &ux_warning_hash_signing_step);

void ui_display_hash_signing_disabled_warning(void) {
    ux_flow_init(0, ux_warning_hash_signing_disabled_flow, NULL);
}
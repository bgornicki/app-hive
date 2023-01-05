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

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "globals.h"
#include "screens/settings.h"
#include "menu.h"

UX_STEP_NOCB(ux_menu_ready_step, pnn, {&C_hive_logo, "Waiting for", "commands"});
UX_STEP_NOCB(ux_menu_version_step, bn, {"Version", APPVERSION});
UX_STEP_CB(ux_menu_settings_step, pb, ui_display_settings(NULL), {&C_icon_coggle, "Settings"});
UX_STEP_CB(ux_menu_about_step, pb, ui_menu_about(), {&C_icon_certificate, "About"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

// FLOW for the main menu:
// #1 screen: ready
// #2 screen: version of the app
// #3 screen: settings
// #4 screen: about submenu
// #5 screen: quit
UX_FLOW(ux_menu_main_flow, &ux_menu_ready_step, &ux_menu_version_step, &ux_menu_settings_step, &ux_menu_about_step, &ux_menu_exit_step, FLOW_LOOP);

void ui_menu_main(const ux_flow_step_t* const start_step) {
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, start_step);
}

UX_STEP_NOCB(ux_menu_info_hive_step, bn, {"Hive", "https://hive.io"});
UX_STEP_NOCB(ux_menu_info_developed_by_step, bn, {"Developed by", "@engrave"});
UX_STEP_CB(ux_menu_back_step, pb, ui_menu_main(NULL), {&C_icon_back, "Back"});

// FLOW for the about submenu:
// #1 screen: chain info
// #2 screen: developer info
// #3 screen: back button to main menu
UX_FLOW(ux_menu_about_flow, &ux_menu_info_hive_step, &ux_menu_info_developed_by_step, &ux_menu_back_step, FLOW_LOOP);

void ui_menu_about() {
    ux_flow_init(0, ux_menu_about_flow, NULL);
}

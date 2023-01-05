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

#include <stdint.h>

#include "get_settings.h"
#include "globals.h"
#include "io.h"
#include "sw.h"
#include "types.h"
#include "common/buffer.h"

int handler_get_settings() {
    uint8_t settings[] = {N_settings.sign_hash_policy};

    buffer_t rdata = {.ptr = settings, .size = sizeof(settings), .offset = 0};

    return io_send_response(&rdata, SW_OK);
}

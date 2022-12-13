/*****************************************************************************
 *   Ledger App Hive
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

#include "varint.h"

static const uint8_t MSB = 0x80;
static const uint8_t REST = 0x7F;

int8_t varint_read(const uint8_t *in, size_t in_len, uint64_t *value) {
    uint32_t shift = 0;
    const uint8_t *ptr = in;
    uint64_t ll;
    *value = 0;
    while (*ptr & MSB) {
        ll = *ptr;
        *value += ((ll & REST) << shift);
        ptr++;
        shift += 7;
        if ((size_t)(ptr - in) >= in_len) {
            return -1;
        }
    }
    ll = *ptr;
    *value += ((ll & REST) << shift);

    return ptr - in + 1;
}
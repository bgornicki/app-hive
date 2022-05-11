/*****************************************************************************
 *   Ledger App Hive
 *   (c) 2021 Bartłomiej (@engrave) Górnicki
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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "common/format.h"

const char HIVE_ASSETS[3][7] = {"HIVE", "HBD", "VESTS"};

bool format_timestamp(uint32_t timestamp, char *out, size_t out_len) {
    if (out_len < DATE_TIME_STR_LEN || out == NULL) {
        return false;
    }

    uint8_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t hours, minutes, seconds, day, month, leap_days = 0;
    uint16_t temp_days = 0;
    uint32_t year, days_since_epoch, day_of_year = 0;

    seconds = timestamp % 60;
    timestamp /= 60;
    minutes = timestamp % 60;
    timestamp /= 60;
    hours = timestamp % 24;
    timestamp /= 24;

    days_since_epoch = timestamp;

    year = 1970 + (days_since_epoch / 365);  // Roughtly calculate year, may not be accurate because of leap-years

    for (uint32_t i = 1972; i < year; i += 4)  // Calculating number of leap days since epoch time
    {
        if (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0)) {
            leap_days++;
        }
    }

    year = 1970 + ((days_since_epoch - leap_days) / 365);  // Calculating accurate current year by (days_since_epoch - extra leap days)
    day_of_year = ((days_since_epoch - leap_days) % 365) + 1;

    if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) {
        month_days[1] = 29;  // February has 29 days for leap years
    } else {
        month_days[1] = 28;
    }

    for (month = 0; month <= 11; month++)  // calculate current month
    {
        if (day_of_year <= temp_days) break;
        temp_days = temp_days + month_days[month];
    }

    if (month >= 1) {
        temp_days = temp_days - month_days[month - 1];  // calculate current day
    }

    day = day_of_year - temp_days;

    snprintf(out, out_len, "%d-%02d-%02dT%02d:%02d:%02d", year, month, day, hours, minutes, seconds);

    return true;
}

bool format_i64(const int64_t i, char *out, uint8_t out_len) {
    char temp[] = "-9223372036854775808";

    char *ptr = temp;
    int64_t num = i;
    int sign = 1;

    if (i < 0) {
        sign = -1;
    }

    while (num != 0) {
        *ptr++ = '0' + (num % 10) * sign;
        num /= 10;
    }

    if (i < 0) {
        *ptr++ = '-';
    } else if (i == 0) {
        *ptr++ = '0';
    }

    int distance = (ptr - temp) + 1;

    if ((int) out_len < distance) {
        return false;
    }

    size_t index = 0;

    while (--ptr >= temp) {
        out[index++] = *ptr;
    }

    out[index] = '\0';

    return true;
}

bool format_u64(const uint64_t i, char *out, uint8_t out_len) {
    if (out_len < MAX_U64_LEN) {
        return false;
    }

    uint64_t value = i;
    char *p = out;

    // determine string length and insert characters in reverse order
    uint64_t shifter = value;
    do {
        ++p;
        shifter = shifter / 10;
    } while (shifter);

    *p = '\0';

    do {
        *--p = 0x30 + (value % 10);  // change to digit character
        value = value / 10;
    } while (value);
    return true;
}

static bool insert_string(char *out, uint8_t out_len, const char *source, size_t position) {
    if (strlen(source) + position > out_len - 1) {  // need space for null character
        return false;
    }

    char tmp[out_len];

    memset(tmp, 0, out_len);

    strncpy(tmp, out, position);
    uint8_t length = strlen(tmp);
    strncpy(tmp + length, source, out_len - length);
    length += strlen(source);
    strncpy(tmp + length, out + position, out_len - length);
    strncpy(out, tmp, out_len);

    return true;
}

bool format_asset(asset_t *asset, char *out, size_t out_len) {
    if (out_len < MAX_HIVE_ASSET_LEN) {
        return false;
    }

    if (!format_i64(asset->amount, out, out_len)) {
        return false;
    }

    // append zeros to match required asset precision
    while (strlen(out) <= asset->precision) {
        if (!insert_string(out, out_len, "0", 0)) {
            return false;
        }
    }

    // Insert dot for precision and space before asset name
    if (!insert_string(out, out_len, ".", strlen(out) - asset->precision) || !insert_string(out, out_len, " ", strlen(out))) {
        return false;
    }

    // Because we're fork of Steem blockchain, our backend nodes require Steem assets during the serialization, but we want to display Hive assets for ledger
    // users. Here, we check the second character to determine asset.
    const char *symbol_ptr = NULL;

    if (memcmp(asset->symbol, "STEEM", strlen("STEEM")) == 0) {
        symbol_ptr = HIVE_ASSETS[0];
    } else if (memcmp(asset->symbol, "SBD", strlen("SBD")) == 0) {
        symbol_ptr = HIVE_ASSETS[1];
    }

    if (!insert_string(out, out_len, symbol_ptr ? symbol_ptr : asset->symbol, strlen(out))) {
        return false;
    }

    return true;
}

bool format_hash(const uint8_t *hash, size_t hash_len, char *out, size_t out_len) {
    if (out_len < (hash_len * 2) + 1) {  // hex representation with null character at the end
        return false;
    }

    memset(out, 0, out_len);

    for (size_t i = 0; i < hash_len; i++) {
        snprintf(out + i * 2, out_len - i * 2, "%02X", hash[i]);
    }

    return true;
}

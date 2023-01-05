#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "common/wif.h"
#include "common/base58.h"

#include "macros.h"

#ifdef TARGET_NANOS
#include "errors.h"
#endif

static bool compress_public_key(uint8_t raw_public_key[static PUBKEY_UNCOMPRESSED_LEN], uint8_t *out, size_t out_len) {
    if (out_len < PUBKEY_COMPRESSED_LEN) {
        return false;
    }
    // Because the elliptic curve is symmetrical along its x axis, we only need the x coordinate and
    // the indicator of y coordinate position
    out[0] = (raw_public_key[63] & 0x1) ? 0x03 : 0x02;
    memmove(out + 1, raw_public_key, PUBKEY_COMPRESSED_LEN - 1);
    return true;
}

bool wif_from_public_key(uint8_t raw_public_key[static PUBKEY_UNCOMPRESSED_LEN], size_t key_len, char *out, size_t out_len) {
    if (raw_public_key == NULL || key_len < PUBKEY_UNCOMPRESSED_LEN || out_len < PUBKEY_WIF_STR_LEN) {
        return false;
    }

    uint8_t compressed[PUBKEY_COMPRESSED_LEN];

    if (!compress_public_key(raw_public_key, compressed, ARRAYLEN(compressed))) {
        return false;
    }

    return wif_from_compressed_public_key(compressed, ARRAYLEN(compressed), out, out_len);
}

bool wif_from_compressed_public_key(uint8_t compressed_key[static PUBKEY_COMPRESSED_LEN], size_t key_len, char *out, size_t out_len) {
    if (key_len != PUBKEY_COMPRESSED_LEN || out_len < PUBKEY_WIF_STR_LEN) {
        return false;
    }

    uint8_t temp[PUBKEY_COMPRESSED_LEN + 4] = {0};
    memmove(temp, compressed_key, PUBKEY_COMPRESSED_LEN);

    uint8_t hash[CX_RIPEMD160_SIZE] = {0};
    cx_ripemd160_t riprip;
    cx_ripemd160_init(&riprip);
    cx_hash(&riprip.header, CX_LAST, temp, PUBKEY_COMPRESSED_LEN, hash, sizeof(hash));
    memmove(temp + PUBKEY_COMPRESSED_LEN, hash, 4);

    memset(out, 0, out_len);
    strncpy(out, "STM", out_len);

    if (base58_encode(temp, sizeof(temp), out + 3, out_len - 3) == -1) {
        return false;
    }

    return true;
}

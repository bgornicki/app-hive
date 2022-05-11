/*****************************************************************************
 *   Ledger App Hive.
 *   (c) 2020 Ledger SAS.
 *   Modification based on (c) 2020 Andrew Chaney
 *   Modifications (c) 2021, 2022 Bartłomiej (@engrave) Górnicki
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

#include <stdint.h>   // uint*_t
#include <string.h>   // memset, explicit_bzero
#include <stdbool.h>  // bool

#include "crypto.h"

#include "types.h"
#include "globals.h"

#include "common/rng_rfc6979.h"
#include "common/signature.h"
#include "common/macros.h"

uint8_t const SECP256K1_N[32] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
                                 0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48, 0xa0, 0x3b, 0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41};

int crypto_derive_private_key(cx_ecfp_private_key_t *private_key,
                              uint8_t chain_code[static CHAINCODE_LEN],
                              const uint32_t *bip32_path,
                              uint8_t bip32_path_len) {
    uint8_t raw_private_key[32] = {0};

    BEGIN_TRY {
        TRY {
            // derive the seed with bip32_path
            os_perso_derive_node_bip32(CX_CURVE_256K1, bip32_path, bip32_path_len, raw_private_key, chain_code);
            // new private_key from raw
            cx_ecfp_init_private_key(CX_CURVE_256K1, raw_private_key, sizeof(raw_private_key), private_key);
        }
        CATCH_OTHER(e) {
            THROW(e);
        }
        FINALLY {
            explicit_bzero(&raw_private_key, sizeof(raw_private_key));
        }
    }
    END_TRY;

    return 0;
}

int crypto_init_public_key(cx_ecfp_private_key_t *private_key, cx_ecfp_public_key_t *public_key, uint8_t raw_public_key[static PUBKEY_UNCOMPRESSED_LEN]) {
    // generate corresponding public key
    cx_ecfp_generate_pair(CX_CURVE_256K1, public_key, private_key, 1);

    memmove(raw_public_key, public_key->W + 1, 64);

    return 0;
}

bool crypto_sign_digest(const uint8_t digest[static DIGEST_LEN], uint8_t signature[static SIGNATURE_LEN]) {
    cx_ecfp_private_key_t private_key = {0};
    uint8_t chain_code[CHAINCODE_LEN] = {0};
    uint8_t der_signature[MAX_DER_SIG_LEN] = {0};
    uint32_t info = 0;
    uint8_t V[DIGEST_LEN];
    uint8_t K[DIGEST_LEN];
    int32_t counter = 0;
    bool is_valid = true;

    PRINTF("Digest: %.*H\n", DIGEST_LEN, digest);

    // derive private key according to BIP32 path
    crypto_derive_private_key(&private_key, chain_code, G_context.bip32_path, G_context.bip32_path_len);

    BEGIN_TRY {
        TRY {
            /* Hive backend only accepts canonical signatures but there is no way of knowing if the signature that is going to be produced will be canonical.
             * That's why we need to derive deterministic k parameter for ECDSA signing and while generating this parameter we will add our loop counter to the
             * digest before hashing. This results in a new deterministic k each round which will result in either a canonical or non-canonical signature. */
            while (true) {
                if (counter == 0) {
                    rng_rfc6979(der_signature, (uint8_t *) digest, private_key.d, private_key.d_len, SECP256K1_N, ARRAYLEN(SECP256K1_N), V, K);
                } else {
                    rng_rfc6979(der_signature, (uint8_t *) digest, NULL, 0, SECP256K1_N, ARRAYLEN(SECP256K1_N), V, K);
                }

                cx_ecdsa_sign(&private_key, CX_NO_CANONICAL | CX_RND_PROVIDED | CX_LAST, CX_SHA256, digest, DIGEST_LEN, der_signature, MAX_DER_SIG_LEN, &info);

                if ((info & CX_ECCINFO_PARITY_ODD) != 0) {
                    der_signature[0] |= 0x01;
                }

                if (!signature_from_der(der_signature, signature, MEMBER_SIZE(transaction_ctx_t, signature))) {
                    is_valid = false;
                    break;
                }

                if (signature_check_canonical(signature + 1)) {
                    break;
                } else {
                    counter++;
                }
            }
        }
        CATCH_OTHER(e) {
            THROW(e);
        }
        FINALLY {
            explicit_bzero(&private_key, sizeof(private_key));
        }
    }
    END_TRY;

    return is_valid;
}
#pragma once

#include "os.h"

#include "common/macros.h"

/**
 * Length of WIF.
 */
#define WIF_LEN ((MEMBER_SIZE(pubkey_ctx_t, wif)) - 1)
/**
 * Length of public key.
 */
#define PUBKEY_LEN (MEMBER_SIZE(pubkey_ctx_t, raw_public_key))

/**
 * Helper to send APDU response with public key, it's wif representation and chain code.
 *
 *
 * 1 + PUBKEY_LEN + 1 + WIF_LEN + CHAINCODE_LEN]
 * response = PUBKEY_LEN (1) ||
 *            G_context.pk_info.public_key (PUBKEY_LEN) ||
 *            WIF_LEN(1) ||
 *            G_context.pk_info.wif ||
 *            G_context.pk_info.chain_code (CHAINCODE_LEN)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_pubkey(void);

/**
 * Helper to send APDU response with compact signature
 *
 * response = G_context.tx_info.signature(65)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_sig(const uint8_t *signature, size_t sig_len);

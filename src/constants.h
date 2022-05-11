#pragma once

#include "common/macros.h"

/**
 * Instruction class of the Hive application.
 */
#define CLA 0xD4

/**
 * Length of APPNAME variable in the Makefile.
 */
#define APPNAME_LEN (sizeof(APPNAME) - 1)

/**
 * Maximum length of MAJOR_VERSION || MINOR_VERSION || PATCH_VERSION.
 */
#define APPVERSION_LEN 3

/**
 * Maximum length of application name.
 */
#define MAX_APPNAME_LEN 64

/**
 * Max APDU command length
 */
#define MAX_APDU_LEN 255

/**
 * Max incoming data chunk size
 */
#define MAX_DATA_CHUNK_LEN 200

/**
 * Maximum transaction length (bytes) - four chunks
 */
#define MAX_TRANSACTION_LEN (3 * MAX_DATA_CHUNK_LEN)

/**
 * Maximum DER encoded signature length (bytes).
 */
#define MAX_DER_SIG_LEN 74

/**
 * Maximum compact signature length (bytes).
 */
#define SIGNATURE_LEN 1 + 32 + 32

/**
 * Hash digest length
 */
#define DIGEST_LEN 32

/**
 * Chaincode length
 */
#define CHAINCODE_LEN 32

// [x-coordinate (32)][y-coordinate (32)]
#define PUBKEY_UNCOMPRESSED_LEN 64

// [prefix (1)][x-coordinate (32)]
#define PUBKEY_COMPRESSED_LEN 33

// [wif (53)][\0]
#define PUBKEY_WIF_STR_LEN 54
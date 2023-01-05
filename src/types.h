#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "constants.h"
#include "common/buffer.h"
#include "common/wif.h"
#include "common/bip32.h"

#include "os.h"
#include "cx.h"

/**
 * Structure to keep operation field data to display on screen during transaction confirmation
 */
typedef struct {
    char title[60];
    char value[255];
} field_t;

/**
 * Callback to reuse action with approve/reject in step FLOW.
 */
typedef void (*action_validate_cb)(bool);

/**
 * Enumeration for the status of IO.
 */
typedef enum {
    READY,     /// ready for new event
    RECEIVED,  /// data received
    WAITING    /// waiting
} io_state_e;

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    GET_PUBLIC_KEY = 0x02,    /// public key of corresponding BIP32 path
    SIGN_TRANSACTION = 0x04,  /// sign transaction with BIP32 path
    GET_VERSION = 0x06,       /// version of the application
    GET_APP_NAME = 0x08,      /// name of the application
    SIGN_HASH = 0x10,         /// sign hash with BIP32 path
    GET_SETTINGS = 0x12       /// settings of the application
} command_e;

/**
 * Structure with fields of APDU command.
 */
typedef struct {
    uint8_t cla;    /// Instruction class
    command_e ins;  /// Instruction code
    uint8_t p1;     /// Instruction parameter 1
    uint8_t p2;     /// Instruction parameter 2
    uint8_t lc;     /// Lenght of command data
    uint8_t *data;  /// Command data
} command_t;

/**
 * Enumeration with parsing state.
 */
typedef enum {
    STATE_NONE,          /// No state
    STATE_TX_RECEIVING,  /// Multi-APDU transaction is streamed to the device
    STATE_PARSED,        /// Transaction data parsed
    STATE_APPROVED       /// Transaction data approved
} state_e;

/**
 * Enumeration with user request type.
 */
typedef enum {
    CONFIRM_PUBLIC_KEY,   /// confirm public key formatted in a Hive way
    CONFIRM_TRANSACTION,  /// confirm transaction information
    CONFIRM_HASH          /// confirm hash
} request_type_e;

/**
 * DER field decoder which should display decoded content on screen
 * Params: input, input_length, field_name, field_value
 */
typedef bool decoder_t(buffer_t *, field_t *, bool);

/**
 * Operation parser, will decode and hash properties from serialized operation
 */
typedef struct parser_t {
    decoder_t *decoders[9];
    char names[9][30];
    uint8_t size;
} parser_t;

/**
 * Structure for public key context information.
 */
typedef struct {
    uint8_t raw_public_key[PUBKEY_UNCOMPRESSED_LEN];  /// x-coordinate (32), y-coodinate (32)
    uint8_t chain_code[CHAINCODE_LEN];                /// for public key derivation
    char wif[PUBKEY_WIF_STR_LEN];                     /// public key in Hive format
} pubkey_ctx_t;

/**
 * Structure for transaction information context.
 */
typedef struct {
    uint8_t raw_tx[MAX_TRANSACTION_LEN];  /// raw transaction serialized
    size_t raw_tx_len;                    /// length of raw transaction

    const parser_t *parser;
    buffer_t operation;

    uint8_t digest[DIGEST_LEN];        /// message digest
    uint8_t signature[SIGNATURE_LEN];  /// compact transaction signature supported by Hive backend
    cx_sha256_t sha;

} transaction_ctx_t;

/**
 * Structure for hash signing context (blind signing)
 */
typedef struct {
    uint8_t hash[DIGEST_LEN];          // input hash
    uint8_t signature[SIGNATURE_LEN];  /// compact hash signature supported by Hive backend
} hash_ctx_t;

/**
 * Structure for global context.
 */
typedef struct {
    state_e state;  /// state of the context
    union {
        pubkey_ctx_t pk_info;       /// public key context
        transaction_ctx_t tx_info;  /// transaction context
        hash_ctx_t hash_info;       /// hash signing context
    };
    request_type_e req_type;              /// user request
    uint32_t bip32_path[MAX_BIP32_PATH];  /// BIP32 path
    uint8_t bip32_path_len;               /// lenght of BIP32 path

} global_ctx_t;

typedef enum {
    PARSING_OK = 1,
    BIP32_PATH_PARSING_ERROR = -1,
    FIELD_PARSING_ERROR = -2,
    OPERATION_COUNT_PARSING_ERROR = -3,
    WRONG_LENGTH_ERROR = -4
} parser_status_e;

typedef enum { DISABLED = 0x00, ENABLED = 0x01 } sign_hash_policy_t;

typedef struct {
    uint8_t initialized;
    sign_hash_policy_t sign_hash_policy;
} settings_t;
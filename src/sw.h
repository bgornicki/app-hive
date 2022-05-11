#pragma once

/**
 * Status word for success.
 */
#define SW_OK 0x9000
/**
 * Status word for denied by user.
 */
#define SW_DENY 0x6985
/**
 * Status word for incorrect P1 or P2.
 */
#define SW_WRONG_P1P2 0x6A86
/**
 * Status word for either wrong Lc or lenght of APDU command less than 5.
 */
#define SW_WRONG_DATA_LENGTH 0x6A87
/**
 * Status word for unknown command with this INS.
 */
#define SW_INS_NOT_SUPPORTED 0x6D00
/**
 * Status word for instruction class is different than CLA.
 */
#define SW_CLA_NOT_SUPPORTED 0x6E00
/**
 * Status word for wrong reponse length (buffer too small or too big).
 */
#define SW_WRONG_RESPONSE_LENGTH 0xB000
/**
 * Status word for fail to display BIP32 path.
 */
#define SW_WRONG_BIP32_PATH 0xB001
/**
 * Status word for wrong transaction length.
 */
#define SW_WRONG_TX_LENGTH 0xB002
/**
 * Status word for fail of transaction parsing.
 */
#define SW_TX_PARSING_FAIL 0xB003
/**
 * Status word for bad state.
 */
#define SW_BAD_STATE 0xB004
/**
 * Status word for signature fail.
 */
#define SW_SIGNATURE_FAIL 0xB005
/**
 * Status word for trying to sign hash with policy disabled in settings.
 */
#define SW_HASH_SIGNING_DISABLED 0xB006
/**
 * Status word invalid length of input dat.
 */
#define SW_WRONG_HASH_LENGTH 0xB007
/**
 * Status word for hash parsing fail.
 */
#define SW_HASH_PARSING_FAIL 0xB008

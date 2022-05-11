# Hive application commands

## Overview

| Command name       | INS  | Description                                                               |
| ------------------ | ---- | ------------------------------------------------------------------------- |
| `GET_PUBLIC_KEY`   | 0x02 | Get public key given BIP32 path (SLIP-0048)                               |
| `SIGN_TRANSACTION` | 0x04 | Sign transaction given BIP32 path (SLIP-0048) and DER encoded transaction |
| `GET_VERSION`      | 0x06 | Get application version as `MAJOR`, `MINOR`, `PATCH` buffer               |
| `GET_APP_NAME`     | 0x08 | Get ASCII encoded application name                                        |
| `SIGN_HASH`        | 0x10 | Sign transaction digest (blind sign)                                      |
| `GET_SETTINGS`     | 0x12 | Get application settings                                                  |

## GET_PUBLIC_KEY

This command returns public key (both raw and WIF format) and chain code. Public key can be derived from BIP 32 path, which have to comply with SLIP-0048 standard (max derivation path is set to 5).

Public key can be optionaly reviewed and accepted by user before being returned (P1 = 0x01).

### Command

| CLA  | INS  | P1                                                                         | P2   | Lc     | CData                                                                                        |
| ---- | ---- | -------------------------------------------------------------------------- | ---- | ------ | -------------------------------------------------------------------------------------------- |
| 0xD4 | 0x02 | 0x00 (no display) <br> 0x01 (ask user to confirm before returning the key) | 0x00 | 1 + 4n | `len(bip32_path) (1)` \|\|<br> `bip32_path{1} (4)` \|\|<br>`...` \|\|<br>`bip32_path{n} (4)` |

### Response

| Response length (bytes) | SW     | RData                                                                                                                     |
| ----------------------- | ------ | ------------------------------------------------------------------------------------------------------------------------- |
| var                     | 0x9000 | `len(public_key) (1)` \|\|<br> `public_key (var)` \|\|<br> `len(wif) (1)` \|\|<br> `wif (var)` \|\|<br> `chain code (32)` |

## SIGN_TRANSACTION

This command signs provided transaction with key derived from BIP 32 path (which must comply with SLIP-0048 standard). Transaction mu be accepted by the user.

Input data is BIP 32 path followed by ASN1 DER encoded transaction (each transaction field is encoded as a StringOctet type), sent to the device in 255 bytes maximum data chunks.

If there is a need to send more than one APDU (i.e transaction is big enought to exceed 250 bytes), BIP 32 path should be only sent in the first chunk.

Transaction fields have to be sent in following order:

- chain id
- ref_block_num
- ref_block_prefix
- expiration
- number of operations
- operation
- number of extensions

Currently, only single operation transactions are supported. App will refuse transaction which contains `number of operations` other than 1. Number of extensions have to be zero, otherwise transaction will be rejected.

### Command

| CLA  | INS  | P1                                              | P2                                        | Lc           | CData                                                                                                                                                                                                                                |
| ---- | ---- | ----------------------------------------------- | ----------------------------------------- | ------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 0xD4 | 0x04 | 0x00 (first chunk) <br> 0x80 (subsequent chunk) | 0x00 (last chunk) <br> 0x80 (expect more) | 1 + 4n + var | **First chunk**:<br> `len(bip32_path) (1)` \|\|<br> `bip32_path{1} (4)` \|\|<br>`...` \|\|<br>`bip32_path{n} (4)` \|\|<br>`DER encoded transaction (var)`<br><br>**Subsequent chunk (optional)**:<br>`DER encoded transaction (var)` |

### Response

| Response length (bytes) | SW     | RData            |
| ----------------------- | ------ | ---------------- |
| var                     | 0x9000 | `signature (33)` |

## GET_VERSION

### Command

| CLA  | INS  | P1   | P2   | Lc   | CData |
| ---- | ---- | ---- | ---- | ---- | ----- |
| 0xD4 | 0x06 | 0x00 | 0x00 | 0x00 | -     |

### Response

| Response length (bytes) | SW     | RData                                         |
| ----------------------- | ------ | --------------------------------------------- |
| 3                       | 0x9000 | `MAJOR (1)` \|\| `MINOR (1)` \|\| `PATCH (1)` |

## GET_APP_NAME

### Command

| CLA  | INS  | P1   | P2   | Lc   | CData |
| ---- | ---- | ---- | ---- | ---- | ----- |
| 0xD4 | 0x08 | 0x00 | 0x00 | 0x00 | -     |

### Response

| Response length (bytes) | SW     | RData           |
| ----------------------- | ------ | --------------- |
| var                     | 0x9000 | `APPNAME (var)` |

## SIGN_HASH

This command signs provided tx digest with key derived from BIP 32 path (which must comply with SLIP-0048 standard). Hash string will be desplayed to the user and must be accepted.

Input data is BIP 32 path followed by transaction digest (array of bytes)

### Command

| CLA  | INS  | P1   | P2   | Lc          | CData                                                                                                              |
| ---- | ---- | ---- | ---- | ----------- | ------------------------------------------------------------------------------------------------------------------ |
| 0xD4 | 0x10 | 0x00 | 0x00 | 1 + 4n + 32 | `len(bip32_path) (1)` \|\|<br> `bip32_path{1} (4)` \|\|<br>`...` \|\|<br>`bip32_path{n} (4)` \|\|<br>`digest (32)` |

### Response

| Response length (bytes) | SW     | RData            |
| ----------------------- | ------ | ---------------- |
| var                     | 0x9000 | `signature (33)` |

## GET_SETTINGS

### Command

| CLA  | INS  | P1   | P2   | Lc   | CData |
| ---- | ---- | ---- | ---- | ---- | ----- |
| 0xD4 | 0x12 | 0x00 | 0x00 | 0x00 | -     |

### Response

| Response length (bytes) | SW     | RData                  |
| ----------------------- | ------ | ---------------------- |
| var                     | 0x9000 | `hash_sign_policy (1)` |

## Status Words

| SW     | SW name                    | Description                                 |
| ------ | -------------------------- | ------------------------------------------- |
| 0x6985 | `SW_DENY`                  | Rejected by user                            |
| 0x6A86 | `SW_WRONG_P1P2`            | Either `P1` or `P2` is incorrect            |
| 0x6A87 | `SW_WRONG_DATA_LENGTH`     | `Lc` or minimum APDU lenght is incorrect    |
| 0x6D00 | `SW_INS_NOT_SUPPORTED`     | No command exists with `INS`                |
| 0x6E00 | `SW_CLA_NOT_SUPPORTED`     | Bad `CLA` used for this application         |
| 0xB000 | `SW_WRONG_RESPONSE_LENGTH` | Wrong response lenght (buffer size problem) |
| 0xB001 | `SW_WRONG_BIP32_PATH`      | BIP32 path conversion to string failed      |
| 0xB002 | `SW_WRONG_TX_LENGTH`       | Wrong raw transaction lenght                |
| 0xB003 | `SW_TX_PARSING_FAIL`       | Failed to parse raw transaction             |
| 0xB004 | `SW_BAD_STATE`             | Security issue with bad state               |
| 0xB005 | `SW_SIGNATURE_FAIL`        | Signature of raw transaction failed         |
| 0xB006 | `SW_HASH_SIGNING_DISABLED` | Hash signing is disabled in settings        |
| 0xB007 | `SW_WRONG_HASH_LENGTH`     | Invalid length of input data                |
| 0xB008 | `SW_HASH_PARSING_FAIL`     | Failed to parse transaction hash            |
| 0x9000 | `SW_OK`                    | Success                                     |

#pragma once

#include <string.h>

#include "types.h"
#include "common/buffer.h"

/**
 * Parse DER encoded transaction
 *
 * @param[in] buf
 *   Pointer to buffer with DER encoded transaction.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e transaction_parse(buffer_t *buf);

parser_status_e hash_parse(buffer_t *buf);
#pragma once
#include "transaction/decoders.h"

/**
 * Get the operation parser object
 *
 * @param operation_nr
 *  Hive operation nr
 * @return pointer to the appropriate parser object
 */
const parser_t *get_operation_parser(uint8_t operation_nr);

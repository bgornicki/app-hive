#pragma once

#include "os.h"

/**
 * Handler for GET_SETTINGS command. Send APDU response with array of settings values.
 *
 * @see settings_t
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_settings(void);

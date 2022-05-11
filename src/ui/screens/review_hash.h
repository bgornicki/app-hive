#pragma once

#include <stdbool.h>

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "constants.h"
#include "globals.h"
#include "io.h"
#include "sw.h"
#include "common/bip32.h"
#include "common/format.h"
#include "common/wif.h"
#include "common/macros.h"
#include "ui/action/validate.h"

/**
 * Display hash string on the device and ask confirmation before signing
 *
 * @return 0 if success, negative integer otherwise.
 *
 */
int ui_display_hash(void);

/**
 * Initialize "Signing hash" display when digest got accepted
 *
 */
void ui_display_signing_hash_message(void);
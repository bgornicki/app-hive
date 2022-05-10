#pragma once

#include <stdbool.h>  // bool
#include "ui/screens/review_transaction.h"
#include "ui/screens/review_hash.h"

/**
 * Action for public key validation and export.
 *
 * @param[in] choice
 *   User choice (either approved or rejected).
 *
 */
void ui_action_validate_pubkey(bool choice);

/**
 * Action for transaction information validation.
 *
 * @param[in] choice
 *   User choice (either approved or rejected).
 *
 */
void ui_action_validate_transaction(bool choice);

/**
 * Action for hash information validation.
 *
 * @param[in] choice
 *   User choice (either approved or rejected).
 *
 */
void ui_action_validate_hash(bool choice);

/*****************************************************************************
 *   Ledger App Hive.
 *   (c) Bartłomiej (@engrave) Górnicki
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include "parsers.h"
#include "constants.h"
#include "decoders.h"
#include "globals.h"
#include "common/buffer.h"
#include "common/asn1.h"
#include "common/bip32.h"

// clang-format off

// 0 vote
const parser_t vote_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_string, &decoder_weight}, 
    .names = {"Operation", "Voter", "Author", "Permlink", "Weight"},
    .size = 5
};

// 1 comment
parser_t const comment_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_string, &decoder_string, &decoder_string, &decoder_string, &decoder_string},
    .names = {"Operation", "Parent author", "Parent permlink", "Author", "Permlink", "Title", "Body", "JSON metadata"},
    .size = 8
};

// 2 transfer && 32 transfer_to_savings
const parser_t transfer_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_asset, &decoder_string},
    .names = {"Operation", "From", "To", "Amount", "Memo"},
    .size = 5
};

// 3 transfer_to_vesting
const parser_t transfer_to_vesting_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_asset},
    .names = {"Operation", "From", "To", "Amount"},
    .size = 4
};

// 4 withdraw_vesting
const parser_t withdraw_vesting_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_asset},
    .names = {"Operation", "Account", "Vesting shares"},
    .size = 3
};

// 5 limit_order_create
const parser_t limit_order_create_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_uint32, &decoder_asset, &decoder_asset, &decoder_boolean, &decoder_date_time},
    .names = {"Operation", "Owner", "Order ID", "Amount to sell", "Min to receive", "Fill or kill", "Expiration"}, 
    .size = 7
};

// 6 limit_order_cancel
const parser_t limit_order_cancel_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_uint32},
    .names = {"Operation", "Owner", "Order ID"}, 
    .size = 3
};

// 7 feed_publish
const parser_t feed_publish_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_asset, &decoder_asset},
    .names = {"Operation", "Publisher", "Base", "Quote"}, 
    .size = 4
};

// 8 convert
const parser_t convert_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_uint32, &decoder_asset},
    .names = {"Operation", "Owner", "Request ID", "Amount"}, 
    .size = 4
};

// 9 account_create
const parser_t account_create_parser = {
    .decoders = {&decoder_operation_name, &decoder_asset, &decoder_string, &decoder_string, &decoder_authority_type, &decoder_authority_type, &decoder_authority_type, &decoder_public_key, &decoder_string},
    .names = {"Operation", "Fee", "Creator", "New acc. name", "Owner", "Active", "Posting", "Memo key", "JSON metadata"},
    .size = 9
};

// 10 account_update
const parser_t account_update_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_optional_authority_type, &decoder_optional_authority_type, &decoder_optional_authority_type, &decoder_public_key, &decoder_string},
    .names = {"Operation", "Account", "Owner", "Active", "Posting", "Memo key", "JSON metadata"},
    .size = 7
};

// 11 witness_update
const parser_t witness_update_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_public_key, &decoder_asset, &decoder_uint32, &decoder_weight, &decoder_asset},
    .names = {"Operation", "Owner", "Url", "Signing key", "Acc. creation fee", "Max block size", "HBD interest rate", "Fee"},
    .size = 8
};

// 12 account_witness_vote
parser_t const account_witness_vote_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_boolean},
    .names = {"Operation", "Account", "Witness", "Approve"},
    .size = 4
};

// 13 set_witness_proxy
const parser_t set_witness_proxy_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string},
    .names = {"Operation", "Account", "Proxy"},
    .size = 3
};

// 17 delete_comment
const parser_t delete_comment_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string},
    .names = {"Operation", "Author", "Permlink"},
    .size = 3
};

// 18 custom_json
const parser_t custom_json_parser = {
    .decoders = {&decoder_operation_name, &decoder_array_of_strings, &decoder_array_of_strings, &decoder_string, &decoder_string},
    .names = {"Operation", "Req. auths", "Req. posting auths", "ID", "JSON"},
    .size = 5
};

// 19 comment_options
const parser_t comment_options_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_asset, &decoder_weight, &decoder_boolean, &decoder_boolean, &decoder_beneficiaries_extensions},
    .names = {"Operation", "Author", "Permlink", "Max payout", "Percent HBD", "Allow votes", "Allow curation", "Extensions"},
    .size = 8
};

// 20 set_withdraw_vesting_route
const parser_t set_withdraw_vesting_route_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_weight, &decoder_boolean},
    .names = {"Operation", "From account", "To account", "Percent", "Autovest"},
    .size = 5
};

// 22 claim_account
const parser_t claim_account_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_asset, &decoder_empty_extensions},
    .names = {"Operation", "Creator", "Fee", "Extensions"},
    .size = 4
};

// 23 create_claimed_account
const parser_t create_claimed_account_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_authority_type, &decoder_authority_type, &decoder_authority_type, &decoder_public_key, &decoder_string, &decoder_empty_extensions},
    .names = {"Operation", "Creator", "New acc. name", "Owner", "Active", "Posting", "Memo key", "JSON metadata", "Extensions"},
    .size = 9
};


// 24 request_account_recovery
const parser_t request_account_recovery_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_authority_type, &decoder_empty_extensions},
    .names = {"Operation", "Recovery account", "Acc. to recover", "New owner auth", "Extensions"},
    .size = 5
};


// 25 recover_account
const parser_t recover_account_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_authority_type, &decoder_authority_type, &decoder_empty_extensions},
    .names = {"Operation", "Acc. to recover", "New owner auth", "Rec. owner auth", "Extensions"},
    .size = 5
};

// 26 change_recovery_account
const parser_t change_recovery_account_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_empty_extensions},
    .names = {"Operation", "Acc. to recover", "New recovery acc", "Extensions"},
    .size = 4
};

// 33 transfer_from_savings
const parser_t transfer_from_savings_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_uint32, &decoder_string, &decoder_asset, &decoder_string},
    .names = {"Operation", "From", "Request ID", "To", "Amount", "Memo"},
    .size = 6
};

// 34 cancel_transfer_from_savings
const parser_t cancel_transfer_from_savings_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_uint32},
    .names = {"Operation", "From", "Request ID"},
    .size = 3
};

// 36 decline_voting_rights
const parser_t decline_voting_rights_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_boolean},
    .names = {"Operation", "Account", "Decline"},
    .size = 3
};

// 37 reset_account
const parser_t reset_account_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_authority_type},
    .names = {"Operation", "Reset account", "Acc. to reset", "New owner auths"},
    .size = 4
};

// 38 set_reset_account
const parser_t set_reset_account_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_string},
    .names = {"Operation", "Account", "Cur. reset acc.", "New reset acc."},
    .size = 4
};

// 39 claim_reward_balance
const parser_t claim_reward_balance_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_asset, &decoder_asset, &decoder_asset},
    .names = {"Operation", "Account", "Reward HIVE", "Reward HBD", "Reward VESTS"},
    .size = 5
};

// 40 delegate_vesting_shares
const parser_t delegate_vesting_shares_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_asset},
    .names = {"Operation", "Delegator", "Delegatee", "Vesting shares"},
    .size = 4
};

// 44 create_proposal
const parser_t create_proposal_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_date_time, &decoder_date_time, &decoder_asset, &decoder_string, &decoder_string, &decoder_empty_extensions},
    .names = {"Operation", "Creator", "Receiver", "Start date", "End date", "Daily pay", "Subject", "Permlink", "Extensions"},
    .size = 9
};

// 45 update_proposal_votes
const parser_t update_proposal_votes_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_array_of_u64, &decoder_boolean, &decoder_empty_extensions},
    .names = {"Operation", "Voter", "Proposals", "Approve", "Extensions"},
    .size = 5
};

// 46 remove_proposal
const parser_t remove_proposal_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_array_of_u64, &decoder_empty_extensions},
    .names = {"Operation", "Proposal owner", "Proposals", "Extensions"},
    .size = 4
};

// 47 update_proposal
const parser_t update_proposal_parser = {
    .decoders = {&decoder_operation_name, &decoder_uint32, &decoder_array_of_u64, &decoder_empty_extensions},
    .names = {"Operation", "Proposal owner", "Proposals", "Extensions"},
    .size = 4
};

// 48 collateralized_convert
const parser_t collateralized_convert_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_uint32, &decoder_asset},
    .names = {"Operation", "Owner", "Request ID", "Amount"},
    .size = 4
};

// 49 recurrent_transfer
const parser_t recurrent_transfer_parser = {
    .decoders = {&decoder_operation_name, &decoder_string, &decoder_string, &decoder_asset, &decoder_string, &decoder_uint16, &decoder_uint16, &decoder_empty_extensions},
    .names = {"Operation", "From", "To", "Amount", "Memo", "Recurrence", "Executions", "Extensions"},
    .size = 8
};

// clang-format on

const parser_t *get_operation_parser(uint8_t operation_nr) {
    switch (operation_nr) {
        case 0:
            return &vote_parser;
        case 1:
            return &comment_parser;
        case 2:
            return &transfer_parser;
        case 3:
            return &transfer_to_vesting_parser;
        case 4:
            return &withdraw_vesting_parser;
        case 5:
            return &limit_order_create_parser;
        case 6:
            return &limit_order_cancel_parser;
        case 7:
            return &feed_publish_parser;
        case 8:
            return &convert_parser;
        case 9:
            return &account_create_parser;
        case 10:
            return &account_update_parser;
        case 11:
            return &witness_update_parser;
        case 12:
            return &account_witness_vote_parser;
        case 13:
            return &set_witness_proxy_parser;
        case 17:
            return &delete_comment_parser;
        case 18:
            return &custom_json_parser;
        case 19:
            return &comment_options_parser;
        case 20:
            return &set_withdraw_vesting_route_parser;
        case 22:
            return &claim_account_parser;
        case 23:
            return &create_claimed_account_parser;
        case 24:
            return &request_account_recovery_parser;
        case 25:
            return &recover_account_parser;
        case 26:
            return &change_recovery_account_parser;
        case 32:
            return &transfer_parser;
        case 33:
            return &transfer_from_savings_parser;
        case 34:
            return &cancel_transfer_from_savings_parser;
        case 36:
            return &decline_voting_rights_parser;
        case 37:
            return &reset_account_parser;
        case 38:
            return &set_reset_account_parser;
        case 39:
            return &claim_reward_balance_parser;
        case 40:
            return &delegate_vesting_shares_parser;
        case 44:
            return &create_proposal_parser;
        case 45:
            return &update_proposal_votes_parser;
        case 46:
            return &remove_proposal_parser;
        case 47:
            return &update_proposal_parser;
        case 48:
            return &collateralized_convert_parser;
        case 49:
            return &recurrent_transfer_parser;
        default:
            THROW(NOT_SUPPORTED);
    }
}

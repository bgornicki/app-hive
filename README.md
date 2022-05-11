# Hive Application for Ledger Nano S/X

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

![](https://user-images.githubusercontent.com/4411084/79356492-53928580-7f3f-11ea-929c-2b7e4ee4c8d5.jpg)

## What is Hive

Hive is an innovative and forward-looking decentralized blockchain and ecosystem, designed to scale with widespread adoption of the currency and platforms in mind. By combining the lightning-fast processing times and fee-less transactions, Hive is positioned to become one of the leading Web3 blockchains used by people around the world.

**Fast** - Transactions take less than 3 seconds

**Scalable** - Future proof resource-bandwidth & storage system

**Powerful** - Battle-tested for 5 years by hundreds of apps, communities & projects around the world.

Learn more: https://hive.io

---

Operations supported by Hive Ledger application:

- vote
- comment
- transfer
- transfer_to_vesting
- withdraw_vesting
- limit_order_create
- limit_order_cancel
- feed_publish
- convert
- account_create
- account_update
- witness_update
- account_witness_vote
- account_witness_proxy
- delete_comment
- custom_json
- comment_options
- set_withdraw_vesting_route
- claim_account
- create_claimed_account
- request_account_recovery
- recover_account
- change_recovery_account
- transfer_to_savings
- transfer_from_savings
- cancel_transfer_from_savings
- decline_voting_rights
- reset_account
- set_reset_account
- claim_reward_balance
- delegate_vesting_shares
- create_proposal
- update_proposal_votes
- remove_proposal
- update_proposal
- collateralized_convert
- recurrent_transfer

## Prerequisite

Be sure to have your environment correctly set up (see [Getting Started](https://developers.ledger.com/docs/nano-app/quickstart/)) and [ledgerblue](https://pypi.org/project/ledgerblue/) and installed.

## Local development

Download `ledger-app-builder`:

```
docker pull ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest
docker image tag ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest ledger-app-builder

```

Compile app in the container:

```
docker run --rm -ti -v "$(realpath .):/app" ledger-app-builder:latest make
```

You can compile the app in debug mode (should print additional information in speculos) with:

```
docker run --rm -ti -v "$(realpath .):/app" ledger-app-builder:latest make DEBUG=1
```

If you want to compile the app for Nano X model, enter the container:

```
docker run --rm -ti -v "$(realpath .):/app" ledger-app-builder:latest
```

And then run

```
BOLOS_SDK=$NANOX_SDK make
```

## Simulate with Speculos

Download speculos

```
docker pull ghcr.io/ledgerhq/speculos:latest
docker image tag ghcr.io/ledgerhq/speculos speculos

```

Run the app in speculos simulator

```bash
docker run --rm -it -v $(realpath .)/bin:/speculos/apps -p 1234:1234 -p 5000:5000 -p 40000:40000 -p 41000:41000 speculos --display headless --vnc-port 41000 --apdu-port 40000 apps/app.elf
```

Open `http://localhost:5000/` in your browser to see the simulator.

## Debug with Speculos

You can also debug this app with GDB thanks to speculos. First, make sure you have `gdb-multiarch` installed:

```bash
sudo apt install gdb-multiarch
```

Then, install `Native debug` extension in your vscode.

Run application in debug mode:

```bash
docker run --rm -it -v $(realpath .)/bin:/speculos/apps -p 1234:1234 -p 5000:5000 -p 40000:40000 -p 41000:41000 speculos --display headless --vnc-port 41000 --apdu-port 40000 --debug apps/app.elf
```

Connect your debugger with vscode by starting `Attach to gdbserver` configuration.

## Load the app to your physical Nano S

You need to have ledgerblue installed (should be handled in "Prerequisite" part of this Readme).

Load it with:

```
python3 -m ledgerblue.loadApp --curve secp256k1 --appFlags 0x240 --path "48'/13'" --tlv --targetId 0x31100004 --targetVersion="2.0.0" --delete --fileName bin/app.hex --appName "Hive" --appVersion "1.0.0" --dataSize $((0x`cat debug/app.map |grep _envram_data | tr -s ' ' | cut -f2 -d' '|cut -f2 -d'x'` - 0x`cat debug/app.map |grep _nvram_data | tr -s ' ' | cut -f2 -d' '|cut -f2 -d'x'`)) `ICONHEX=\`python3 ./dev-env/SDK/nanos-secure-sdk/icon3.py --hexbitmaponly icons/nanos_app_hive.gif  2>/dev/null\` ; [ ! -z "$ICONHEX" ] && echo "--icon $ICONHEX"`
```

## Tests

Big part of source code is covered by unit tests, written in `cmocka` framework.

### Unit testing

```
docker run --rm -ti -v "$(realpath .):/app" ledger-app-builder:latest sh -c "cd unit-tests && cmake -Bbuild -H. && make -C build && CTEST_OUTPUT_ON_FAILURE=1 make -C build test"
```

## Documentation

High level documentation such as [APDU](doc/APDU.md), [commands](doc/COMMANDS.md) are included in developer documentation which can be generated with [doxygen](https://www.doxygen.nl)

```
doxygen .doxygen/Doxyfile
```

the process outputs HTML and LaTeX documentations in `doc/html` and `doc/latex` folders.

## Tests & Continuous Integration

The flow processed in [Gitlab CI/CD](https://docs.gitlab.com/ee/ci/) is the following:

- Code formatting with [clang-format](http://clang.llvm.org/docs/ClangFormat.html)
- Compilation of the application for Ledger Nano S [ledger-app-builder](https://github.com/LedgerHQ/ledger-app-builder)
- Unit tests of C functions with [cmocka](https://cmocka.org/) (see [unit-tests/](unit-tests/))
- End-to-end tests with [Speculos](https://github.com/LedgerHQ/speculos) emulator (see [tests/](tests/))
- Code coverage with [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)/[lcov](http://ltp.sourceforge.net/coverage/lcov.php) and upload to [codecov.io](https://about.codecov.io)
- Documentation generation with [doxygen](https://www.doxygen.nl)

It outputs 4 artifacts:

- `hive-app` within output files of the compilation process
- `speculos-log` within APDU command/response when executing end-to-end tests
- `code-coverage` within HTML details of code coverage
- `documentation` within HTML auto-generated documentation

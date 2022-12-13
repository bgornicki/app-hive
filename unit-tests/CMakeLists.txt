cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# project information
project(unit_tests
  VERSION 0.1
  DESCRIPTION "Unit tests for Ledger Nano application"
  LANGUAGES C)

# guard against bad build-type strings
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CTest)
ENABLE_TESTING()

# specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -g -O0 --coverage")

set(GCC_COVERAGE_LINK_FLAGS "--coverage -lgcov")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

add_compile_definitions(
  TEST
  APPNAME="HIVE"
  HAVE_SECP256K1_CURVE
  APPVERSION="1.0.1"
  MAJOR_VERSION=1
  MINOR_VERSION=0
  PATCH_VERSION=1
  OS_IO_SEPROXYHAL
  HAVE_BAGL
  HAVE_UX_FLOW
  HAVE_SPRINTF
  HAVE_IO_USB
  HAVE_L4_USBLIB
  IO_USB_MAX_ENDPOINTS=6
  IO_HID_EP_LENGTH=64
  HAVE_USB_APDU
  USB_SEGMENT_SIZE=64
  BLE_SEGMENT_SIZE=32
  HAVE_WEBUSB
  WEBUSB_URL_SIZE_B=0
  WEBUSB_URL=""
  IO_SEPROXYHAL_BUFFER_SIZE_B=128
  HAVE_SHA224
  HAVE_HASH HAVE_RIPEMD160
  HAVE_ECC
  BAGL_WIDTH=128
  BAGL_HEIGHT=64)

include_directories(../src)
include_directories(/opt/nanos-secure-sdk/include)
include_directories(/opt/nanos-secure-sdk/lib_cxng/include)
include_directories(/opt/nanos-secure-sdk/lib_ux/include)
include_directories(/opt/nanos-secure-sdk/lib_bagl/include)
include_directories(/opt/nanos-secure-sdk/lib_stusb/include)

add_executable(test_format common/test_format.c)
add_executable(test_asn1 common/test_asn1.c)
add_executable(test_buffer common/test_buffer.c)
add_executable(test_base58 common/test_base58.c)
add_executable(test_bip32 common/test_bip32.c)
add_executable(test_transaction_parse transaction/test_transaction_parse.c)
add_executable(test_decoder_operation_name transaction/decoders/test_decoder_operation_name.c)
add_executable(test_decoder_string transaction/decoders/test_decoder_string.c)
add_executable(test_decoder_array_of_strings transaction/decoders/test_decoder_array_of_strings.c)
add_executable(test_decoder_boolean transaction/decoders/test_decoder_boolean.c)
add_executable(test_decoder_date_time transaction/decoders/test_decoder_date_time.c)
add_executable(test_decoder_uint8 transaction/decoders/test_decoder_uint8.c)
add_executable(test_decoder_uint16 transaction/decoders/test_decoder_uint16.c)
add_executable(test_decoder_uint64 transaction/decoders/test_decoder_uint64.c)
add_executable(test_decoder_uint32 transaction/decoders/test_decoder_uint32.c)
add_executable(test_decoder_array_of_u64 transaction/decoders/test_decoder_array_of_u64.c)
add_executable(test_decoder_weight transaction/decoders/test_decoder_weight.c)
add_executable(test_decoder_empty_extensions transaction/decoders/test_decoder_empty_extensions.c)
add_executable(test_decoder_asset transaction/decoders/test_decoder_asset.c)
add_executable(test_decoder_authority_type transaction/decoders/test_decoder_authority_type.c)
add_executable(test_decoder_optional_authority_type transaction/decoders/test_decoder_optional_authority_type.c)
add_executable(test_decoder_public_key transaction/decoders/test_decoder_public_key.c)
add_executable(test_decoder_beneficiaries_extensions transaction/decoders/test_decoder_beneficiaries_extensions.c)
add_executable(test_get_operation_parser transaction/test_get_operation_parser.c)
add_executable(test_wif common/test_wif.c)

add_library(format SHARED ../src/common/format.c)
add_library(asn1 SHARED ../src/common/asn1.c)
add_library(buffer SHARED ../src/common/buffer.c)
add_library(read SHARED ../src/common/read.c)
add_library(write SHARED ../src/common/write.c)
add_library(varint SHARED ../src/common/varint.c)
add_library(bip32 SHARED ../src/common/bip32.c)
add_library(wif SHARED ../src/common/wif.c)
add_library(base58 SHARED ../src/common/base58.c)
add_library(parsers SHARED ../src/transaction/parsers.c)
add_library(transaction_parse SHARED ../src/transaction/transaction_parse.c)
add_library(decoders SHARED ../src/transaction/decoders.c)
add_library(globals SHARED ../src/globals.c)
add_library(mocks SHARED mocks.c)

target_link_libraries(test_format PUBLIC cmocka gcov format)
target_link_libraries(test_asn1 PUBLIC cmocka gcov asn1 buffer read bip32 varint)
target_link_libraries(test_buffer PUBLIC cmocka gcov buffer asn1 read bip32 varint)
target_link_libraries(test_base58 PUBLIC cmocka gcov base58)
target_link_libraries(test_bip32 PUBLIC cmocka gcov bip32 read)
target_link_libraries(decoders buffer read asn1 bip32 wif base58 varint mocks -Wl,--wrap,cx_ripemd160_init_no_throw -Wl,--wrap,pic -Wl,--wrap,os_longjmp -Wl,--wrap,cx_hash_no_throw -Wl,--wrap,cx_hash_get_size)
target_link_libraries(wif -Wl,--wrap,cx_ripemd160_init_no_throw -Wl,--wrap,cx_hash_no_throw -Wl,--wrap,cx_hash_get_size,--wrap,os_longjmp)
target_link_libraries(transaction_parse decoders globals format asn1)
target_link_libraries(test_transaction_parse PUBLIC cmocka gcov mocks transaction_parse parsers decoders)
target_link_libraries(parsers -Wl,--wrap,os_longjmp)
target_link_libraries(test_decoder_operation_name PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_string PUBLIC cmocka gcov transaction_parse mocks varint)
target_link_libraries(test_decoder_array_of_strings PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_boolean PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_date_time PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_uint8 PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_uint16 PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_uint64 PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_uint32 PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_array_of_u64 PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_weight PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_empty_extensions PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_asset PUBLIC cmocka gcov transaction_parse mocks)
target_link_libraries(test_decoder_authority_type PUBLIC cmocka gcov transaction_parse wif mocks)
target_link_libraries(test_decoder_optional_authority_type PUBLIC cmocka gcov transaction_parse wif mocks)
target_link_libraries(test_decoder_public_key PUBLIC cmocka gcov transaction_parse wif mocks)
target_link_libraries(test_decoder_beneficiaries_extensions PUBLIC cmocka gcov transaction_parse wif mocks)
target_link_libraries(test_get_operation_parser PUBLIC cmocka gcov parsers transaction_parse mocks -Wl,--wrap,os_longjmp)
target_link_libraries(test_wif PUBLIC cmocka gcov wif base58 mocks -Wl,--wrap,os_longjmp)

add_test(test_format test_format)
add_test(test_asn1 test_asn1)
add_test(test_buffer test_buffer)
add_test(test_base58 test_base58)
add_test(test_bip32 test_bip32)
add_test(test_wif test_wif)
add_test(test_transaction_parse test_transaction_parse)
add_test(test_decoder_operation_name test_decoder_operation_name)
add_test(test_decoder_string test_decoder_string)
add_test(test_decoder_array_of_strings test_decoder_array_of_strings)
add_test(test_decoder_boolean test_decoder_boolean)
add_test(test_decoder_date_time test_decoder_date_time)
add_test(test_decoder_uint8 test_decoder_uint8)
add_test(test_decoder_uint16 test_decoder_uint16)
add_test(test_decoder_uint64 test_decoder_uint64)
add_test(test_decoder_uint32 test_decoder_uint32)
add_test(test_decoder_array_of_u64 test_decoder_array_of_u64)
add_test(test_decoder_weight test_decoder_weight)
add_test(test_decoder_empty_extensions test_decoder_empty_extensions)
add_test(test_decoder_asset test_decoder_asset)
add_test(test_decoder_authority_type test_decoder_authority_type)
add_test(test_decoder_optional_authority_type test_decoder_optional_authority_type)
add_test(test_decoder_public_key test_decoder_public_key)
add_test(test_get_operation_parser test_get_operation_parser)
add_test(test_decoder_beneficiaries_extensions test_decoder_beneficiaries_extensions)
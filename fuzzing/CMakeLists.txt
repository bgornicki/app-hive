cmake_minimum_required(VERSION 3.10)

project(HiveFuzzer VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# BOLOS SDK
set(BOLOS_SDK $ENV{BOLOS_SDK})
add_compile_definitions(APP_LOAD_PARAMS="--curve secp256k1")

add_compile_definitions(
  APPNAME="HIVE" 
  HAVE_SECP256K1_CURVE
  CX_CURVE_256K1=0x21
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

add_link_options(-Wl,--wrap,cx_ripemd160_init_no_throw -Wl,--wrap,pic -Wl,--wrap,os_longjmp -Wl,--wrap,cx_hash_no_throw -Wl,--wrap,cx_hash_get_size -Wl,--wrap,cx_ecdsa_sign) 

include_directories(.
        ../src/
        "${BOLOS_SDK}/include"
        "${BOLOS_SDK}/lib_bagl/include"
        "${BOLOS_SDK}/lib_cxng/include"
        "${BOLOS_SDK}/lib_stusb/include"
        "${BOLOS_SDK}/lib_ux/include"
)

add_compile_options(-g -O3)

# Build with code coverage generation
#if(CODE_COVERAGE)
#    if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
#        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
#        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
#    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
#        add_compile_options(-fprofile-arcs -ftest-coverage)
#        link_libraries(gcov)
#    else()
#        message(FATAL_ERROR "Unsupported compiler used with code coverage generation")
#    endif()
#endif()

# Fuzzer target
set(APP_SRC_DIR "../src")

set(APP_SOURCES
    ${APP_SRC_DIR}/globals.c
    #${APP_SRC_DIR}/crypto.c
    #${APP_SRC_DIR}/apdu/dispatcher.c
    #${APP_SRC_DIR}/apdu/parser.c
    #${APP_SRC_DIR}/handler/get_app_name.c
    #${APP_SRC_DIR}/handler/get_public_key.c
    #${APP_SRC_DIR}/handler/get_version.c
    #${APP_SRC_DIR}/handler/sign_tx.c
    ${APP_SRC_DIR}/transaction/decoders.c
    ${APP_SRC_DIR}/transaction/parsers.c
    ${APP_SRC_DIR}/transaction/transaction_parse.c
    ${APP_SRC_DIR}/common/asn1.c
    ${APP_SRC_DIR}/common/base58.c
    ${APP_SRC_DIR}/common/bip32.c
    ${APP_SRC_DIR}/common/buffer.c
    ${APP_SRC_DIR}/common/format.c
    ${APP_SRC_DIR}/common/read.c
    ${APP_SRC_DIR}/common/wif.c
)

add_executable(fuzz_hive
        fuzz_hive.c
        mocks.c
        ${APP_SOURCES}
)

target_include_directories(fuzz_hive PUBLIC ../src)
target_compile_options(fuzz_hive PUBLIC -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined)
target_link_options(fuzz_hive PUBLIC -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined)

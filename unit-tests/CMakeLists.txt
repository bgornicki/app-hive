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
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CTest)
ENABLE_TESTING()

# specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic -g -O0 --coverage -Wno-array-bounds")

set(GCC_COVERAGE_LINK_FLAGS "--coverage -lgcov")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

add_compile_definitions(TEST)

include_directories(../src)

add_executable(test_bip32 test_bip32.c)
add_executable(test_buffer test_buffer.c)
add_executable(test_format test_format.c)
add_executable(test_write test_write.c)
add_executable(test_apdu_parser test_apdu_parser.c)

add_library(bip32 SHARED ../src/common/bip32.c)
add_library(buffer SHARED ../src/common/buffer.c)
add_library(read SHARED ../src/common/read.c)
add_library(write SHARED ../src/common/write.c)
add_library(format SHARED ../src/common/format.c)
add_library(apdu_parser SHARED ../src/apdu/parser.c)

target_link_libraries(test_bip32 PUBLIC cmocka gcov bip32 buffer read)
target_link_libraries(test_buffer PUBLIC cmocka gcov buffer read)
target_link_libraries(test_format PUBLIC cmocka gcov format)
target_link_libraries(test_write PUBLIC cmocka gcov write)
target_link_libraries(test_apdu_parser PUBLIC cmocka gcov apdu_parser)

add_test(test_bip32 test_bip32)
add_test(test_buffer test_buffer)
add_test(test_format test_format)
add_test(test_write test_write)
add_test(test_apdu_parser test_apdu_parser)
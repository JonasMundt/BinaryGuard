
cmake_minimum_required(VERSION 3.21)

project(BinaryGuard VERSION 0.8.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Core library
add_library(binaryguard_core STATIC
    src/parser.c
    src/crc32.c
    src/file_io.c
)

target_include_directories(binaryguard_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# CLI application
add_executable(binaryguard src/main.c)

target_link_libraries(binaryguard PRIVATE
    binaryguard_core
)

# Unit tests
enable_testing()

add_executable(test_crc32 tests/test_crc32.c)
target_link_libraries(test_crc32 PRIVATE binaryguard_core)

add_executable(test_parser tests/test_parser.c)
target_link_libraries(test_parser PRIVATE binaryguard_core)

add_test(NAME CRC32_UnitTests COMMAND test_crc32)
add_test(NAME Parser_UnitTests COMMAND test_parser)

# CLI integration tests
function(add_cli_test name input expected_exit expected_text)
    add_test(
        NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            "-DPROGRAM=$<TARGET_FILE:binaryguard>"
            "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/${input}"
            "-DEXPECT_EXIT=${expected_exit}"
            "-DEXPECT_TEXT=${expected_text}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_cli.cmake"
    )
endfunction()

add_cli_test(
    CLI_ValidEmpty
    samples/valid-empty.bgd
    0 "CRC32: OK"
)

add_cli_test(
    CLI_ValidPayload
    samples/valid-nine.bgd
    0 "CRC32: OK"
)

add_cli_test(
    CLI_InvalidMagic
    src/main.c
    1 "Invalid magic bytes"
)

add_cli_test(
    CLI_TruncatedPayload
    samples/invalid-truncated.bgd
    1 "Payload is truncated"
)

add_cli_test(
    CLI_TrailingData
    samples/invalid-trailing.bgd
    1 "Unexpected trailing data"
)

add_cli_test(
    CLI_InvalidCRC
    samples/invalid-crc.bgd
    1 "CRC32 mismatch"
)

# File I/O component tests
add_executable(test_file_io tests/test_file_io.c)

target_link_libraries(test_file_io PRIVATE
    binaryguard_core
)

add_test(
    NAME FileIO_ComponentTests
    COMMAND test_file_io
        "${CMAKE_CURRENT_SOURCE_DIR}/samples"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
)

# Security and boundary tests
add_executable(test_boundaries
    tests/test_boundaries.c
)

target_link_libraries(test_boundaries PRIVATE
    binaryguard_core
)

add_test(
    NAME Parser_BoundaryTests
    COMMAND test_boundaries
)


# Local fuzz harness smoke test
add_executable(test_fuzz_smoke
    tests/fuzz_parser.c
    tests/test_fuzz_smoke.c
)

target_link_libraries(test_fuzz_smoke PRIVATE
    binaryguard_core
)

add_test(
    NAME Parser_FuzzSmoke
    COMMAND test_fuzz_smoke
)

# Optional Clang/libFuzzer build for CI
option(BG_ENABLE_FUZZING "Build fuzz targets" OFF)

if(BG_ENABLE_FUZZING)
    if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR
            "Fuzzing requires Clang"
        )
    endif()

    add_executable(fuzz_parser
        tests/fuzz_parser.c
        src/parser.c
    )

    target_include_directories(fuzz_parser PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_compile_options(fuzz_parser PRIVATE
        -fsanitize=fuzzer,address,undefined
        -fno-omit-frame-pointer
        -g
    )

    target_link_options(fuzz_parser PRIVATE
        -fsanitize=fuzzer,address,undefined
    )
endif()


# Strict compiler warnings
foreach(target binaryguard_core binaryguard test_crc32 test_parser test_file_io test_boundaries test_fuzz_smoke)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror
        )
    endif()
endforeach()


#include "crc32.h"
#include "file_io.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_FILE "binaryguard_limit_test.bgd"

// Store a 32-bit number in little-endian order.
static void write_u32_le(uint8_t *out, uint32_t value)
{
    for (unsigned int i = 0; i < 4u; ++i) {
        out[i] = (uint8_t)(value & 0xFFu);
        value >>= 8;
    }
}

// Create a temporary binary test file.
static int create_file(
    const uint8_t *header,
    const uint8_t *payload,
    size_t length,
    int add_trailing_byte)
{
    FILE *file = NULL;

#ifdef _MSC_VER
    if (fopen_s(&file, TEST_FILE, "wb") != 0) {
        return 0;
    }
#else
    file = fopen(TEST_FILE, "wb");
#endif

    if (file == NULL) {
        return 0;
    }

    int ok = fwrite(header, 1, BG_HEADER_SIZE, file)
             == BG_HEADER_SIZE;

    if (ok && length > 0) {
        ok = fwrite(payload, 1, length, file) == length;
    }

    if (ok && add_trailing_byte) {
        const uint8_t extra = 0xFF;
        ok = fwrite(&extra, 1, 1, file) == 1;
    }

    if (fclose(file) != 0) {
        ok = 0;
    }

    return ok;
}

// Create, load and validate one test case.
static int run_test(
    const char *name,
    const uint8_t *header,
    const uint8_t *payload,
    size_t length,
    int trailing,
    BgFileResult expected,
    BgResult expected_parser,
    uint32_t expected_crc)
{
    if (!create_file(header, payload, length, trailing)) {
        fprintf(stderr, "FAIL: %s (file creation)\n", name);
        return 1;
    }

    BgFile loaded = {0};
    BgResult parser_error = BG_OK;

    BgFileResult actual = bg_file_load(
        TEST_FILE, &loaded, &parser_error
    );

    int passed = actual == expected &&
                 parser_error == expected_parser;

    if (expected == BG_FILE_OK) {
        passed = passed &&
            loaded.payload != NULL &&
            loaded.header.payload_length == length &&
            loaded.header.crc32 == expected_crc &&
            memcmp(loaded.payload, payload, length) == 0 &&
            bg_crc32(loaded.payload, length) == expected_crc;
    } else {
        passed = passed &&
            loaded.payload == NULL &&
            loaded.header.payload_length == 0;
    }

    bg_file_free(&loaded);

    // Remove the generated file.
    if (remove(TEST_FILE) != 0) {
        passed = 0;
    }

    printf("%s: %s\n", passed ? "PASS" : "FAIL", name);
    return passed ? 0 : 1;
}

int main(void)
{
    const size_t max_size = BG_MAX_PAYLOAD_SIZE;

    uint8_t *payload = malloc(max_size);

    if (payload == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Generate deterministic test data.
    for (size_t i = 0; i < max_size; ++i) {
        payload[i] = (uint8_t)(i & 0xFFu);
    }

    uint32_t crc = bg_crc32(payload, max_size);

    uint8_t header[BG_HEADER_SIZE] = {
        'B', 'G', 'R', 'D', 1
    };

    write_u32_le(header + 8, BG_MAX_PAYLOAD_SIZE);
    write_u32_le(header + 12, crc);

    int failures = 0;

    // Test 1: Exact maximum size.
    failures += run_test(
        "Exact 1 MiB",
        header, payload, max_size, 0,
        BG_FILE_OK, BG_OK, crc
    );

    // Test 2: One byte missing.
    failures += run_test(
        "Truncated by one byte",
        header, payload, max_size - 1, 0,
        BG_FILE_TRUNCATED_PAYLOAD, BG_OK, crc
    );

    // Test 3: One unexpected extra byte.
    failures += run_test(
        "Unexpected trailing byte",
        header, payload, max_size, 1,
        BG_FILE_TRAILING_DATA, BG_OK, crc
    );

    // Test 4: Declared size exceeds the limit.
    write_u32_le(
        header + 8,
        BG_MAX_PAYLOAD_SIZE + 1u
    );

    failures += run_test(
        "Payload limit exceeded",
        header, payload, 0, 0,
        BG_FILE_INVALID_HEADER,
        BG_ERROR_SIZE_LIMIT, crc
    );

    free(payload);

    printf("\nPayload limit test failures: %d\n",
           failures);

    return failures == 0 ? 0 : 1;
}


#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Verify that invalid input is rejected
// without modifying the output structure.
static int expect_error(
    const char *name,
    const uint8_t *data,
    size_t length,
    BgResult expected)
{
    BgHeader header = {
        77, 77, 12345, 67890
    };

    BgResult actual = bg_parse_header(
        data, length, &header
    );

    int passed =
        actual == expected &&
        header.version == 77 &&
        header.flags == 77 &&
        header.payload_length == 12345 &&
        header.crc32 == 67890;

    printf("%s: %s\n",
           passed ? "PASS" : "FAIL",
           name);

    return passed ? 0 : 1;
}

// Verify correctly parsed values.
static int expect_success(
    const char *name,
    const uint8_t *data,
    uint32_t expected_length,
    uint32_t expected_crc)
{
    BgHeader header = {0};

    BgResult result = bg_parse_header(
        data, BG_HEADER_SIZE, &header
    );

    int passed =
        result == BG_OK &&
        header.version == 1 &&
        header.flags == 0 &&
        header.payload_length == expected_length &&
        header.crc32 == expected_crc;

    printf("%s: %s\n",
           passed ? "PASS" : "FAIL",
           name);

    return passed ? 0 : 1;
}

int main(void)
{
    int failures = 0;

    uint8_t data[BG_HEADER_SIZE] = {
        'B', 'G', 'R', 'D',
        1, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    // Test every incomplete header length.
    for (size_t length = 0;
         length < BG_HEADER_SIZE;
         ++length) {

        char name[64];

        snprintf(name, sizeof(name),
                 "Header length %zu", length);

        failures += expect_error(
            name, data, length,
            BG_ERROR_SHORT_HEADER);
    }

    // Exact maximum payload: 1 MiB.
    data[10] = 0x10;

    failures += expect_success(
        "Maximum payload size",
        data,
        BG_MAX_PAYLOAD_SIZE,
        0
    );

    // Exceed the maximum by one byte.
    data[8] = 1;

    failures += expect_error(
        "Maximum exceeded by one",
        data, sizeof(data),
        BG_ERROR_SIZE_LIMIT
    );

    // Reject the maximum uint32 value.
    data[8] = 0xFF;
    data[9] = 0xFF;
    data[10] = 0xFF;
    data[11] = 0xFF;

    failures += expect_error(
        "UINT32_MAX payload",
        data, sizeof(data),
        BG_ERROR_SIZE_LIMIT
    );

    // Verify little-endian decoding.
    data[8] = 0x34;
    data[9] = 0x12;
    data[10] = 0;
    data[11] = 0;

    data[12] = 0x78;
    data[13] = 0x56;
    data[14] = 0x34;
    data[15] = 0x12;

    failures += expect_success(
        "Little-endian decoding",
        data,
        UINT32_C(0x1234),
        UINT32_C(0x12345678)
    );

    // A null input must be rejected.
    failures += expect_error(
        "Null input",
        NULL, BG_HEADER_SIZE,
        BG_ERROR_INVALID_ARGUMENT
    );

    printf("\nBoundary test failures: %d\n",
           failures);

    return failures == 0 ? 0 : 1;
}

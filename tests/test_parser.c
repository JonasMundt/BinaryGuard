
#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int check_result(
    const char *name,
    const uint8_t *data,
    size_t length,
    BgHeader *output,
    BgResult expected)
{
    BgResult actual = bg_parse_header(data, length, output);

    if (actual != expected) {
        fprintf(stderr, "FAIL: %s\n", name);
        return 1;
    }

    printf("PASS: %s\n", name);
    return 0;
}

int main(void)
{
    int failures = 0;

    const uint8_t valid[BG_HEADER_SIZE] = {
        'B', 'G', 'R', 'D',
        1, 0, 0, 0,
        9, 0, 0, 0,
        0x26, 0x39, 0xF4, 0xCB
    };

    BgHeader header = {0};

    // Valid header
    failures += check_result(
        "Valid header",
        valid, sizeof(valid), &header, BG_OK);

    // Verify parsed values
    if (header.version != 1 ||
        header.flags != 0 ||
        header.payload_length != 9 ||
        header.crc32 != UINT32_C(0xCBF43926)) {
        fprintf(stderr, "FAIL: Parsed header fields\n");
        failures++;
    } else {
        printf("PASS: Parsed header fields\n");
    }

    // Truncated header
    failures += check_result(
        "Short header",
        valid, 15, &header, BG_ERROR_SHORT_HEADER);

    // Invalid argument tests
    failures += check_result(
        "Null input",
        NULL, BG_HEADER_SIZE,
        &header, BG_ERROR_INVALID_ARGUMENT);

    failures += check_result(
        "Null output",
        valid, BG_HEADER_SIZE,
        NULL, BG_ERROR_INVALID_ARGUMENT);

    // Invalid header field tests
    struct TestCase {
        const char *name;
        size_t offset;
        uint8_t value;
        BgResult expected;
    };

    const struct TestCase cases[] = {
        {"Invalid magic", 0, 'X', BG_ERROR_MAGIC},
        {"Invalid version", 4, 2, BG_ERROR_VERSION},
        {"Invalid flags", 5, 1, BG_ERROR_FLAGS},
        {"Invalid reserved byte 1", 6, 1, BG_ERROR_FLAGS},
        {"Invalid reserved byte 2", 7, 1, BG_ERROR_FLAGS},
        {"Payload too large", 10, 0x10, BG_ERROR_SIZE_LIMIT}
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        uint8_t modified[BG_HEADER_SIZE];

        memcpy(modified, valid, sizeof(modified));
        modified[cases[i].offset] = cases[i].value;

        failures += check_result(
            cases[i].name,
            modified,
            sizeof(modified),
            &header,
            cases[i].expected);
    }

    return failures == 0 ? 0 : 1;
}


#include "crc32.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static int check_crc(
    const char *name,
    const uint8_t *data,
    size_t length,
    uint32_t expected)
{
    uint32_t actual = bg_crc32(data, length);

    if (actual != expected) {
        fprintf(stderr,
                "FAIL: %s (expected 0x%08" PRIX32
                ", got 0x%08" PRIX32 ")\n",
                name, expected, actual);
        return 1;
    }

    printf("PASS: %s\n", name);
    return 0;
}

int main(void)
{
    int failures = 0;

    // Test 1: Empty payload
    failures += check_crc(
        "Empty payload", NULL, 0, UINT32_C(0));

    // Test 2: Standard CRC32 test vector
    const uint8_t digits[] = "123456789";

    failures += check_crc(
        "Standard test vector",
        digits,
        sizeof(digits) - 1,
        UINT32_C(0xCBF43926));

    // Test 3: Modified payload
    const uint8_t modified[] = "023456789";

    failures += check_crc(
        "Modified payload",
        modified,
        sizeof(modified) - 1,
        UINT32_C(0xDC8F2D65));

    return failures == 0 ? 0 : 1;
}

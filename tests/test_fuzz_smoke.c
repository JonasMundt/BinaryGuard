
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int LLVMFuzzerTestOneInput(
    const uint8_t *data,
    size_t size);

int main(void)
{
    const uint8_t valid[16] = {
        'B', 'G', 'R', 'D',
        1, 0, 0, 0,
        9, 0, 0, 0,
        0x26, 0x39, 0xF4, 0xCB
    };

    uint8_t invalid[16];

    memcpy(invalid, valid, sizeof(invalid));
    invalid[0] = 'X';

    // Valid header
    LLVMFuzzerTestOneInput(valid, sizeof(valid));

    // Incomplete header
    LLVMFuzzerTestOneInput(valid, 5);

    // Empty input
    LLVMFuzzerTestOneInput(valid, 0);

    // Invalid magic bytes
    LLVMFuzzerTestOneInput(invalid, sizeof(invalid));

    // Unsupported version
    invalid[0] = 'B';
    invalid[4] = 2;
    LLVMFuzzerTestOneInput(invalid, sizeof(invalid));

    printf("Fuzz harness smoke tests passed\n");

    return 0;
}


#include "parser.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Decode a little-endian 32-bit integer.
static uint32_t expected_u32(const uint8_t *data)
{
    return (uint32_t)data[0]
         | ((uint32_t)data[1] << 8)
         | ((uint32_t)data[2] << 16)
         | ((uint32_t)data[3] << 24);
}

// Entry point used by libFuzzer.
int LLVMFuzzerTestOneInput(
    const uint8_t *data,
    size_t size)
{
    // Initialize output with recognizable values.
    BgHeader header = {
        0xA5,
        0xA5,
        UINT32_MAX,
        UINT32_MAX
    };

    BgResult result = bg_parse_header(
        data, size, &header
    );

    if (result == BG_OK) {
        // A successful parse must have a valid header.
        if (size < BG_HEADER_SIZE ||
            memcmp(data, "BGRD", 4) != 0 ||
            data[4] != 1 ||
            data[5] != 0 ||
            data[6] != 0 ||
            data[7] != 0) {
            abort();
        }

        // Verify decoded header fields.
        if (header.version != 1 ||
            header.flags != 0 ||
            header.payload_length !=
                expected_u32(data + 8) ||
            header.crc32 !=
                expected_u32(data + 12) ||
            header.payload_length >
                BG_MAX_PAYLOAD_SIZE) {
            abort();
        }
    } else {
        // Failed parsing must leave output unchanged.
        if (header.version != 0xA5 ||
            header.flags != 0xA5 ||
            header.payload_length != UINT32_MAX ||
            header.crc32 != UINT32_MAX) {
            abort();
        }

        // Every incomplete header must be rejected.
        if (size < BG_HEADER_SIZE &&
            result != BG_ERROR_SHORT_HEADER) {
            abort();
        }
    }

    return 0;
}

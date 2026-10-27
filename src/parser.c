
#include "parser.h"

#include <string.h>

// Read an unsigned 32-bit little-endian integer
static uint32_t read_u32_le(const uint8_t *data)
{
    return (uint32_t)data[0]
         | ((uint32_t)data[1] << 8)
         | ((uint32_t)data[2] << 16)
         | ((uint32_t)data[3] << 24);
}

BgResult bg_parse_header(
    const uint8_t *bytes,
    size_t length,
    BgHeader *out)
{
    if (bytes == NULL || out == NULL) {
        return BG_ERROR_INVALID_ARGUMENT;
    }

    if (length < BG_HEADER_SIZE) {
        return BG_ERROR_SHORT_HEADER;
    }

    if (memcmp(bytes, "BGRD", 4) != 0) {
        return BG_ERROR_MAGIC;
    }

    if (bytes[4] != 1) {
        return BG_ERROR_VERSION;
    }

    if (bytes[5] != 0 ||
        bytes[6] != 0 ||
        bytes[7] != 0) {
        return BG_ERROR_FLAGS;
    }

    uint32_t payload_length = read_u32_le(bytes + 8);

    if (payload_length > BG_MAX_PAYLOAD_SIZE) {
        return BG_ERROR_SIZE_LIMIT;
    }

    out->version = bytes[4];
    out->flags = bytes[5];
    out->payload_length = payload_length;
    out->crc32 = read_u32_le(bytes + 12);

    return BG_OK;
}

const char *bg_result_message(BgResult result)
{
    switch (result) {
        case BG_OK:
            return "Success";
        case BG_ERROR_INVALID_ARGUMENT:
            return "Invalid parser argument";
        case BG_ERROR_SHORT_HEADER:
            return "File header is too short";
        case BG_ERROR_MAGIC:
            return "Invalid magic bytes";
        case BG_ERROR_VERSION:
            return "Unsupported format version";
        case BG_ERROR_FLAGS:
            return "Invalid flags or reserved bytes";
        case BG_ERROR_SIZE_LIMIT:
            return "Payload exceeds maximum size";
        default:
            return "Unknown parser error";
    }
}

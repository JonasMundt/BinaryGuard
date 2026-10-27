
#ifndef BG_PARSER_H
#define BG_PARSER_H

#include <stddef.h>
#include <stdint.h>

#define BG_HEADER_SIZE 16u
#define BG_MAX_PAYLOAD_SIZE 1048576u

typedef struct {
    uint8_t version;
    uint8_t flags;
    uint32_t payload_length;
    uint32_t crc32;
} BgHeader;

typedef enum {
    BG_OK = 0,
    BG_ERROR_INVALID_ARGUMENT,
    BG_ERROR_SHORT_HEADER,
    BG_ERROR_MAGIC,
    BG_ERROR_VERSION,
    BG_ERROR_FLAGS,
    BG_ERROR_SIZE_LIMIT
} BgResult;

BgResult bg_parse_header(
    const uint8_t *bytes,
    size_t length,
    BgHeader *out
);

const char *bg_result_message(BgResult result);

#endif

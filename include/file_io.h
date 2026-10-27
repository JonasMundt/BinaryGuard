
#ifndef BG_FILE_IO_H
#define BG_FILE_IO_H

#include "parser.h"

#include <stdint.h>

// A loaded BinaryGuard file.
typedef struct {
    BgHeader header;
    uint8_t *payload;
} BgFile;

// Possible file-loading results.
typedef enum {
    BG_FILE_OK = 0,
    BG_FILE_INVALID_ARGUMENT,
    BG_FILE_OPEN_FAILED,
    BG_FILE_READ_FAILED,
    BG_FILE_SHORT_HEADER,
    BG_FILE_INVALID_HEADER,
    BG_FILE_OUT_OF_MEMORY,
    BG_FILE_TRUNCATED_PAYLOAD,
    BG_FILE_TRAILING_DATA,
    BG_FILE_CLOSE_FAILED
} BgFileResult;

// Load a file and validate its structure.
// On success, the caller owns the payload.
// On failure, out remains unchanged.
// out must not already own an allocation.
BgFileResult bg_file_load(
    const char *path,
    BgFile *out,
    BgResult *parser_error
);

// Release payload memory and reset the structure.
void bg_file_free(BgFile *file);

// Return a readable error message.
const char *bg_file_result_message(BgFileResult result);

#endif

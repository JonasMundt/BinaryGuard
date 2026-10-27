
#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>

// Open a binary file for reading.
static FILE *bg_open_read(const char *path)
{
    FILE *file = NULL;

#ifdef _MSC_VER
    (void)fopen_s(&file, path, "rb");
#else
    file = fopen(path, "rb");
#endif

    return file;
}

void bg_file_free(BgFile *file)
{
    if (file == NULL) {
        return;
    }

    free(file->payload);
    *file = (BgFile){0};
}

BgFileResult bg_file_load(
    const char *path,
    BgFile *out,
    BgResult *parser_error)
{
    if (path == NULL || out == NULL ||
        parser_error == NULL) {
        return BG_FILE_INVALID_ARGUMENT;
    }

    *parser_error = BG_OK;

    // Temporary result until loading succeeds.
    BgFile loaded = {0};

    FILE *file = bg_open_read(path);

    if (file == NULL) {
        return BG_FILE_OPEN_FAILED;
    }

    BgFileResult result = BG_FILE_OK;
    uint8_t bytes[BG_HEADER_SIZE];

    // Read header.
    size_t bytes_read = fread(
        bytes, 1, sizeof(bytes), file
    );

    if (bytes_read != sizeof(bytes)) {
        result = ferror(file)
            ? BG_FILE_READ_FAILED
            : BG_FILE_SHORT_HEADER;
        goto cleanup;
    }

    // Parse and validate header.
    *parser_error = bg_parse_header(
        bytes,
        sizeof(bytes),
        &loaded.header
    );

    if (*parser_error != BG_OK) {
        result = BG_FILE_INVALID_HEADER;
        goto cleanup;
    }

    // Allocate and read payload.
    if (loaded.header.payload_length > 0) {
        size_t length =
            (size_t)loaded.header.payload_length;

        loaded.payload = malloc(length);

        if (loaded.payload == NULL) {
            result = BG_FILE_OUT_OF_MEMORY;
            goto cleanup;
        }

        size_t payload_read = fread(
            loaded.payload, 1, length, file
        );

        if (payload_read != length) {
            result = ferror(file)
                ? BG_FILE_READ_FAILED
                : BG_FILE_TRUNCATED_PAYLOAD;
            goto cleanup;
        }
    }

    // Reject trailing data.
    if (fgetc(file) != EOF) {
        result = BG_FILE_TRAILING_DATA;
        goto cleanup;
    }

    if (ferror(file)) {
        result = BG_FILE_READ_FAILED;
        goto cleanup;
    }

cleanup:
    // Always close the file.
    if (fclose(file) != 0 && result == BG_FILE_OK) {
        result = BG_FILE_CLOSE_FAILED;
    }

    if (result == BG_FILE_OK) {
        // Transfer ownership to the caller.
        *out = loaded;
    } else {
        // Release memory on every error path.
        bg_file_free(&loaded);
    }

    return result;
}

const char *bg_file_result_message(BgFileResult result)
{
    switch (result) {
        case BG_FILE_OK:
            return "Success";
        case BG_FILE_INVALID_ARGUMENT:
            return "Invalid file argument";
        case BG_FILE_OPEN_FAILED:
            return "Cannot open file";
        case BG_FILE_READ_FAILED:
            return "Error reading file";
        case BG_FILE_SHORT_HEADER:
            return "File header is too short";
        case BG_FILE_INVALID_HEADER:
            return "Invalid file header";
        case BG_FILE_OUT_OF_MEMORY:
            return "Memory allocation failed";
        case BG_FILE_TRUNCATED_PAYLOAD:
            return "Payload is truncated";
        case BG_FILE_TRAILING_DATA:
            return "Unexpected trailing data";
        case BG_FILE_CLOSE_FAILED:
            return "Error closing file";
        default:
            return "Unknown file error";
    }
}

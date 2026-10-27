
#include "file_io.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_PATH_SIZE 1024u

static int make_path(
    char *buffer,
    size_t capacity,
    const char *directory,
    const char *filename)
{
    int n = snprintf(
        buffer, capacity, "%s/%s",
        directory, filename
    );

    return n >= 0 && (size_t)n < capacity;
}

// Test successful file loading.
static int test_valid(
    const char *directory,
    const char *filename,
    const uint8_t *expected_data,
    size_t expected_length,
    uint32_t expected_crc)
{
    char path[TEST_PATH_SIZE];

    if (!make_path(path, sizeof(path),
                   directory, filename)) {
        return 1;
    }

    BgFile file = {0};
    BgResult parser_error = BG_OK;

    BgFileResult result = bg_file_load(
        path, &file, &parser_error
    );

    int passed =
        result == BG_FILE_OK &&
        parser_error == BG_OK;

    if (passed) {
        passed =
            file.header.version == 1 &&
            file.header.flags == 0 &&
            file.header.payload_length == expected_length &&
            file.header.crc32 == expected_crc;

        if (expected_length > 0) {
            passed = passed &&
                file.payload != NULL &&
                memcmp(file.payload,
                       expected_data,
                       expected_length) == 0;
        } else {
            passed = passed && file.payload == NULL;
        }
    }

    // Memory must always be released.
    bg_file_free(&file);

    // Verify that cleanup reset the structure.
    passed = passed &&
        file.payload == NULL &&
        file.header.payload_length == 0;

    printf("%s: %s\n",
           passed ? "PASS" : "FAIL",
           filename);

    return passed ? 0 : 1;
}

// Test expected loading failures.
static int test_error(
    const char *path,
    BgFileResult expected,
    BgResult expected_parser)
{
    BgFile file = {0};
    BgResult parser_error = BG_OK;

    BgFileResult result = bg_file_load(
        path, &file, &parser_error
    );

    int passed =
        result == expected &&
        parser_error == expected_parser &&
        file.payload == NULL &&
        file.header.version == 0 &&
        file.header.payload_length == 0;

    bg_file_free(&file);

    printf("%s: %s\n",
           passed ? "PASS" : "FAIL",
           path);

    return passed ? 0 : 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr,
                "Usage: test_file_io <samples> <invalid-file>\n");
        return 1;
    }

    int failures = 0;
    char path[TEST_PATH_SIZE];

    const uint8_t digits[] = "123456789";

    // Valid files
    failures += test_valid(
        argv[1], "valid-empty.bgd",
        NULL, 0, UINT32_C(0));

    failures += test_valid(
        argv[1], "valid-nine.bgd",
        digits, sizeof(digits) - 1,
        UINT32_C(0xCBF43926));

    // Truncated payload
    if (!make_path(path, sizeof(path),
                   argv[1], "invalid-truncated.bgd")) {
        return 1;
    }

    failures += test_error(
        path, BG_FILE_TRUNCATED_PAYLOAD, BG_OK);

    // Unexpected trailing data
    if (!make_path(path, sizeof(path),
                   argv[1], "invalid-trailing.bgd")) {
        return 1;
    }

    failures += test_error(
        path, BG_FILE_TRAILING_DATA, BG_OK);

    // Invalid header
    failures += test_error(
        argv[2], BG_FILE_INVALID_HEADER,
        BG_ERROR_MAGIC);

    // Missing file
    if (!make_path(path, sizeof(path),
                   argv[1], "nonexistent-test-file.bgd")) {
        return 1;
    }

    failures += test_error(
        path, BG_FILE_OPEN_FAILED, BG_OK);

    // Invalid arguments
    BgFile file = {0};
    BgResult parser_error = BG_OK;

    int arguments_ok =
        bg_file_load(NULL, &file, &parser_error)
            == BG_FILE_INVALID_ARGUMENT &&
        bg_file_load("unused", NULL, &parser_error)
            == BG_FILE_INVALID_ARGUMENT &&
        bg_file_load("unused", &file, NULL)
            == BG_FILE_INVALID_ARGUMENT;

    bg_file_free(NULL);

    printf("%s: Invalid arguments\n",
           arguments_ok ? "PASS" : "FAIL");

    if (!arguments_ok) {
        failures++;
    }

    return failures == 0 ? 0 : 1;
}

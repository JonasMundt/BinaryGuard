
#include "crc32.h"
#include "file_io.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: binaryguard <file.bgd>\n");
        return 1;
    }

    BgFile file = {0};
    BgResult parser_error = BG_OK;

    // Load and validate file structure.
    BgFileResult result = bg_file_load(
        argv[1],
        &file,
        &parser_error
    );

    if (result != BG_FILE_OK) {
        if (result == BG_FILE_INVALID_HEADER) {
            fprintf(stderr, "Error: %s\n",
                    bg_result_message(parser_error));
        } else {
            fprintf(stderr, "Error: %s\n",
                    bg_file_result_message(result));
        }

        return 1;
    }

    // Validate payload checksum.
    uint32_t calculated_crc = bg_crc32(
        file.payload,
        (size_t)file.header.payload_length
    );

    if (calculated_crc != file.header.crc32) {
        fprintf(stderr,
                "Error: CRC32 mismatch "
                "(expected 0x%08" PRIX32
                ", calculated 0x%08" PRIX32 ")\n",
                file.header.crc32,
                calculated_crc);

        bg_file_free(&file);
        return 1;
    }

    // Display file information.
    printf("BinaryGuard v1.0.0\n");
    printf("Magic: BGRD\n");
    printf("Version: %u\n",
           (unsigned int)file.header.version);
    printf("Payload length: %" PRIu32 " bytes\n",
           file.header.payload_length);
    printf("CRC32 (stored): 0x%08" PRIX32 "\n",
           file.header.crc32);
    printf("Payload size: OK\n");
    printf("CRC32: OK\n");

    // Release allocated memory.
    bg_file_free(&file);

    return 0;
}


#include "crc32.h"

uint32_t bg_crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = UINT32_MAX;

    for (size_t i = 0; i < length; ++i) {
        crc ^= (uint32_t)data[i];

        for (unsigned int bit = 0; bit < 8u; ++bit) {
            if ((crc & 1u) != 0u) {
                crc = (crc >> 1) ^ UINT32_C(0xEDB88320);
            } else {
                crc >>= 1;
            }
        }
    }

    return crc ^ UINT32_MAX;
}

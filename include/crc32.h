
#ifndef BG_CRC32_H
#define BG_CRC32_H

#include <stddef.h>
#include <stdint.h>

// Calculate CRC-32/ISO-HDLC (IEEE).
// data may be NULL only when length is zero.
uint32_t bg_crc32(const uint8_t *data, size_t length);

#endif

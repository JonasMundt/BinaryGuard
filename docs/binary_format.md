
# BinaryGuard Binary Format Specification

## Format Version 1

File extension: `.bgd`

BinaryGuard uses a custom binary file format with a
fixed 16-byte header followed by a variable-length payload.

All multi-byte integers use little-endian byte order.

## Header Layout

| Offset | Size | Field | Description |
|---|---|---|---|
| 0 | 4 | Magic | ASCII "BGRD" |
| 4 | 1 | Version | Must be 1 |
| 5 | 1 | Flags | Must be 0 |
| 6 | 2 | Reserved | Must be 0 |
| 8 | 4 | Payload Length | Unsigned 32-bit integer |
| 12 | 4 | CRC32 | Checksum of payload |

## Payload

- Located immediately after the 16-byte header.
- Maximum supported size: 1 MiB (1,048,576 bytes).
- Payload may be empty.
- No trailing bytes are permitted.

## CRC32

- Algorithm: CRC-32/ISO-HDLC (IEEE)
- Polynomial (reflected): 0xEDB88320
- Initial value: 0xFFFFFFFF
- Final XOR: 0xFFFFFFFF
- Checksum covers payload bytes only.
- Empty payload CRC32: 0x00000000

## Validation Rules

A file is invalid if:

1. The file is shorter than 16 bytes.
2. Magic bytes do not match "BGRD".
3. Version is unsupported.
4. Flags or reserved bytes are nonzero.
5. Payload length exceeds the allowed limit.
6. Actual file size differs from header + payload length.
7. CRC32 does not match the payload.

The parser must reject malformed files without
out-of-bounds access or resource leaks.

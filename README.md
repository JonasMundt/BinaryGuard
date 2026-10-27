
# BinaryGuard

**A defensive binary file parser and validator written in C17.**

BinaryGuard is a lightweight command-line application designed to safely parse and validate a custom binary file format.

The project demonstrates low-level C programming, explicit memory management, defensive input validation, automated testing, and cross-platform continuous integration.

## Features

- Binary file parsing using C17
- Custom, versioned binary file format
- Header and metadata validation
- Little-endian integer decoding
- CRC32 payload integrity verification
- Detection of truncated and malformed files
- Rejection of unexpected trailing data
- Bounded dynamic memory allocation (max. 1 MiB payload)
- Structured error handling and resource cleanup
- Automated unit, component, and integration tests
- AddressSanitizer and UndefinedBehaviorSanitizer testing
- Coverage-guided fuzzing with libFuzzer
- Windows and Linux CI builds

## Technology Stack

| Technology | Purpose |
|---|---|
| C17 | Core implementation |
| CMake | Cross-platform build system |
| CTest | Automated testing |
| MSVC / Clang | Compilation and diagnostics |
| AddressSanitizer | Memory error detection |
| UndefinedBehaviorSanitizer | Undefined behavior detection |
| libFuzzer | Coverage-guided fuzz testing |
| GitHub Actions | Continuous integration |

## Binary File Format

BinaryGuard uses a custom `.bgd` format consisting of a fixed 16-byte header followed by a variable-length payload.

All multi-byte integers are stored in little-endian byte order.

| Offset | Size | Field | Description |
|---|---|---|---|
| 0 | 4 bytes | Magic | ASCII `BGRD` |
| 4 | 1 byte | Version | Must be `1` |
| 5 | 1 byte | Flags | Must be `0` |
| 6 | 2 bytes | Reserved | Must be `0` |
| 8 | 4 bytes | Payload Length | Unsigned 32-bit integer |
| 12 | 4 bytes | CRC32 | Payload checksum |
| 16 | Variable | Payload | Maximum 1 MiB |

CRC32 uses the CRC-32/ISO-HDLC (IEEE) algorithm.

CRC32 detects accidental data corruption. It is not a cryptographic authentication mechanism.

See [Binary Format Specification](docs/binary_format.md) for additional details.

## Building

### Windows

Requirements:

- Visual Studio 2022 with MSVC C/C++ tools
- CMake 3.21+
- Ninja

Open an x64 Developer Command Prompt and run:

```cmd
cmake -S . -B build -G Ninja
cmake --build build
```

### Linux

Requirements:

- C17-compatible compiler
- CMake 3.21+
- Ninja

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage

On Windows:

```cmd
build\binaryguard.exe samples\valid-nine.bgd
```

Example output:

```text
BinaryGuard v1.0.0.
Magic: BGRD
Version: 1
Payload length: 9 bytes
CRC32 (stored): 0xCBF43926
Payload size: OK
CRC32: OK
```

Corrupted files are rejected with an explanatory error message and a nonzero exit status.

Example:

```text
Error: CRC32 mismatch (expected 0xCBF43926, calculated 0xDC8F2D65)
```

## Architecture

The project separates responsibilities into several modules:

| Module | Responsibility |
|---|---|
| `main.c` | Command-line interface and output |
| `parser.c` | Binary header parsing and validation |
| `file_io.c` | File loading and memory ownership |
| `crc32.c` | CRC32 checksum calculation |
| `tests/` | Automated validation and fuzz testing |

The parser works independently of the CLI, allowing it to be tested directly using in-memory input.

## Testing

Run all registered tests:

```cmd
ctest --test-dir build --output-on-failure
```

The test suite includes:

- CRC32 verification
- Header parsing
- Invalid and incomplete headers
- CLI integration tests
- File I/O component tests
- Payload boundary tests
- Fuzz harness smoke tests

The current suite contains 13 CTest tests.

### Memory Safety

AddressSanitizer is used to detect supported classes of memory errors.

Linux CI additionally combines AddressSanitizer with UndefinedBehaviorSanitizer.

### Fuzz Testing

The parser is tested with libFuzzer using automatically generated and mutated binary inputs.

The GitHub Actions workflow runs a bounded fuzzing session of 10,000 executions.

Fuzzing and sanitizer checks increase confidence in robustness but do not prove the absence of all defects.

## Continuous Integration

GitHub Actions automatically executes:

1. Windows builds and CTest using MSVC
2. Linux builds and CTest using Clang, ASan and UBSan
3. libFuzzer testing on Linux

The workflow is located at `.github/workflows/ci.yml`.

## Project Goals

BinaryGuard was developed as a software engineering portfolio project to demonstrate:

- System-level programming in C
- Manual memory and resource management
- Defensive handling of untrusted binary input
- Modular software architecture
- Automated testing and debugging
- Cross-platform builds and continuous integration

## Limitations

- Supports the documented format version 1 only
- Maximum payload size: 1 MiB
- Read-only CLI; files are not created or modified
- CRC32 provides error detection, not cryptographic security
- No graphical user interface

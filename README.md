
# BinaryGuard

A defensive binary file parser and validator written in C17.

BinaryGuard validates a custom binary file format,
including header structure, versioning, payload length,
and CRC32 integrity.

## Features

- C17 binary file parsing
- Strict header and payload validation
- CRC32 integrity checking
- Defensive memory and resource management
- Unit, component and CLI integration tests
- AddressSanitizer and UndefinedBehaviorSanitizer
- libFuzzer fuzz testing
- Cross-platform CMake builds

## Build

Requires CMake 3.21+ and a C17 compiler.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Usage

```sh
binaryguard samples/valid-nine.bgd
```

## Binary Format

See [format specification](docs/binary_format.md).

## Testing

CTest runs the automated test suite.

Additional sanitizer and fuzzing checks
run through GitHub Actions.

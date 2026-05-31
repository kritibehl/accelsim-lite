# ASan Report

## Scope

AddressSanitizer-style build profile for AccelSim-Lite.

## Command

```bash
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
Verified status

ASan profile passed in the CMake build matrix.

Claims boundary

This documents sanitizer-profile validation for local C++ test workflows.

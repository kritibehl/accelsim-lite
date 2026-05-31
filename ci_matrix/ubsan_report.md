
UBSan Report
Scope

UndefinedBehaviorSanitizer-style build profile for AccelSim-Lite.

Command
cmake -S . -B build-ubsan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer"
cmake --build build-ubsan -j
ctest --test-dir build-ubsan --output-on-failure
Verified status

UBSan profile passed in the CMake build matrix.

Claims boundary

This documents sanitizer-profile validation for local C++ test workflows.

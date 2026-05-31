
Warnings-as-Errors Report
Scope

Warnings-as-errors build profile for AccelSim-Lite.

Command
cmake -S . -B build-werror -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror"
cmake --build build-werror -j
ctest --test-dir build-werror --output-on-failure
Verified status

Warnings-as-errors profile passed in the CMake build matrix.

Claims boundary

This documents build hygiene validation for local C++ test workflows.

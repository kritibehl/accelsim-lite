# AccelSim-Lite build profile notes.
#
# This file documents supported local/CI profile names used by
# ci/run_build_matrix.sh.
#
# Profiles:
# - Debug
# - Release
# - ASan
# - UBSan
# - WarningsAsErrors
#
# Sanitizer profiles are configured through CMAKE_CXX_FLAGS from the
# matrix runner so the main CMakeLists.txt stays simple and portable.

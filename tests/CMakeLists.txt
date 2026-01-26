include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(accelsim_tests
  test_queue.cpp
  test_determinism.cpp
)

target_compile_definitions(accelsim_tests PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(accelsim_tests PRIVATE accelsim GTest::gtest_main)

add_test(NAME accelsim_tests COMMAND accelsim_tests)

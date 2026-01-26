cmake_minimum_required(VERSION 3.16)
project(accelsim_lite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(accelsim
  src/instruction.cpp
  src/trace_writer.cpp
  src/memory_controller.cpp
  src/compute_unit.cpp
  src/simulator.cpp
)
target_include_directories(accelsim PUBLIC include)

add_executable(accelsim_cli src/main.cpp)
target_link_libraries(accelsim_cli PRIVATE accelsim)

enable_testing()
add_subdirectory(tests)

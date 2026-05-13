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
  src/scheduler.cpp
)
target_include_directories(accelsim PUBLIC include)

###add_executable(accelsim_cli src/main.cpp)
###target_link_libraries(accelsim_cli PRIVATE accelsim)

enable_testing()
add_subdirectory(tests)

# ACCELSIM_LITE_STAGE2
add_library(accelsim_lite_core
  src/pipeline.cpp
  src/scheduler.cpp
  src/simulator.cpp
  src/workload_runner.cpp
)

target_include_directories(accelsim_lite_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(accelsim_lite_core PUBLIC cxx_std_17)

add_executable(accelsim-lite
  tools/accelsim_cli.cpp
)

target_link_libraries(accelsim-lite PRIVATE accelsim_lite_core)

set_target_properties(accelsim-lite PROPERTIES OUTPUT_NAME accelsim-lite)
# /ACCELSIM_LITE_STAGE2


add_executable(threaded_workload_runner
  parallel_execution/threaded_workload_runner.cpp
)

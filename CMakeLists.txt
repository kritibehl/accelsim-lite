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


add_library(firmware_validation
  firmware_style_validation/fixed_point_filter.cpp
  firmware_style_validation/ring_buffer.cpp
)

target_include_directories(firmware_validation PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/firmware_style_validation
)


add_library(trajectory_graph_lite
  trajectory_graph_lite/src/trajectory_graph.cpp
)

target_include_directories(trajectory_graph_lite PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/trajectory_graph_lite/include
)

add_executable(trajectory-graph
  trajectory_graph_lite/src/trajectory_graph_cli.cpp
)

target_link_libraries(trajectory-graph PRIVATE
  trajectory_graph_lite
)

add_library(storage_datapath
  storage_datapath/src/storage_datapath.cpp
)

target_include_directories(storage_datapath PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/storage_datapath/include
)

add_executable(storage-datapath
  storage_datapath/src/storage_datapath_cli.cpp
)

target_link_libraries(storage-datapath PRIVATE
  storage_datapath
)

add_library(performance_engine
  performance_engine/src/performance_engine.cpp
)

target_include_directories(performance_engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/performance_engine/include
)

find_package(Threads REQUIRED)

target_link_libraries(performance_engine PUBLIC
  Threads::Threads
)

add_executable(performance-benchmark
  performance_engine/src/performance_benchmark_cli.cpp
)

target_link_libraries(performance-benchmark PRIVATE
  performance_engine
)

add_executable(performance-profile-runner
  performance_engine/src/performance_profile_runner.cpp
)

target_link_libraries(performance-profile-runner PRIVATE
  performance_engine
)

add_library(pthread_queue_c STATIC
  benchmarks/pthreads_queue/src/pthread_queue.c
)

target_include_directories(pthread_queue_c PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/pthreads_queue/include
)

target_link_libraries(pthread_queue_c PUBLIC
  Threads::Threads
)

set_target_properties(
  pthread_queue_c
  PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
)

add_executable(pthread-queue-benchmark
  benchmarks/pthreads_queue/src/pthread_queue_benchmark.c
)

target_link_libraries(pthread-queue-benchmark PRIVATE
  pthread_queue_c
  Threads::Threads
  m
)

set_target_properties(
  pthread-queue-benchmark
  PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
)

add_executable(pthread-queue-test
  benchmarks/pthreads_queue/src/pthread_queue_test.c
)

target_link_libraries(pthread-queue-test PRIVATE
  pthread_queue_c
)

add_test(
  NAME pthread_queue_test
  COMMAND pthread-queue-test
)

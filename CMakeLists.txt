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

add_executable(workload_scheduler_sim
  scheduler_lab/workload_scheduler_sim.cpp
)

add_executable(stack_bytecode_interpreter
  execution_engine_lab/stack_bytecode_interpreter.cpp
)

add_executable(bytecode_optimizer
  execution_engine_lab/bytecode_optimizer.cpp
)

add_executable(tiered_execution_sim
  execution_engine_lab/tiered_execution_sim.cpp
)

add_executable(object_shape_cache_sim
  execution_engine_lab/object_shape_cache_sim.cpp
)

add_executable(wasm_style_stack_validator
  execution_engine_lab/wasm_style_stack_validator.cpp
)

add_executable(semantic_equivalence_harness
  execution_engine_lab/semantic_equivalence_harness.cpp
)

add_executable(runtime_equivalence_harness
  execution_engine_lab/runtime_equivalence_harness.cpp
)

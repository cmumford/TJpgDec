cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang)

project(tjpgdec LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-strict-aliasing")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-strict-aliasing")
endif()

if (WASM_FUZZING)
  add_definitions(-DWASM_FUZZING)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=fuzzer")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=fuzzer")
  set(WASM_ASAN 1)
  set(WASM_LLVM_CODE_COVERAGE 1)
endif()

if (WASM_LLVM_CODE_COVERAGE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
endif()

if (WASM_ASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

include_directories(
  "${PROJECT_SOURCE_DIR}/src"
  "."
)

add_library(tjpgdec "")
target_sources(tjpgdec
  PRIVATE
    "src/tjpgd.c"
    "src/tjpgd.h"
)

if (WIN32)
add_executable(tjpgd_vc
  "sample/tjpgd_vc/jdev.c"
  "src/tjpgd.h"
)
target_link_libraries(tjpgd_vc tjpgdec)
else (WIN32)
add_executable(simple
  "test/simple/simple.c"
  "src/tjpgd.h"
)
target_link_libraries(simple tjpgdec)
endif (WIN32)

add_executable(fuzzer
  "test/fuzz/fuzzer.cc"
  "src/tjpgd.h"
)
target_link_libraries(fuzzer
  tjpgdec
)

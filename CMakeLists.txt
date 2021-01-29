cmake_minimum_required(VERSION 3.9)

project(tjpgdec LANGUAGES C)

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED OFF)
  set(CMAKE_C_EXTENSIONS OFF)
endif(NOT CMAKE_C_STANDARD)

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

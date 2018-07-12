project(cbor)
cmake_minimum_required(VERSION 2.8)

set(FLAGS "-Wall -Wextra -pedantic -Wno-long-long")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(FLAGS "${FLAGS} -O3")
  set(CMAKE_EXE_LINKER_FLAGS "-s")
  add_definitions(-DRELEASE -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(FLAGS "${FLAGS} -O0 -g")
  add_definitions(-DDEBUG)
else()
  set(FLAGS "${FLAGS} -O2 -pg")
  add_definitions(-DRELEASE -DNDEBUG)
endif()

set(CMAKE_CXX_FLAGS "${FLAGS} -std=c++14")
set(CMAKE_C_FLAGS "${FLAGS} -std=c11")

include_directories(src)

add_library(cbor src/cbor.h src/cbor.c)

add_executable(cbor-write src/examples/cbor-write.cc)
target_link_libraries(cbor-write cbor)

add_executable(cbor-read src/examples/cbor-read.cc)
target_link_libraries(cbor-read cbor)

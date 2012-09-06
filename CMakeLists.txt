project(MDD)
cmake_minimum_required(VERSION 2.8)

add_subdirectory("gtest-1.6.0")

file(GLOB_RECURSE HEADERS "include" "*.h")
file(GLOB TEST_SOURCES "test/*.cpp")
file(GLOB SOURCES "src/*.cpp")
add_custom_target(mdd_headers SOURCES ${HEADERS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(CMAKE_CXX_FLAGS_ANALYSIS "${CMAKE_CXX_FLAGS} --analyze")

include_directories(
  include
  ${CMAKE_SOURCE_DIR}/gtest-1.6.0/include
  /windows/devel/ltsmin/include
  /windows/devel/ltsmin/src
)
add_executable(mdd_test
  ${TEST_SOURCES}
)
add_library(cppmdd
  ${SOURCES}
)
target_link_libraries(mdd_test gtest)

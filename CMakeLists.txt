project(MDD)
cmake_minimum_required(VERSION 2.8)

add_subdirectory("gtest-1.6.0")

file(GLOB_RECURSE HEADERS "include" "*.h")
file(GLOB TEST_SOURCES "test/*.cpp")
add_custom_target(mdd_headers SOURCES ${HEADERS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_ANALYSIS "${CMAKE_CXX_FLAGS} --analyze")

include_directories(
  include
  ${CMAKE_SOURCE_DIR}/gtest-1.6.0/include
)
add_executable(mdd_test
  ${TEST_SOURCES}
)
target_link_libraries(mdd_test gtest)

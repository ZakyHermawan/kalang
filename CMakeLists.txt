SET(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "C++ compiler" FORCE)
cmake_minimum_required(VERSION 3.5)
project(kalang)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

file(GLOB SOURCES src/*)

add_executable(${PROJECT_NAME} ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support core irreader native orcjit)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} ${llvm_libs})

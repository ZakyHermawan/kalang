cmake_minimum_required(VERSION 3.5)
project(kaleidoscope)

file(GLOB SOURCES src/*)

add_executable(compiler ${SOURCES})

target_include_directories(compiler
  PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

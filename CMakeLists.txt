cmake_minimum_required(VERSION 3.16)

project(markdown_tool C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcurl")

add_executable(mdTool ${CMAKE_SOURCE_DIR}/src/mdTool.c)
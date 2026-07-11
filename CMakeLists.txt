cmake_minimum_required(VERSION 3.10.0)
project(mchack)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_SOURCE_DIR}/compile_commands.json"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/release")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(MCHACK_LOG_PATH="/tmp/mchack.log")
add_compile_definitions(MCHACK_LIB_NAME="libcheat.so")
add_compile_definitions(MCHACK_PROCESS_NAME="java")

add_compile_options(-Wall -Wextra -Wpedantic)
add_subdirectory(src/libs/zydis)
add_subdirectory(src/injector)
add_subdirectory(src/cheat)
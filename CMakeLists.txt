cmake_minimum_required(VERSION 3.10.0)
project(mchack VERSION 0.1.0 LANGUAGES C)

# for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/release")

find_package(JNI REQUIRED)
add_subdirectory(injector)
add_library(mchack SHARED main.c)

target_include_directories(mchack PRIVATE ${JNI_INCLUDE_DIRS})

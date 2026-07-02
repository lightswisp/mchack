cmake_minimum_required(VERSION 3.10.0)
project(injector)

option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
add_subdirectory(zydis)
add_executable(injector injector.c)
target_link_libraries(injector PRIVATE "Zydis")


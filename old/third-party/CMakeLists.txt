cmake_minimum_required(VERSION 3.27)

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

set(SPIRV_REFLECT_STATIC_LIB ON)
add_subdirectory(SPIRV-Reflect)
add_subdirectory(rapidobj)

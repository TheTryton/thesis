cmake_minimum_required(VERSION 3.27)
project(thesis)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)

#add_subdirectory(compute)
add_subdirectory(render)
#add_subdirectory(thesis)

cmake_minimum_required(VERSION 3.27)
project(thesis)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)

add_subdirectory(third-party)

add_subdirectory(asio_tcp_proxy)
add_subdirectory(k_way_merge_sort)
add_subdirectory(core)
add_subdirectory(compute)
add_subdirectory(render)
#add_subdirectory(thesis)

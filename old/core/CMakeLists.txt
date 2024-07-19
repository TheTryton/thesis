cmake_minimum_required(VERSION 3.27)
project(thesis_core)

find_package(EnTT CONFIG REQUIRED)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" thesis_core_headers)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" thesis_core_sources)

add_executable(thesis_core ${thesis_core_headers} ${thesis_core_sources})

target_include_directories(thesis_core PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_libraries(thesis_core PUBLIC EnTT::EnTT)
set_target_properties(thesis_core
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
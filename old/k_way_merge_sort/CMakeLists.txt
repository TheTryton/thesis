cmake_minimum_required(VERSION 3.30)
project(k_way_merge_sort)

#set(OpenMP_RUNTIME_MSVC "OpenMP MSVC version" CACHE STRING "experimental")
find_package(OpenMP COMPONENTS CXX REQUIRED)
message("${OpenMP_VERSION}")

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" k_way_merge_sort_headers)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" k_way_merge_sort_sources)

add_executable(k_way_merge_sort ${k_way_merge_sort_headers} ${k_way_merge_sort_sources})
target_link_libraries(k_way_merge_sort OpenMP::OpenMP_CXX)
target_include_directories(k_way_merge_sort PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_compile_options(k_way_merge_sort PRIVATE "-openmp:llvm")
set_target_properties(k_way_merge_sort
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
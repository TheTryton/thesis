cmake_minimum_required(VERSION 3.27)
project(thesis_render)

find_package(Vulkan REQUIRED)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" thesis_render_headers)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" thesis_render_sources)
find_glsl_files("${CMAKE_CURRENT_SOURCE_DIR}/shaders" thesis_render_shaders)

copy_files_to_build("${thesis_render_shaders}")
add_executable(thesis_render ${thesis_render_headers} ${thesis_render_sources})

target_include_directories(thesis_render PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_libraries(thesis_render PRIVATE
        Vulkan::Vulkan
)
set_target_properties(thesis_render
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
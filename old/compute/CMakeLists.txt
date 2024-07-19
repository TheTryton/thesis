cmake_minimum_required(VERSION 3.27)
project(thesis_compute)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
find_package(Stb REQUIRED)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" thesis_compute_headers)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" thesis_compute_sources)
find_glsl_files("${CMAKE_CURRENT_SOURCE_DIR}/shaders" thesis_compute_shaders)

copy_files_to_build("${thesis_compute_shaders}")
add_library(thesis_compute SHARED ${thesis_compute_headers} ${thesis_compute_sources})

target_include_directories(thesis_compute PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        ${Stb_INCLUDE_DIR}
)
target_link_libraries(thesis_compute PRIVATE
        Vulkan::Vulkan
        OpenGL::GL
        OpenCL::OpenCL
)
set_target_properties(thesis_compute
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
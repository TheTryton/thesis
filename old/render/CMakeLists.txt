cmake_minimum_required(VERSION 3.27)
project(thesis_render)

find_package(Vulkan COMPONENTS glslc REQUIRED)
find_package(VulkanHeaders CONFIG)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(SailC++ CONFIG REQUIRED)
#find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(FFMPEG REQUIRED)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" thesis_render_headers)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" thesis_render_sources)

find_glsl_files("${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex" thesis_render_vertex_shaders)
find_glsl_files("${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment" thesis_render_fragment_shaders)
find_glsl_files("${CMAKE_CURRENT_SOURCE_DIR}/shaders/compute" thesis_render_compute_shaders)
find_files("${CMAKE_CURRENT_SOURCE_DIR}/resources" thesis_resources)

copy_files_to_build("${thesis_resources}")
add_executable(thesis_render ${thesis_render_headers} ${thesis_render_sources})

target_compile_vulkan_shaders(
        TARGET thesis_render
        STAGE vertex
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}
        SRCS ${thesis_render_vertex_shaders}
)
target_compile_vulkan_shaders(
        TARGET thesis_render
        STAGE fragment
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}
        SRCS ${thesis_render_fragment_shaders}
)
target_compile_vulkan_shaders(
        TARGET thesis_render
        STAGE compute
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}
        SRCS ${thesis_render_compute_shaders}
)
target_copy_data(
        TARGET thesis_render
        INPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data"
)
target_include_directories(thesis_render PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        ${Stb_INCLUDE_DIR}
        ${FFMPEG_INCLUDE_DIRS}
)
target_link_directories(thesis_render PRIVATE
        ${FFMPEG_LIBRARY_DIRS}
)
target_link_libraries(thesis_render PRIVATE
        Vulkan::Vulkan
        Vulkan::Headers
        GPUOpen::VulkanMemoryAllocator
        glfw
        SAIL::sail-c++
        #Qt6::Core
        #Qt6::Gui
        #Qt6::Widgets
        ${FFMPEG_LIBRARIES}
)
set_target_properties(thesis_render
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
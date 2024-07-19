cmake_minimum_required(VERSION 3.27)
project(thesis)

set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" thesis_headers)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" thesis_sources)
find_glsl_files("${CMAKE_CURRENT_SOURCE_DIR}/shaders" thesis_shaders)
find_ui_files("${CMAKE_CURRENT_SOURCE_DIR}/ui" thesis_uis)

copy_files_to_build("${thesis_shaders}")
qt5_wrap_ui(thesis_ui_headers ${thesis_uis})
message("${CMAKE_CURRENT_BINARY_DIR}")
add_executable(thesis ${thesis_headers} ${thesis_sources} ${thesis_ui_headers})

target_include_directories(thesis PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(thesis PRIVATE
        OpenGL::GL
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)
set_target_properties(thesis
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
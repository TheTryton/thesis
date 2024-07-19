#common functions used in project configuration/generation

function(find_c_files path found_files)

    file(GLOB_RECURSE found_files_internal RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${path}/*.h"
            "${path}/*.c"
    )

    set(${found_files} ${found_files_internal} PARENT_SCOPE)

endfunction()

function(find_cpp_files path found_files)

    file(GLOB_RECURSE found_files_internal RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${path}/*.hpp"
            "${path}/*.inl"
            "${path}/*.cpp"
    )

    set(${found_files} ${found_files_internal} PARENT_SCOPE)

endfunction()

function(find_ui_files path found_files)

    file(GLOB_RECURSE found_files_internal RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${path}/*.ui"
    )

    set(${found_files} ${found_files_internal} PARENT_SCOPE)

endfunction()

function(find_rc_files path found_files)

    file(GLOB_RECURSE found_files_internal RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${path}/*.qrc"
    )

    set(${found_files} ${found_files_internal} PARENT_SCOPE)

endfunction()

function(find_glsl_files path found_files)

    file(GLOB_RECURSE found_files_internal RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${path}/*.glsl"
            "${path}/*.vert"
            "${path}/*.frag"
    )

    set(${found_files} ${found_files_internal} PARENT_SCOPE)

endfunction()

function(find_c_and_cpp_files path found_files)

    find_c_files(${path} found_files_internal1)
    find_cpp_files(${path} found_files_internal2)

    set(${found_files} ${found_files_internal1} ${found_files_internal2} PARENT_SCOPE)

endfunction()

function(find_files path found_files)

    file(GLOB_RECURSE found_files_internal RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${path}/*"
    )

    set(${found_files} ${found_files_internal} PARENT_SCOPE)

endfunction()

function(copy_files_to_build files)

    foreach (file ${files})
        configure_file(${file} ${file} COPYONLY)
    endforeach ()

endfunction()

function(set_default_output_directories proj_name)

    set_target_properties(${proj_name} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/debug"
            LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/debug"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/debug"
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/release"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/release"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/release"
            ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/relwithdebinfo"
            LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/relwithdebinfo"
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/relwithdebinfo"
            ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin/minsizerel"
            LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin/minsizerel"
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin/minsizerel"
    )

endfunction()

function(target_compile_vulkan_shaders)
    cmake_parse_arguments(
            PARSED_ARG
            ""
            "TARGET;STAGE;OUTPUT"
            "SRCS"
            ${ARGN}
    )

    if(NOT PARSED_ARG_TARGET)
        message(FATAL_ERROR "target_compile_vulkan_shaders requires TARGET argument!")
    endif ()

    if(NOT PARSED_ARG_STAGE)
        message(FATAL_ERROR "target_compile_vulkan_shaders requires STAGE argument!")
    endif ()

    if(NOT PARSED_ARG_OUTPUT)
        message(FATAL_ERROR "target_compile_vulkan_shaders requires OUTPUT argument!")
    endif ()

    foreach (src_relative_path ${PARSED_ARG_SRCS})
        set(src_path "${CMAKE_CURRENT_SOURCE_DIR}/${src_relative_path}")

        get_filename_component(src_filename ${src_relative_path} NAME_WLE REALPATH)
        get_filename_component(src_directory ${src_relative_path} DIRECTORY REALPATH)

        set(src_relative_path_without_filename "${src_directory}/${src_filename}")
        set(compiled_src_path "${PARSED_ARG_OUTPUT}/${src_relative_path_without_filename}.spv")

        get_filename_component(compiled_src_directory ${compiled_src_path} DIRECTORY REALPATH)

        file(MAKE_DIRECTORY ${compiled_src_directory})

        add_custom_command(
            OUTPUT ${compiled_src_path}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} --target-env=vulkan -fshader-stage=${PARSED_ARG_STAGE} ${src_path} -o ${compiled_src_path}
            DEPENDS ${src_path}
            COMMENT "Compiling ${PARSED_ARG_STAGE} shader: ${src_path}"
        )

        list(APPEND compiled_src_paths ${compiled_src_path})
    endforeach ()

    set(custom_target_name "${PARSED_ARG_TARGET}_${PARSED_ARG_STAGE}_shaders")

    add_custom_target(${custom_target_name}
            DEPENDS ${compiled_src_paths}
            COMMENT "${PARSED_ARG_STAGE} shaders for target ${PARSED_ARG_TARGET}"
            VERBATIM
    )

    add_dependencies(${PARSED_ARG_TARGET} ${custom_target_name})

endfunction()

function(target_copy_data)
    cmake_parse_arguments(
            PARSED_ARG
            ""
            "TARGET;INPUT_DIRECTORY;OUTPUT_DIRECTORY"
            ""
            ${ARGN}
    )

    if(NOT PARSED_ARG_TARGET)
        message(FATAL_ERROR "target_compile_vulkan_shaders requires TARGET argument!")
    endif ()

    if(NOT PARSED_ARG_INPUT_DIRECTORY)
        message(FATAL_ERROR "target_compile_vulkan_shaders requires INPUT_DIRECTORY argument!")
    endif ()

    if(NOT PARSED_ARG_OUTPUT_DIRECTORY)
        message(FATAL_ERROR "target_compile_vulkan_shaders requires OUTPUT_DIRECTORY argument!")
    endif ()

    set(custom_target_name "${PARSED_ARG_TARGET}_copy_data")

    find_files(${PARSED_ARG_INPUT_DIRECTORY} data_files)

    add_custom_target(${custom_target_name}
            DEPENDS ${data_files}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${PARSED_ARG_INPUT_DIRECTORY} ${PARSED_ARG_OUTPUT_DIRECTORY}
            COMMENT "Copying data for target ${PARSED_ARG_TARGET}"
            VERBATIM
    )

    add_dependencies(${PARSED_ARG_TARGET} ${custom_target_name})

endfunction()
cmake_minimum_required(VERSION 3.27)
project(asio_tcp_proxy)

find_package(Boost CONFIG REQUIRED COMPONENTS asio)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" asio_tcp_proxy_headers)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" asio_tcp_proxy_sources)

add_executable(asio_tcp_proxy ${asio_tcp_proxy_headers} ${asio_tcp_proxy_sources})
target_include_directories(asio_tcp_proxy PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_libraries(asio_tcp_proxy PRIVATE
        Boost::asio
)
set_target_properties(asio_tcp_proxy
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
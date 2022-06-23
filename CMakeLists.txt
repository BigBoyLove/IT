cmake_minimum_required(VERSION 3.22)
project(metall_node_storage)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_FIND_LIBRARY_SUFFIXES .so)


find_package(Boost REQUIRED)

add_subdirectory(metall_node_storage)
add_executable(01_store_nodes 01_store_nodes.cpp)

target_link_libraries(01_store_nodes
        metall_node_storage::metall_node_storage
)

add_executable(02_load_nodes 02_load_nodes.cpp)

target_link_libraries(02_load_nodes
        metall_node_storage::metall_node_storage
        )


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

find_package(Boost REQUIRED)

include(FetchContent)
FetchContent_Declare(
        metall
        GIT_REPOSITORY https://github.com/LLNL/metall.git
        GIT_TAG v0.20
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(metall)

set(BUILD_SHARED_LIBS "TRUE" CACHE STRING "" FORCE)
FetchContent_Declare(
        RDF4CPP
        GIT_REPOSITORY "https://github.com/rdf4cpp/rdf4cpp"
        GIT_TAG v0.0.1
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(RDF4CPP)

add_library(metall_node_storage
        src/BNodeBackend.cpp
        src/IRIBackend.cpp
        src/LiteralBackend.cpp
        src/MetallNodeStorageBackend.cpp
        src/VariableBackend.cpp
        )


target_include_directories(metall_node_storage PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

target_link_libraries(metall_node_storage PUBLIC
        Metall::Metall
        Boost::headers
        rdf4cpp::rdf4cpp
        )

add_library(metall_node_storage::metall_node_storage ALIAS metall_node_storage)
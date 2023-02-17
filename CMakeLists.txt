cmake_minimum_required(VERSION 3.0.0)

set(PROJECT wg-monitor)
project(${PROJECT} VERSION 0.0.1)

# C++ variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OS defines used inside code
if (WIN32)
    add_compile_options(-DWINDOWS)
endif (WIN32)
if (UNIX)
    add_compile_options(-DLINUX)
endif (UNIX)

add_compile_options($<$<CONFIG:Debug>:-DDEBUG>$<$<CONFIG:Release>:-DRELEASE>)

set(
    SRC
    src/main.cpp
    src/Hook.cpp
    src/HTTPServer.cpp
    src/log.cpp
    src/Monitor.cpp
    src/tools.cpp
    src/wg-extension.cpp
)

add_executable(${PROJECT} ${SRC})

target_include_directories(
    ${PROJECT}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extlib/include
)

target_link_directories(
    ${PROJECT}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extlib/lib
)

target_link_libraries(
    ${PROJECT}
    PRIVATE fmt
    PRIVATE httplib
    PRIVATE spdlog
    PRIVATE wireguard
)

add_custom_target(
    dist
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dist.sh ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${PROJECT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
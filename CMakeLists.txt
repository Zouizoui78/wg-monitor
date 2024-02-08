cmake_minimum_required(VERSION 3.5)
project(wg-monitor VERSION 0.0.1)

set(
    SRC
    src/main.cpp
    src/Hook.cpp
    src/HTTPServer.cpp
    src/Monitor.cpp
    src/tools.cpp
    src/wg-extension.cpp
)

find_package(spdlog REQUIRED)

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-DDEBUG>$<$<CONFIG:Release>:-DRELEASE>)

target_include_directories(
    ${PROJECT}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/wireguard
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/cpp-httplib
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE spdlog::spdlog
    PRIVATE ssl
    PRIVATE crypto
)

add_custom_target(
    dist
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dist.sh ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${PROJECT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

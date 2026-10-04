cmake_minimum_required(VERSION 4.0)
project(carross LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(/utf-8)
file(WRITE
    ${CMAKE_BINARY_DIR}/carross.rc
    "101 ICON \"${CMAKE_SOURCE_DIR}/assets/icon.ico\"\n"
)

add_subdirectory(src)
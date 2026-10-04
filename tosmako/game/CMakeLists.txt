cmake_minimum_required(VERSION 3.16)

project(game LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_library(game STATIC
    coworkgame.cpp
    coworkgame.h
    pregame.h pregame.cpp
    pvpgame.h pvpgame.cpp
    singlegame.h singlegame.cpp
)

target_link_libraries(game PRIVATE Qt${QT_VERSION_MAJOR}::Widgets core)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(game PRIVATE TOMATO_GAME_LIBRARY)

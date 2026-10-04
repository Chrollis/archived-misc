cmake_minimum_required(VERSION 3.16)

project(core LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_library(core STATIC
    entity.cpp entity.h
    snake.h snake.cpp
    game.h game.cpp
    difficultydialog.h difficultydialog.cpp difficultydialog.ui
    scoredialog.h scoredialog.cpp scoredialog.ui
)

target_link_libraries(core PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(core PRIVATE TOMATO_CORE_LIBRARY)

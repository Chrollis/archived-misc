cmake_minimum_required(VERSION 3.16)

project(chrollis-tank VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

add_subdirectory(core)
add_subdirectory(upper)

option(BUILD_EDITOR "Build the map editor" OFF)
if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(chrollis-tank
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(chrollis-tank SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(chrollis-tank
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(chrollis-tank PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    core
    upper
)

add_custom_command(TARGET chrollis-tank POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:chrollis-tank>/assets"
    COMMENT "Copying assets/ next to the executable"
)

include(GNUInstallDirs)
install(TARGETS chrollis-tank
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(chrollis-tank)
endif()

if(APPLE)
    set(ASSETS_INSTALL_DIR "chrollis-tank.app/Contents/MacOS")
else()
    set(ASSETS_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
endif()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    DESTINATION "${ASSETS_INSTALL_DIR}/assets"
)

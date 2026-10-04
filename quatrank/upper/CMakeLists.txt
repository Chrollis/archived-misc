add_library(upper STATIC
    player.h player.cpp
    hostile.h hostile.cpp
    origame.h origame.cpp
)

target_include_directories(upper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(upper PUBLIC
    core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)
add_library(core STATIC
    utils.h utils.cpp
    myitem.h myitem.cpp
    atlas.h atlas.cpp
    entity.h entity.cpp
    bullet.h bullet.cpp
    oritank.h oritank.cpp
    loot.h loot.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)
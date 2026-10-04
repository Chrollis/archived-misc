if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(editor
        main.cpp
        editor.cpp
        editor.h
    )
else()
    add_executable(editor
        main.cpp
        editor.cpp
        editor.h
    )
endif()

target_link_libraries(editor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    core
    upper
)

add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets"
        "$<TARGET_FILE_DIR:editor>/assets"
    COMMENT "Copying assets/ next to the editor"
)
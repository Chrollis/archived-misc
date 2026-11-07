file(GLOB HEADERS *.hpp)
file(GLOB SOURCES *.cpp)

add_executable(chrollis-crosshair WIN32
    ${HEADERS} ${SOURCES}
    ${CMAKE_BINARY_DIR}/crosshair.rc
)

target_link_libraries(chrollis-crosshair PRIVATE
    comdlg32 user32 gdi32 gdiplus shell32
)
target_include_directories(chrollis-crosshair PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/json/include
)
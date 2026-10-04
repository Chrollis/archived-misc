file(GLOB HEADERS *.hpp)
file(GLOB SOURCES *.cpp)

add_executable(carross WIN32
    ${HEADERS} ${SOURCES}
    ${CMAKE_BINARY_DIR}/carross.rc
)

target_link_libraries(carross PRIVATE
    comdlg32 user32 gdi32 gdiplus shell32
)
target_include_directories(carross PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/json/include
)
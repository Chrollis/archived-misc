file(GLOB HEADERS *.hpp)
file(GLOB SOURCES *.cpp)

add_library(exp3 STATIC
    ${HEADERS} ${SOURCES}
)

target_include_directories(exp3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/third_party/json/include)
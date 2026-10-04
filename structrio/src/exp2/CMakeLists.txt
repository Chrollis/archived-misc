file(GLOB HEADERS *.hpp)
file(GLOB SOURCES *.cpp)

add_library(exp2 STATIC
    ${HEADERS} ${SOURCES}
)
target_include_directories(exp2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
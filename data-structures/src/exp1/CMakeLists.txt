file(GLOB HEADERS *.hpp)
file(GLOB SOURCES *.cpp)

add_library(exp1 STATIC
    ${HEADERS} ${SOURCES}
)
target_include_directories(exp1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
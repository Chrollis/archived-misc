add_subdirectory(exp1)
add_subdirectory(exp2)
add_subdirectory(exp3)

file(GLOB HEADERS *.hpp)
file(GLOB SOURCES *.cpp)

add_executable(structrio
    ${HEADERS} ${SOURCES}
)

target_include_directories(structrio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(structrio PRIVATE exp1 exp2 exp3)

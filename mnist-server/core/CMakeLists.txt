file(GLOB_RECURSE CORE_SOURCES "src/*.cpp")
file(GLOB_RECURSE CORE_HEADERS "include/*.h")

add_library(core STATIC ${CORE_HEADERS} ${CORE_SOURCES})

target_include_directories(core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/stb
        ${EIGEN_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/json/include
)

file(GLOB SOURCE_FILES "src/*.cpp")

add_library(core ${SOURCE_FILES})

target_include_directories(core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EASYX_INCLUDE}
)

target_link_libraries(core PUBLIC 
    ${CMAKE_SOURCE_DIR}/third_party/cxxopts/include   
    OpenMP::OpenMP_CXX
    ${EASYX_LIB}
)
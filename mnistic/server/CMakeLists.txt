file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp")
file(GLOB_RECURSE SERVER_HEADERS "include/*.h")

add_executable(server ${SERVER_SOURCES})

target_link_libraries(server
    PRIVATE
        core                           
        OpenMP::OpenMP_CXX        
)

target_include_directories(server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/third_party/httplib/include
)

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
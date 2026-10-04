add_library(upper STATIC
    le_net5.hpp le_net5.cpp
    vgg16.hpp vgg16.cpp
)

target_include_directories(upper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(upper
    PUBLIC
        core
        Qt6::Core
    PRIVATE
        Eigen3::Eigen
)

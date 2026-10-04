add_library(core STATIC
    cnn_base.h cnn_base.cpp
    convolve_layer.hpp convolve_layer.cpp
    pool_layer.hpp pool_layer.cpp
    filter.hpp filter.cpp
    full_connect_layer.hpp full_connect_layer.cpp
    mnist_data.hpp mnist_data.cpp
    image_process.hpp image_process.cpp
    language_manager.h language_manager.cpp
    activation_function.hpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core
    PUBLIC
        Qt::Core
        Qt::Widgets
        Eigen3::Eigen
        ${OpenCV_LIBS}
    PRIVATE
        OpenMP::OpenMP_CXX
)

cmake_minimum_required(VERSION 3.19)
project(numreco LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Concurrent)
find_package(Qt6 REQUIRED COMPONENTS Core)

find_package(OpenMP REQUIRED)

set(EIGEN3_INCLUDE_DIR "D:/Program/eigen-3.4.0")
add_library(Eigen3::Eigen INTERFACE IMPORTED)
set_target_properties(Eigen3::Eigen PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIR}"
)

find_package(OpenCV REQUIRED CONFIGS PATHS "D:/Program/opencv-4.5.5-mingw/x64/mingw/lib")
include_directories(${OpenCV_INCLUDE_DIRS})

qt_standard_project_setup()

# Build the model core and the upper-level models as static libraries.
add_subdirectory(core)
add_subdirectory(upper)

qt_add_executable(numreco
    WIN32 MACOSX_BUNDLE
    main.cpp
    # Root directory holds only the UI layer.
    mainwindow.h mainwindow.cpp mainwindow.ui
)

target_link_libraries(numreco
    PRIVATE
        core
        upper
        Qt::Core
        Qt::Widgets
        Qt::Concurrent
        OpenMP::OpenMP_CXX
        Eigen3::Eigen
        ${OpenCV_LIBS}
)
target_link_libraries(numreco PRIVATE Qt6::Core)

include(GNUInstallDirs)

install(TARGETS numreco
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET numreco
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# Copy the i18n/ language resources next to the executable
# (the application reads them from ./i18n/ at runtime).
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/i18n/
    DESTINATION ${CMAKE_INSTALL_BINDIR}/i18n
)

# Copy the OpenCV runtime DLLs next to the executable.
# The MinGW import libraries (*.dll.a) live in lib/, while the
# runtime DLLs needed by the application are located in bin/.
set(OPENCV_MINGW_ROOT "D:/Program/opencv-4.5.5-mingw/x64/mingw")
file(GLOB OPENCV_RUNTIME_DLLS
    "${OPENCV_MINGW_ROOT}/bin/*.dll"
)

install(FILES ${OPENCV_RUNTIME_DLLS}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

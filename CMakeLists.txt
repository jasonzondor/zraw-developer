cmake_minimum_required(VERSION 3.20)
project(zraw-developer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

# Find LibRaw
pkg_check_modules(LIBRAW REQUIRED libraw)
include_directories(${LIBRAW_INCLUDE_DIRS})

# Find libtiff
pkg_check_modules(LIBTIFF REQUIRED libtiff-4)
include_directories(${LIBTIFF_INCLUDE_DIRS})

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${LIBRAW_INCLUDE_DIRS}
    ${LIBTIFF_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/RawProcessor.cpp
    src/core/ImageBuffer.cpp
    src/core/CLIHandler.cpp
    src/core/ImageExporter.cpp
    src/core/XMPHandler.cpp
    src/gpu/GLContext.cpp
    src/gpu/ShaderProgram.cpp
    src/gpu/GPUPipeline.cpp
    src/adjustments/ExposureAdjustment.cpp
    src/adjustments/ContrastAdjustment.cpp
    src/adjustments/SharpnessAdjustment.cpp
    src/ui/MainWindow.cpp
    src/ui/ImageViewer.cpp
    src/ui/AdjustmentPanel.cpp
    src/ui/ResettableSlider.cpp
)

set(HEADERS
    src/core/RawProcessor.h
    src/core/ImageBuffer.h
    src/core/CLIHandler.h
    src/core/ImageExporter.h
    src/core/XMPHandler.h
    src/gpu/GLContext.h
    src/gpu/ShaderProgram.h
    src/gpu/GPUPipeline.h
    src/adjustments/ExposureAdjustment.h
    src/adjustments/ContrastAdjustment.h
    src/adjustments/SharpnessAdjustment.h
    src/ui/MainWindow.h
    src/ui/ImageViewer.h
    src/ui/AdjustmentPanel.h
    src/ui/ResettableSlider.h
)

# Create executable
add_executable(zraw-developer ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(zraw-developer
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
    ${LIBRAW_LIBRARIES}
    ${LIBTIFF_LIBRARIES}
)

# Compiler flags
target_compile_options(zraw-developer PRIVATE
    -Wall
    -Wextra
    -O3
    -march=native
)

# Install target
install(TARGETS zraw-developer DESTINATION bin)

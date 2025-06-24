cmake_minimum_required(VERSION 3.16)

project(MetroOptimization VERSION 0.1 LANGUAGES CXX) # Your project is named MetroOptimization

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Define your source and header files
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        metrosystem.cpp
)

set(PROJECT_HEADERS
        mainwindow.h
        metrosystem.h
)

# ---- DELETE THIS BLOCK ----
# target_sources(MetroFlowFinal   <-- This was the problem
#  PRIVATE
#    metrosystem.h
#    metrosystem.cpp
# )
# -------------------------

# Create the executable using the project name (MetroOptimization)
# and the already defined source/header variables
qt_add_executable(${PROJECT_NAME} # This will use "MetroOptimization"
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

# Link against the required Qt6 modules
target_link_libraries(${PROJECT_NAME} PRIVATE # This will use "MetroOptimization"
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Copy metroFinalData.csv to the build directory
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metroFinalData.csv"
    "${CMAKE_CURRENT_BINARY_DIR}/metroFinalData.csv"
    COPYONLY
)

# Copy map_generator.py to the build directory
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/map_generator.py"
    "${CMAKE_CURRENT_BINARY_DIR}/map_generator.py"
    COPYONLY
)

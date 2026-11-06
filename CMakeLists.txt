cmake_minimum_required(VERSION 3.21)
project(GDFreeNONGs VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK is not set. Install Geode v6.0.0-alpha.1 SDK and point GEODE_SDK to it.")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/hooks/MusicDownloadManager.cpp
)

setup_geode_mod(${PROJECT_NAME})

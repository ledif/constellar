cmake_minimum_required(VERSION 4.3)
project(
    constellar
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE
        "RelWithDebInfo"
        CACHE STRING "Build type" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core DBus Network Test Widgets)

add_compile_definitions(QT_NO_KEYWORDS)

enable_testing()

add_subdirectory(src/common)
add_subdirectory(src/daemon)
add_subdirectory(src/cli)
add_subdirectory(src/gui)
add_subdirectory(tests)

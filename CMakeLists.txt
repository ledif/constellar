cmake_minimum_required(VERSION 4.3)
project(
    constellar
    VERSION 0.2.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
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

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(KDEInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core DBus Network Test Widgets Quick QuickControls2)
include(ECMFindQmlModule)
include(ECMQmlModule)
find_package(KF6 REQUIRED COMPONENTS Kirigami I18n CoreAddons QQC2DesktopStyle IconThemes)

add_compile_definitions(QT_NO_KEYWORDS)

enable_testing()

include(SpecDoc)

add_subdirectory(src/common)
add_subdirectory(src/client)
add_subdirectory(src/daemon)
add_subdirectory(src/cli)
add_subdirectory(src/gui)
add_subdirectory(tests)

project(kraft)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/ ${CMAKE_MODULE_PATH} )

find_package(KDE4 4.4.0 REQUIRED) 
find_package(KdepimLibs REQUIRED)
find_package(Ctemplate REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include(KDE4Defaults)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} src)

add_subdirectory(src)
add_subdirectory(database)
add_subdirectory(reports)
add_subdirectory(importfilter)
add_subdirectory(tools)
add_subdirectory(styles)



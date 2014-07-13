project(kraft)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/ ${CMAKE_MODULE_PATH} )

find_package(Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Ctemplate REQUIRED)

add_definitions(${QT_DEFINITIONS} )

include_directories(${QT_INCLUDES} src)

add_subdirectory(src)
add_subdirectory(database)
add_subdirectory(reports)
add_subdirectory(importfilter)
add_subdirectory(tools)
add_subdirectory(styles)



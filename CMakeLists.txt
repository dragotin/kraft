PROJECT(kraft)
find_package(KDE4 REQUIRED)
find_package (KdepimLibs)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include(KDE4Defaults)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} src)

add_subdirectory(src)
add_subdirectory(database)
add_subdirectory(reports)
add_subdirectory(importfilter)
add_subdirectory(tools)


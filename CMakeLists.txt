project(kraft)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(ECMInstallIcons)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

# TODO: Build without deprecated QUrl methods
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

# QCA still uses slots/signals keywords
# remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

find_package(Qt5 REQUIRED
    Core
    Concurrent
    DBus
    Network
    Widgets
    Sql
)

find_package(KF5 REQUIRED

    CoreAddons
    DocTools
    I18n
    Emoticons
    GlobalAccel
    GuiAddons
    KIO
    NotifyConfig
    Sonnet
    TextWidgets
    Wallet
    WidgetsAddons
    XmlGui
)
find_package(Ctemplate REQUIRED)

#option(QTINDICATE_DISABLE "Disable support for notifications via indicator")

add_definitions(${QT_DEFINITIONS} )

include_directories(${QT_INCLUDES} src)

add_subdirectory(src)
add_subdirectory(database)
add_subdirectory(reports)
add_subdirectory(importfilter)
add_subdirectory(tools)
add_subdirectory(styles)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

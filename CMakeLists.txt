project(kraft)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_AUTOMOC TRUE)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

# TODO: Build without deprecated QUrl methods
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

# remove override later, this is for a bug in kxmlguiwindow.h which
# does not implement the member of QTabWidget hasHeightForWidth
ADD_DEFINITIONS(-Doverride=)

# QCA still uses slots/signals keywords
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(KF5AkonadiContact)

find_package(Qt5 CONFIG REQUIRED
    Core
    Concurrent
    DBus
    Network
    Widgets
    Sql
    WebEngine
    WebEngineWidgets
)

find_package(KF5 REQUIRED COMPONENTS
    Akonadi
    Contacts
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
include_directories( /usr/include/KF5/AkonadiCore )

add_subdirectory(src)
add_subdirectory(database)
add_subdirectory(reports)
add_subdirectory(importfilter)
add_subdirectory(tools)
add_subdirectory(styles)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

project(kraft)

cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0063 NEW)

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_AUTOMOC TRUE)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

# Uncomment to enable some tweaks for AppImage build
# add_definitions(-DBUILD_APPIMAGE=1)

kde_enable_exceptions()
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5 CONFIG REQUIRED
    Core
    Gui
    Sql
    Widgets
    Xml
    Svg
)

# XmlGui for the ConfigSkeleton
find_package(KF5 REQUIRED COMPONENTS
    I18n
    Config
    Contacts
)

# Grantlee templating engine
find_package(Grantlee5 REQUIRED)
set_package_properties(Grantlee5 PROPERTIES
    DESCRIPTION "Library for templating html and pdf output"
    URL "https://www.grantlee.org/"
    PURPOSE "Optionally used for templating"
    TYPE OPTIONAL
)

find_package(KF5Akonadi)
set_package_properties(KF5Akonadi PROPERTIES
    DESCRIPTION "Library for general Access to Akonadi"
    URL "https://www.kde.org/"
    PURPOSE "Optionally used for addressbook integration"
    TYPE OPTIONAL
)

find_package(KF5AkonadiContact)
set_package_properties(KF5AkonadiContact PROPERTIES
    DESCRIPTION "Library for Accessing Contacts stored in Akonadi"
    URL "https://www.kde.org/"
    PURPOSE "Optionally used for addressbook integration"
    TYPE OPTIONAL
)

if(KF5Akonadi_FOUND AND KF5AkonadiContact_FOUND)
   add_definitions(-DHAVE_AKONADI)
endif()

find_package(Ctemplate REQUIRED)

find_package(Asciidoctor)
# Sets the variable ASCIIDOCTOR_FOUND

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-override")
# disable the warning about null-pointer zero. FIXME.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-as-null-pointer-constant")

set(QT_DEFINITIONS "${Qt5Core_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS}")
add_definitions(${QT_DEFINITIONS} )

include_directories(${QT_INCLUDES} src)
include_directories( /usr/include/KF5/AkonadiCore )

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
        ki18n_install(po)
endif()

add_subdirectory(src)
add_subdirectory(database)
add_subdirectory(reports)
add_subdirectory(views)
add_subdirectory(importfilter)
add_subdirectory(tools)
add_subdirectory(styles)
add_subdirectory(meta)
add_subdirectory(tests)
add_subdirectory(manual)


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

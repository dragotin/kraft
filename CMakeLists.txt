cmake_minimum_required(VERSION 3.16.0)
project(kraft)
cmake_policy(SET CMP0063 NEW)

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_AUTOMOC TRUE)

option(AKONADI_LEGACY_BUILD "Build with older versions of Akonadi (KF5 Prefix)" OFF)
# Akonadi Prefix: Set this to KF5 for builds with Akonadi Libs before 5.23
set(AKO_PREFIX "KPim5")

if (AKONADI_LEGACY_BUILD)
  set(AKO_PREFIX "KF5")
endif()
message("Akonadi Prefix is ${AKO_PREFIX}")

include(GetGitRevisionDescription)

# set git revision info
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
# if we cannot get it from git, directly try .tag (packages)
# this will work if the tar balls have been properly created
# via git-archive.
if ("${GIT_SHA1}" STREQUAL "GITDIR-NOTFOUND")
    if (EXISTS "${CMAKE_SOURCE_DIR}/.tag")
        file(READ ${CMAKE_SOURCE_DIR}/.tag sha1_candidate)
        string(REPLACE "\n" "" sha1_candidate ${sha1_candidate})
        if (NOT ${sha1_candidate} STREQUAL "$Format:%H$")
            message("${sha1_candidate}")
            set (GIT_SHA1 "${sha1_candidate}")
        endif()
    else()
        set (GIT_SHA1 "unknown")
    endif()
endif()

message(STATUS "Git dynamic information")
message("GIT_SHA1: ${GIT_SHA1}")

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("GIT_BRANCH: ${GIT_BRANCH}")

if($ENV{SOURCE_DATE_EPOCH})
    set(BUILD_HOST_NAME "reproduciblebuild")
else ()
    cmake_host_system_information(RESULT BUILD_HOST_NAME QUERY HOSTNAME)
endif()
if(${CMAKE_VERSION} VERSION_GREATER "3.22.0")
    cmake_host_system_information(RESULT BUILD_HOST_DISTRI QUERY DISTRIB_PRETTY_NAME)
else ()
    set(BUILD_HOST_DISTRI "unknown distribution")
endif()
message("Build host name: ${BUILD_HOST_NAME}")
message("Build host distribution: ${BUILD_HOST_DISTRI}")

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

find_package(${AKO_PREFIX}Akonadi)
set_package_properties(${AKO_PREFIX}Akonadi PROPERTIES
    DESCRIPTION "Library for general Access to Akonadi"
    URL "https://www.kde.org/"
    PURPOSE "Optionally used for addressbook integration"
    TYPE OPTIONAL
)

find_package(${AKO_PREFIX}AkonadiContact)
set_package_properties(${AKO_PREFIX}AkonadiContact PROPERTIES
    DESCRIPTION "Library for Accessing Contacts stored in Akonadi"
    URL "https://www.kde.org/"
    PURPOSE "Optionally used for addressbook integration"
    TYPE OPTIONAL
)

find_package(${AKO_PREFIX}ContactEditor)
set_package_properties(${AKO_PREFIX}ContactEditor PROPERTIES
    DESCRIPTION "Library for editing contacts stored in Akonadi"
    URL "https://www.kde.org/"
    PURPOSE "Optionally used for addressbook integration"
    TYPE OPTIONAL
)

if(${AKO_PREFIX}Akonadi_FOUND AND ${AKO_PREFIX}AkonadiContact_FOUND AND ${AKO_PREFIX}ContactEditor_FOUND)
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
include_directories( /usr/include/${AKO_PREFIX}/AkonadiCore )
include_directories(/usr/include/${AKO_PREFIX}/)

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

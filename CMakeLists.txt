cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(kraft LANGUAGES CXX)
cmake_policy(SET CMP0063 NEW)

FIND_PACKAGE(ECM 6.0.0 REQUIRED NO_MODULE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
SET(KF_IGNORE_PLATFORM_CHECK TRUE)
set(QT_MIN_VERSION 6.2.4)

set(CMAKE_AUTOMOC TRUE)

set(AKO_PREFIX "KPim6")

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

include(ExternalProject)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(KDEInstallDirs6)
include(ECMQtDeclareLoggingCategory)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)

kde_enable_exceptions()
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Xml Sql Svg SvgWidgets Network Concurrent Test)

qt_standard_project_setup()

find_package(KF6 REQUIRED COMPONENTS
    I18n
    Config
    Contacts
    TextTemplate
)

find_package(${AKO_PREFIX}Akonadi)
find_package(${AKO_PREFIX}AkonadiContactCore)
find_package(${AKO_PREFIX}AkonadiContactWidgets)

if( ${AKO_PREFIX}AkonadiContactCore_FOUND AND ${AKO_PREFIX}AkonadiContactWidgets_FOUND )
    message ("## Defined Akonadi FOUND")
    add_definitions(-DHAVE_AKONADI)
endif()

find_package(Ctemplate REQUIRED)

find_package(Asciidoctor)
# Sets the variable ASCIIDOCTOR_FOUND

# disable the warning about null-pointer zero. FIXME.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-as-null-pointer-constant")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-override")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs")

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

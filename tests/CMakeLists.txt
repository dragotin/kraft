cmake_minimum_required(VERSION 2.8.11)
project(kraft)

cmake_policy(SET CMP0063 NEW)

enable_testing()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( "${CMAKE_CURRENT_BINARY_DIR}/../src" )

find_package(Qt5Test REQUIRED)

set(unitman_src
    ../src/unitmanager.cpp
    ../src/kraftdb.cpp
    ../src/einheit.cpp
    ../src/metaxmlparser.cpp
    ../src/doctype.cpp
    ../src/attribute.cpp
    ../src/numbercycle.cpp
)

add_executable(t_unitman t_unitman.cpp ${unitman_src})
add_test(t_unitman t_unitman)
target_link_libraries(t_unitman Qt5::Test Qt5::Core Qt5::Sql Qt5::Xml)

# ============================================================ 

set(format_src
    ../src/format.cpp
)
add_executable(t_format t_format.cpp ${format_src})
add_test(t_format t_format)
target_link_libraries(t_format Qt5::Test Qt5::Core) 

# ============================================================ 

set(metaparser_src
    ../src/metaxmlparser.cpp
)
add_executable(t_metaparser t_metaparser.cpp ${metaparser_src})
add_test(t_metaparser t_metaparser)

target_link_libraries(t_metaparser Qt5::Test Qt5::Xml)

# ============================================================ 

set(doctype_src
    ../src/doctype.cpp
    ../src/attribute.cpp
    ../src/numbercycle.cpp
    ../src/kraftdb.cpp
    ../src/metaxmlparser.cpp
)

add_executable(t_doctype t_doctype.cpp ${doctype_src})
add_test(t_doctype t_doctype)

target_link_libraries(t_doctype Qt5::Test Qt5::Sql Qt5::Widgets Qt5::Xml)



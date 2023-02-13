cmake_minimum_required(VERSION 2.8.11)
project(kraft)

cmake_policy(SET CMP0063 NEW)

enable_testing()

# Tell CMake to run moc when necessary:
# set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
# set(CMAKE_INCLUDE_CURRENT_DIR ON)

# include_directories( "${CMAKE_CURRENT_BINARY_DIR}/../src" )

find_package(Qt5Test REQUIRED)

set(test_libs Qt5::Test kraftlib)

add_executable(t_unitman t_unitman.cpp)
target_link_libraries(t_unitman ${test_libs})
add_test(t_unitman t_unitman)

# ============================================================

add_executable(t_format t_format.cpp)
add_test(t_format t_format)
target_link_libraries(t_format ${test_libs})

# ============================================================

add_executable(t_metaparser t_metaparser.cpp)
add_test(t_metaparser t_metaparser)

target_link_libraries(t_metaparser ${test_libs})

# ============================================================

add_executable(t_defaultprovider t_defaultprovider.cpp)
add_test(t_defaultprovider t_defaultprovider)

target_link_libraries(t_defaultprovider ${test_libs})

# ============================================================ 

add_executable(t_doctype t_doctype.cpp)
add_test(t_doctype t_doctype)

target_link_libraries(t_doctype ${test_libs})

# ============================================================

add_executable(t_attributes t_attributes.cpp)
add_test(t_attributes t_attributes)

target_link_libraries(t_attributes ${test_libs})

# ============================================================

add_executable(t_stringutil t_stringutil.cpp)
add_test(t_stringutil t_stringutil)

target_link_libraries(t_stringutil ${test_libs})

# ============================================================

add_executable(t_kraftdoc t_kraftdoc.cpp)
add_test(t_kraftdoc t_kraftdoc)

target_link_libraries(t_kraftdoc ${test_libs})

# ============================================================


set(TESTS_PATH ${CMAKE_CURRENT_SOURCE_DIR})
configure_file ("testconfig.h.in" "testconfig.h")

# ============================================================

add_executable(t_xmlsaver t_xmlsaver.cpp)
add_test(t_xmlsaver t_xmlsaver)

target_link_libraries(t_xmlsaver ${test_libs})

# ============================================================

add_executable(t_kraftattrib t_kraftattrib.cpp)
add_test(t_kraftattrib t_kraftattrib)

target_link_libraries(t_kraftattrib ${test_libs})


set(TESTS_PATH ${CMAKE_CURRENT_SOURCE_DIR})
configure_file ("testconfig.h.in" "testconfig.h")


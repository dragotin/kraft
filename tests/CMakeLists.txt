enable_testing()

# Tell CMake to run moc when necessary:
# set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
# set(CMAKE_INCLUDE_CURRENT_DIR ON)

# include_directories( "${CMAKE_CURRENT_BINARY_DIR}/../src" )

set(test_libs Qt6::Test kraftlib)

add_executable(t_unitman t_unitman.cpp)
target_link_libraries(t_unitman ${test_libs})
add_test(NAME t_unitman COMMAND t_unitman)

# ============================================================

add_executable(t_format t_format.cpp)
add_test(NAME t_format COMMAND t_format)
target_link_libraries(t_format ${test_libs})

# ============================================================

add_executable(t_metaparser t_metaparser.cpp)
add_test(NAME t_metaparser COMMAND t_metaparser)

target_link_libraries(t_metaparser ${test_libs})

# ============================================================

add_executable(t_defaultprovider t_defaultprovider.cpp)
add_test(NAME t_defaultprovider COMMAND t_defaultprovider)

target_link_libraries(t_defaultprovider ${test_libs})

# ============================================================ 

add_executable(t_doctype t_doctype.cpp)
add_test(NAME t_doctype COMMAND t_doctype)

target_link_libraries(t_doctype ${test_libs})

# ============================================================

add_executable(t_numbercycle t_numbercycle.cpp)
add_test(NAME t_numbercycle COMMAND t_numbercycle)

target_link_libraries(t_numbercycle ${test_libs})

# ============================================================

add_executable(t_attributes t_attributes.cpp)
add_test(NAME t_attributes COMMAND t_attributes)

target_link_libraries(t_attributes ${test_libs})

# ============================================================

add_executable(t_stringutil t_stringutil.cpp)
add_test(NAME t_stringutil COMMAND t_stringutil)

target_link_libraries(t_stringutil ${test_libs})

# ============================================================

add_executable(t_kraftdoc t_kraftdoc.cpp)
add_test(NAME t_kraftdoc COMMAND t_kraftdoc)

target_link_libraries(t_kraftdoc ${test_libs})

# ============================================================

add_executable(t_docposition t_docposition.cpp)
add_test(NAME t_docposition COMMAND t_docposition)

target_link_libraries(t_docposition ${test_libs})

# ============================================================


set(TESTS_PATH ${CMAKE_CURRENT_SOURCE_DIR})
configure_file ("testconfig.h.in" "testconfig.h")

# ============================================================

add_executable(t_xmlsaver t_xmlsaver.cpp)
add_test(NAME t_xmlsaver COMMAND t_xmlsaver)

target_link_libraries(t_xmlsaver ${test_libs})

# ============================================================

add_executable(t_xmldocindex t_xmldocindex.cpp)
add_test(NAME t_xmldocindex COMMAND t_xmldocindex)

target_link_libraries(t_xmldocindex ${test_libs})

# ============================================================

add_executable(t_kraftattrib t_kraftattrib.cpp)
add_test(NAME t_kraftattrib COMMAND t_kraftattrib)

target_link_libraries(t_kraftattrib ${test_libs})

# ============================================================

add_executable(t_jsonindexfile t_jsonindexfile.cpp)
add_test(NAME t_jsonindexfile COMMAND t_jsonindexfile)

target_link_libraries(t_jsonindexfile ${test_libs})


set(TESTS_PATH ${CMAKE_CURRENT_SOURCE_DIR})
configure_file ("testconfig.h.in" "testconfig.h")


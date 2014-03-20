
set(findcontact_NAME findcontact)
set(FINDCONTACT_SRC findcontact.cpp)

set(AUTOMOC ON)

kde4_add_executable(${findcontact_NAME} ${FINDCONTACT_SRC})

target_link_libraries(${findcontact_NAME}
        ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_AKONADI_CONTACT_LIBS} )

########### install files ###############
install(TARGETS ${findcontact_NAME} ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES erml2pdf.py  DESTINATION ${DATA_INSTALL_DIR}/kraft/tools )

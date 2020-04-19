
set(findcontact_NAME findcontact)
set(FINDCONTACT_SRC findcontact.cpp ../src/addressprovider.cpp ../src/addressprovider_akonadi.cpp)

set(AUTOMOC ON)

#
# For now there is only the Akonadi based address backend, and thus
# the findcontact tool is only built if akonadi is there.
# If there are other backends, this must be FIXED.
if(KF5Akonadi_FOUND)
    add_executable(${findcontact_NAME} ${FINDCONTACT_SRC})

    target_link_libraries( ${findcontact_NAME}
        Qt5::Core
        Qt5::Widgets
        KF5::Contacts
        KF5::AkonadiCore
        KF5::AkonadiContact
    )

    ########### install files ###############
    install(TARGETS ${findcontact_NAME} ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(FILES erml2pdf.py watermarkpdf.py  DESTINATION ${DATA_INSTALL_DIR}/kraft/tools )


set(findcontact_NAME findcontact)
set(FINDCONTACT_SRC findcontact.cpp
    ../src/addressprovider.cpp
    ../src/addressproviderakonadi.cpp
    ../src/addressproviderlocal.cpp
    ../src/addressproviderprivate.cpp
    ../src/models/contactsdirmodel.cpp)

set(AUTOMOC ON)

include_directories("../src")

# For now there is only the Akonadi based address backend, and thus
# the findcontact tool is only built if akonadi is there.
# If there are other backends, this must be FIXED.
if(${AKO_PREFIX}Akonadi_FOUND)
    add_executable(${findcontact_NAME} ${FINDCONTACT_SRC})

    target_link_libraries( ${findcontact_NAME}
        Qt6::Core
        Qt6::Xml
        Qt6::Widgets
        KF6::Contacts
        ${AKO_PREFIX}::AkonadiCore
        ${AKO_PREFIX}::AkonadiContactCore
    )

    ########### install files ###############
    install(TARGETS ${findcontact_NAME} ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(FILES watermarkpdf.py  DESTINATION ${KDE_INSTALL_DATADIR}/kraft/tools )


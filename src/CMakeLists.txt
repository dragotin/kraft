
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory(pics)

########### next target ###############

set( kraftcat_LIB_SRCS
    catalogtemplate.cpp
    katalog.cpp
    katalogman.cpp
    brunskatalog.cpp
    brunsrecord.cpp
    defaultprovider.cpp
    doctext.cpp
    doctype.cpp
    kataloglistview.cpp
    katalogview.cpp
    brunskataloglistview.cpp
    brunskatalogview.cpp
    filterheader.cpp
    catalogchapteredit.cpp
    kraftdb.cpp
    geld.cpp
    einheit.cpp
    attribute.cpp
    numbercycle.cpp
    setupassistant.cpp
    )


kde4_add_ui_files(kraftcat_LIB_SRCS calctemplate.ui dbinit.ui
createdb.ui
dbselect.ui
mysqldetails.ui
sqlitedetails.ui
statuspage.ui
upgradedb.ui )

kde4_add_kcfg_files(kraftcat_LIB_SRCS databasesettings.kcfgc kraftsettings.kcfgc)

kde4_add_library(kraftcat SHARED ${kraftcat_LIB_SRCS})

target_link_libraries(kraftcat ${KDE4_KDECORE_LIBS}  ${KDE4_KIO_LIBS} ${QT_QTSQL_LIBRARY} ${KDE4_KDEUI_LIBS})

set_target_properties(kraftcat PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS kraftcat ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

LINK_DIRECTORIES( lib )
set(kplant_SRCS brunsviewmain.cpp brunsviewer.cpp)

kde4_add_executable(kplant ${kplant_SRCS} )

target_link_libraries(kplant kraftcat ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kplant ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(kraft_SRCS
    templkatalog.cpp
    docposition.cpp
    matkatalog.cpp
    zeitcalcpart.cpp
    fixcalcpart.cpp
    floskeltemplate.cpp
    floskel.cpp
    calcpart.cpp
    kraftview.cpp
    kraftdoc.cpp
    main.cpp
    unitmanager.cpp
    stockmaterial.cpp
    stockmaterialman.cpp
    materialcalcpart.cpp
    flostempldialog.cpp
    stdsatzman.cpp
    materialtempldialog.cpp
    zeitcalcdialog.cpp
    fixcalcdialog.cpp
    matcalcdialog.h
    matcalcdialog.cpp
    templatesaverbase.cpp
    templatesaverdb.cpp
    materialsaverbase.cpp
    materialsaverdb.cpp
    prefsdialog.cpp
    prefswages.cpp
    templkatalogview.cpp
    templkataloglistview.cpp
    positionviewwidget.cpp
    documentsaverbase.cpp
    documentsaverdb.cpp
    documentman.cpp
    docdigest.cpp
    docdigestview.cpp
    doctypeedit.cpp
    portal.cpp
    portalview.cpp
    archiveman.cpp
    reportgenerator.cpp
    htmlview.cpp
    docpostcard.cpp
    catalogselection.cpp
    addressselection.cpp
    kraftdocheaderedit.cpp
    kraftdocfooteredit.cpp
    inserttempldialog.cpp
    archdocposition.cpp
    archdoc.cpp
    materialkataloglistview.cpp
    materialkatalogview.cpp
    materialselectdialog.cpp
    kraftdocedit.cpp
    kraftdocpositionsedit.cpp
    portalhtmlview.cpp
    templtopositiondialogbase.cpp
    insertplantdialog.cpp
    headerselection.cpp
    textselection.cpp
    newdocassistant.cpp
    docassistant.cpp
    texteditdialog.cpp
    templateprovider.cpp
    headertemplateprovider.cpp
    footertemplateprovider.cpp
    catalogtemplateprovider.cpp
    addresstemplateprovider.cpp
    texttemplate.cpp
    doclocaledialog.cpp
    tagman.cpp
    positiontagdialog.cpp
    tagtemplatesdialog.cpp
    importitemdialog.cpp
    importfilter.cpp
    kraftview_ro.cpp
    taxeditdialog.cpp
    numbercycledialog.cpp
    impviewwidgets.cpp

)

kde4_add_kcfg_files(kraft_SRCS databasesettings.kcfgc kraftsettings.kcfgc)

kde4_add_ui_files(kraft_SRCS
    zeitpartui.ui
    fixpartui.ui
    matpartui.ui
    inserttmplbase.ui
    materialdialog.ui
    insertplantbase.ui
    docheader.ui
    positionwidget.ui
    docfooter.ui
    texteditbase.ui
    positionwidget.ui
    doctypeeditbase.ui
    taxeditbase.ui
    wageseditbase.ui
    importtodocbase.ui
    numbercycleseditbase.ui

)

kde4_add_executable(kraft ${kraft_SRCS})

target_link_libraries(kraft kraftcat ${QT_AND_KDECORE_LIBS} ${QT_QTSQL_LIBRARY} ${KDE4_KHTML_LIBS} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KIO_LIBS}  ctemplate pthread)

install(TARGETS kraft ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES  kraft.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  kraftsettings.kcfg databasesettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES  docoverview.css docoverview_ro.css catalogview.css DESTINATION ${DATA_INSTALL_DIR}/kraft)
install(FILES  kraftui.rc katalogview.rc DESTINATION ${DATA_INSTALL_DIR}/kraft)

kde4_install_icons(${ICON_INSTALL_DIR})



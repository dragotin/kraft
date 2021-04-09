
include_directories(${QT_INCLUDES} src)
add_subdirectory(pics)

configure_file(${CMAKE_CURRENT_LIST_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

########### next target ###############
qt5_add_resources(KRAFT_RC_SRC pics/kraft.qrc)

set(kraft_SRCS
    attribute.cpp
    einheit.cpp
    doctype.cpp
    numbercycle.cpp
    katalogman.cpp
    stdsatzman.cpp
    katalog.cpp
    templkatalog.cpp
    docposition.cpp
    matkatalog.cpp
    calcdialogbase.cpp
    timecalcpart.cpp
    fixcalcpart.cpp
    floskeltemplate.cpp
    floskel.cpp
    calcpart.cpp
    kraftview.cpp
    kraftdoc.cpp
    catalogchapter.cpp
    addeditchapterdialog.cpp
    unitmanager.cpp
    stockmaterial.cpp
    materialcalcpart.cpp
    flostempldialog.cpp
    stdsatzman.cpp
    materialtempldialog.cpp
    timecalcdialog.cpp
    fixcalcdialog.cpp
    matcalcdialog.h
    matcalcdialog.cpp
    templatesaverbase.cpp
    templatesaverdb.cpp
    materialsaverbase.cpp
    materialsaverdb.cpp
    prefsdialog.cpp
    prefswages.cpp
    prefsunits.cpp
    katalogview.cpp
    filterheader.cpp
    kataloglistview.cpp
    templkatalogview.cpp
    templkataloglistview.cpp
    positionviewwidget.cpp
    documentsaverbase.cpp
    documentsaverdb.cpp
    exportxrechnung.cpp
    documentsaverxml.cpp
    dbtoxmlconverter.cpp
    documentman.cpp
    docdigest.cpp
    alldocsview.cpp
    doctypeedit.cpp
    doctext.cpp
    portal.cpp
    portalview.cpp
    archiveman.cpp
    reportgenerator.cpp
    htmlview.cpp
    docpostcard.cpp
    catalogtemplate.cpp
    catalogselection.cpp
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
    textselection.cpp
    newdocassistant.cpp
    docassistant.cpp
    texteditdialog.cpp
    templateprovider.cpp
    headertemplateprovider.cpp
    footertemplateprovider.cpp
    catalogtemplateprovider.cpp
    texttemplate.cpp
    grantleetemplate.cpp
    tagman.cpp
    itemtagdialog.cpp
    tagtemplatesdialog.cpp
    importitemdialog.cpp
    importfilter.cpp
    kraftview_ro.cpp
    taxeditdialog.cpp
    numbercycledialog.cpp
    impviewwidgets.cpp
    setupassistant.cpp
    models/documentmodel.cpp
    models/documentproxymodels.cpp
    models/datemodel.cpp
    models/docbasemodel.cpp
    docdigestdetailview.cpp
    addressprovider.cpp
    addressselectorwidget.cpp
    addressprovider_akonadi.cpp
    addressselectordialog.cpp
    kraftdb.cpp
    geld.cpp
    defaultprovider.cpp
    metaxmlparser.cpp
    texttemplateinterface.cpp
    documenttemplate.cpp
    pdfconverter.cpp
    format.cpp
    stringutil.cpp
    epcqrcode.cpp

    3rdparty/qrcodegen.cpp
)

kconfig_add_kcfg_files(kraft_SRCS
    databasesettings.kcfgc
    kraftsettings.kcfgc
)

ki18n_wrap_ui(kraft_SRCS
    calctemplate.ui
    dbinit.ui
    createdb.ui
    dbselect.ui
    mysqldetails.ui
    sqlitedetails.ui
    identity.ui
    statuspage.ui
    upgradedb.ui
    timepart.ui
    fixpartui.ui
    matpartui.ui
    inserttmplbase.ui
    materialdialog.ui
    docheader.ui
    positionwidget.ui
    docfooter.ui
    texteditbase.ui
    positionwidget.ui
    doctypeeditbase.ui
    taxeditbase.ui
    wageseditbase.ui
    unitseditbase.ui
    importtodocbase.ui
    numbercycleseditbase.ui
    xrechnung.ui
)

set(KRAFT_LINK_LIBS
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml

    KF5::I18n
    KF5::Contacts
    KF5::ConfigCore
    KF5::ConfigGui
    ${CTEMPLATE_LIBRARIES} pthread
    Grantlee5::Templates
)
if(${AKO_PREFIX}Akonadi_FOUND)
  list(APPEND KRAFT_LINK_LIBS
    ${AKO_PREFIX}::AkonadiCore
    ${AKO_PREFIX}::AkonadiContact
    ${AKO_PREFIX}::ContactEditor
    ${AKO_PREFIX}::AkonadiAgentBase
    ${AKO_PREFIX}::AkonadiWidgets
    ${AKO_PREFIX}::AkonadiXml
  )
endif()

add_library(kraftlib STATIC ${KRAFT_RC_SRC} ${kraft_SRCS})
target_compile_options(kraftlib PRIVATE -Wall -Wno-suggest-override)

add_executable(kraft main.cpp)

set_target_properties(kraft PROPERTIES OUTPUT_NAME kraft)

target_link_libraries(kraftlib
PUBLIC
    ${KRAFT_LINK_LIBS}
)

target_link_libraries(kraft
  PUBLIC
    kraftlib
)

install(TARGETS kraft ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kraftsettings.kcfg databasesettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES  kraftui.rc katalogview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kraft)

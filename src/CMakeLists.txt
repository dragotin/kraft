# include_directories(${QT_INCLUDES} src)
add_subdirectory(pics)

configure_file(${CMAKE_CURRENT_LIST_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

########### next target ###############
qt6_add_resources(KRAFT_RC_SRC pics/kraft.qrc)

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
    reportgenerator.cpp
    htmlview.cpp
    docpostcard.cpp
    catalogtemplate.cpp
    catalogselection.cpp
    kraftdocheaderedit.cpp
    kraftdocfooteredit.cpp
    inserttempldialog.cpp
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
    kraftattrib.cpp
    kraftobj.cpp
    xmldocindex.cpp
    jsonindexfile.cpp
    reportitemlist.cpp
    reportitem.cpp
    docidentgenerator.cpp
    myidentity.cpp
    3rdparty/qrcodegen.cpp
)

kconfig_add_kcfg_files(kraft_SRCS
    databasesettings.kcfgc
    kraftsettings.kcfgc
)

set( kraft_UIS
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
    documentactions.ui
    finalizedoc.ui
    dbtoxml.ui
)

ki18n_wrap_ui(kraft_SRCS ${kraft_UIS})



set(KRAFT_LINK_LIBS
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Xml

    KF6::I18n
    KF6::Contacts
    KF6::ConfigCore
    KF6::ConfigGui
    ${CTEMPLATE_LIBRARIES} pthread
    KF6::TextTemplate
)
if(${AKO_PREFIX}Akonadi_FOUND)
  list(APPEND KRAFT_LINK_LIBS
    ${AKO_PREFIX}::AkonadiContactCore
    ${AKO_PREFIX}::AkonadiContactWidgets
  )
endif()

add_library(kraftlib STATIC ${KRAFT_RC_SRC} ${kraft_SRCS})

qt_add_ui(kraftlib INCLUDE_PREFIX "src" SOURCES ${kraft_UIS})

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

install(FILES  kraftui.rc katalogview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kraft)

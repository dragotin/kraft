/***************************************************************************
             portalview.cpp  - the main portal class
                             -------------------
    begin                : 2004-05-09
    copyright            : (C) 2004 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// include files for Qt
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>
#include <QHBoxLayout>
#include <QtCore>
#include <QPushButton>

#include <KLocalizedString>

#include "version.h"
#include "kraftdb.h"
#include "portalview.h"
#include "portalhtmlview.h"
#include "katalogman.h"
#include "alldocsview.h"
#include "documentman.h"
#include "defaultprovider.h"
#include "reportgenerator.h"
#include "texttemplate.h"

PortalView::PortalView(QWidget *parent, const char*)
    : QWidget( parent ),
      _allDocsView(nullptr),
      mCatalogBrowser(nullptr),
      mSystemBrowser(nullptr)
{
    _contentsWidget = new QListWidget(this);
    _contentsWidget->setViewMode(QListView::IconMode);
    _contentsWidget->setIconSize(QSize(96, 84));
    _contentsWidget->setMovement(QListView::Static);
    _contentsWidget->setMaximumWidth(128);
    _contentsWidget->setSpacing(12);

    _pagesWidget = new QStackedWidget(this);
    _pagesWidget->addWidget(documentDigests());
    _pagesWidget->addWidget(new QWidget());  // doc timeline
    _pagesWidget->addWidget(katalogDetails()); // catalogs
    _sysPageIndx = _pagesWidget->addWidget(systemDetails()); // system

    createIcons();
    _contentsWidget->setCurrentRow(0);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(_contentsWidget);

    QPushButton *pb = new QPushButton(i18n("About Kraft"));
    pb->setIcon(QIcon::fromTheme("kraft", QIcon(":/kraft/global/kraft_small_arm.png")));
    vbox->addWidget(pb);

    connect(pb, &QPushButton::clicked, this, &PortalView::displaySystemsTab);
    horizontalLayout->addLayout(vbox);
    horizontalLayout->addWidget(_pagesWidget, 1);
    setLayout(horizontalLayout);
}

PortalView::~PortalView( )
{
}

void PortalView::createIcons()
{
    QListWidgetItem *documentsButton = new QListWidgetItem(_contentsWidget);
    documentsButton->setIcon(QIcon(":/kraft/document-new.png"));
    documentsButton->setText(i18n("Documents"));
    documentsButton->setTextAlignment(Qt::AlignHCenter);
    documentsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *timeLineButton = new QListWidgetItem(_contentsWidget);
    timeLineButton->setIcon(QIcon(":/kraft/document-open-recent.png"));
    timeLineButton->setText(i18n("Timeline"));
    timeLineButton->setTextAlignment(Qt::AlignHCenter);
    timeLineButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *catButton = new QListWidgetItem(_contentsWidget);

    catButton->setIcon(QIcon(":/kraft/catalogue.png"));
    catButton->setText(i18n("Catalogs"));
    catButton->setTextAlignment(Qt::AlignHCenter);
    catButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect( _contentsWidget, &QListWidget::itemClicked,
             this, &PortalView::changePage);
}

void PortalView::changePage(QListWidgetItem *current)
{
    if (!current)
        return;

    int indx = _contentsWidget->row(current);
    if( indx == 0 ) {
        // the flat documents list
        _allDocsView->setView( AllDocsView::FlatList );
    } else if( indx == 1 ) {
        // the timeline
        _allDocsView->setView( AllDocsView::TreeView );
        indx = 0;
    }

    _pagesWidget->setCurrentIndex(indx);
}

void PortalView::displaySystemsTab()
{
    _pagesWidget->setCurrentIndex(_sysPageIndx);
}

QWidget* PortalView::katalogDetails()
{
  QWidget *w = new QWidget;

  QBoxLayout *b = new QHBoxLayout;
  w->setLayout( b );
  mCatalogBrowser = new PortalHtmlView( w );
  mCatalogBrowser->setTitle( i18n( "Kraft Document Overview" ) );
  mCatalogBrowser->setStylesheetFile( "catalogview.css" );

  b->addWidget( mCatalogBrowser );

  QString html;

  html = "<h2>" + i18n("Available Catalogs") + "</h2>";
  html += "<div>\n";
  html += i18n( "No catalogs available." );
  html += "</div>";
  mCatalogBrowser->displayContent( html );

  connect( mCatalogBrowser, SIGNAL( openCatalog( const QString& ) ),
           SIGNAL( openKatalog( const QString& ) ) );

  return w;
}

void PortalView::fillCatalogDetails()
{
  if ( ! mCatalogBrowser ) return;

    const QStringList katalogNamen = KatalogMan::self()->allKatalogNames();
    QString html;

    html = QStringLiteral("<br/><h2>") + i18n("Available Catalogs") + QStringLiteral("</h2>");
    html += QStringLiteral("<div><table width=\"60%\" cellpadding=\"4\" cellspacing=\"0\" border=\"0\">\n");

    int cnt = 0;
    for(QStringList::ConstIterator namesIt = katalogNamen.begin();
        namesIt != katalogNamen.end(); ++namesIt )
    {
        QString katName = *namesIt;
        html += printKatLine( katName, cnt++ );
    }

    html += QStringLiteral("</table></div>\n");

    mCatalogBrowser->displayContent( html );
}

QString PortalView::printKatLine( const QString& name, int cnt ) const
{
    QString urlName = name.toHtmlEscaped();

    // qDebug () << "Converted Katalog name: " << urlName << endl;
    QString html;

    html += "<tr";
    if ( cnt % 2 ) {
      html += " class=\"odd\"";
    }
    html += ">\n";

    html += "<td class=\"bigfont\"><div>"+urlName+"</div></td>";
    html += "<td class=\"bigfont\" align=\"right\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=open\">";
    html += i18n("Open");
    html += "</a></td></tr>";

    KatalogMan::CatalogDetails details = KatalogMan::self()->catalogDetails(name);
    html += "<tr";
    if ( cnt % 2 ) {
      html += " class=\"odd\"";
    }
    html += ">\n";

    if( details.countEntries == 0 ) {
        html += QString("<td colspan=\"2\"><span style=\"font-size:75%;\">%1</span></td>").arg(i18n("No templates yet."));
    } else {
        QLocale *locale = DefaultProvider::self()->locale();
        QString dateStr = locale->toString(details.maxModDate);
        html += QString("<td class=\"sub\" colspan=\"2\">") +
                i18n("%1 templates in %2 chapters<br/>last modified at %3",
                details.countEntries, details.countChapters, dateStr)
                + QLatin1String("</td>");
    }
#if 0
    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=xml\">";
    html += i18n("XML Export");
    html += "</td>";

    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=delete\">";
    html += i18n("Remove");
    html += "</td>";
#endif
    html += "</tr>\n";
    return html;
}

QString PortalView::ptag( const QString& content,  const QString& c ) const
{
  QString html( "<p" );
  if ( ! c.isEmpty() ) {
    html += QString( " class=\"%1\"" ).arg( c );
  }
  html += ">";
  html += content;
  html += "</p>";

  return html;
}

QWidget* PortalView::systemDetails()
{
  QWidget *w = new QWidget;
  QBoxLayout *b = new QHBoxLayout;
  w->setLayout( b );
  mSystemBrowser = new PortalHtmlView( w );
  b->addWidget( mSystemBrowser );
  mSystemBrowser->setStylesheetFile( "systemview.css" );

  return w;
}

QString PortalView::systemView( const QString& htmlMsg ) const
{
  if ( ! mSystemBrowser ) return QString ("");

  const QString templateName = ( htmlMsg.isNull() ? QString( "views/systemviewdetails.thtml" ) : QString ( "views/systemviewerror.thtml" ) );
  const QString tmplFile = DefaultProvider::self()->locateFile( templateName );

  // Note: This code is stolen from DocDigestDetailView::slotShowDocDetails
  // It should be refactored.
  TextTemplate tmpl;
  tmpl.setTemplateFileName(tmplFile);
  if( !tmpl.isOk() ) {
      return QString ("");
  }

  const QString logoFile = DefaultProvider::self()->locateFile("styles/pics/kraftapp_logo_trans.png" );

  tmpl.setValue( "KRAFT_LOGO_FILE", logoFile );
  tmpl.setValue( "KRAFT_WEBSITE", i18n( "Kraft Website" ) );
  QDate d = QDate::currentDate();
  tmpl.setValue( "KRAFT_COPYRIGHT_YEAR", QString::number(d.year()) );
  tmpl.setValue( "KRAFT_LICENSE_TEXT", i18nc("The string is followed by a link to the GPL2 text", "Kraft is free software licensed under the"));
  tmpl.setValue( "KRAFT_GITHUB", i18nc("The string is followed by the link to github", "Kraft is maintained on "));
  tmpl.setValue( "KRAFT_AUTHORS", i18n("Authors"));
  tmpl.setValue( "KRAFT_MAINTAINER", i18n("Developer and Maintainer"));
  tmpl.setValue( "KRAFT_DEVELOPER", i18n("Developer"));
  tmpl.setValue( "KRAFT_GRAPHICS", i18nc("The person who provided the logo graphics", "Logo design"));
  tmpl.setValue( "KRAFT_MANUAL", i18nc("The person who provided the user manual", "User Manual"));
  // kraft infos
  tmpl.setValue("KRAFT_INTRO_DESC", i18n("Kraft helps you to handle documents like quotes and invoices in your small business."));
  tmpl.setValue( "KRAFT_WELCOME_LABEL", i18n( "Welcome to Kraft" ) );
  tmpl.setValue( "KRAFT_VERSION_LABEL", i18n( "Kraft Version" ) );
  tmpl.setValue( "KRAFT_VERSION", KRAFT_VERSION );
  tmpl.setValue( "KRAFT_CODENAME_LABEL", i18n( "Codename" ) );
  tmpl.setValue( "KRAFT_CODENAME", KRAFT_CODENAME );
  const QString countryName = DefaultProvider::self()->locale()->nativeCountryName();
  tmpl.setValue( "COUNTRY_SETTING_LABEL", i18n( "Country Setting" ) );
  tmpl.setValue( "COUNTRY_SETTING", QString( "%1 (%2)" ).arg( countryName ).arg( DefaultProvider::self()->locale()->country() ));
  const QString languageName = DefaultProvider::self()->locale()->nativeLanguageName();
  tmpl.setValue( "LANGUAGE_SETTING_LABEL", i18n( "Language Setting" ) );
  tmpl.setValue( "LANGUAGE_SETTING", QString( "%1 (%2)" ).arg( languageName ).arg( DefaultProvider::self()->locale()->language() ));

  if ( !htmlMsg.isNull() ) {
      tmpl.setValue( "ERROR_TITLE_LABEL", i18n( "Kraft Initialisation Problem" ) );
      QString errorMessage = i18n( "There is a initialisation error on your system. Kraft will not work that way." );
      errorMessage += htmlMsg;
      tmpl.setValue( "ERROR_TEXT", errorMessage );
      return tmpl.expand();
  }

  // database infos
  tmpl.setValue( "DATABASE_TITLE_LABEL", i18n( "Database Information" ) );
  tmpl.setValue( "DATABASE_NAME_LABEL", i18n( "Kraft database name" ) );
  tmpl.setValue( "DATABASE_NAME", KraftDB::self()->databaseName() );

  QString schemaVersion = QString::number( KraftDB::self()->currentSchemaVersion() );
  if ( KraftDB::self()->currentSchemaVersion() != KRAFT_REQUIRED_SCHEMA_VERSION ) {
    schemaVersion += " - " + QString( "<font color='red'>%1: %2</font>" ).arg( i18n ( "Required Version" ))
            .arg( KRAFT_REQUIRED_SCHEMA_VERSION );
  }
  tmpl.setValue( "DATABASE_SCHEMA_VERSION_LABEL", i18n( "Database schema version" ) );
  tmpl.setValue( "DATABASE_SCHEMA_VERSION", schemaVersion );
  tmpl.setValue( "DATABASE_DRIVER_LABEL", i18n( "Qt database driver" ) );
  tmpl.setValue( "DATABASE_DRIVER", KraftDB::self()->qtDriver() );

  bool dbOk = KraftDB::self()->getDB()->isOpen();
  const QString databaseConnection = ( dbOk ? i18n("established") : QString( "<font color='red'>%1</font>" ).arg( i18n( "NOT AVAILABLE!" ) ) );
  tmpl.setValue( "DATABASE_CONNECTION_LABEL", i18n( "Database connection" ) );
  tmpl.setValue( "DATABASE_CONNECTION", databaseConnection );

  if( dbOk ) {
    QSqlQuery q("SHOW VARIABLES like 'version';");
    if( q.isActive() ) {
      q.next();
      const QString version = q.value(1).toString();
      tmpl.createDictionary("DATABASE_VERSION_SECTION");
      tmpl.setValue( "DATABASE_VERSION_SECTION", "DATABASE_VERSION_LABEL", i18n( "Database Version" ) );
      tmpl.setValue( "DATABASE_VERSION_SECTION", "DATABASE_VERSION", version );
    }
  }

  // Akonadi and friends
  QScopedPointer<AddressProvider> aprov;
  aprov.reset( new AddressProvider);
  tmpl.setValue( "ADDRESSBOOK_BACKEND_LABEL", i18n( "Addressbook Backend" ) );
  tmpl.setValue( "ADDRESSBOOK_BACKEND_TYPE_LABEL", i18n( "Backend type" ) );
  const QString backendTypeValue = QString( "%1 (%2)").arg( aprov->backendName())
          .arg(aprov->backendUp() ? i18n("running") : i18n("not running") );
  tmpl.setValue( "ADDRESSBOOK_BACKEND_TYPE", backendTypeValue );

  // external tools
  tmpl.setValue( "EXTERNAL_TOOLS_LABEL", i18n( "External Tools" ) );
  tmpl.setValue( "RML2PDF_TOOL_LABEL", i18n( "RML to PDF conversion tool" ) );
  QStringList trml2pdf = DefaultProvider::self()->findTrml2Pdf();

  QString trml2pdfValue = (trml2pdf.count() ? trml2pdf.join(" ") : i18n("not found!") );
  tmpl.setValue( "RML2PDF_TOOL", trml2pdfValue );

  tmpl.setValue( "ICONV_TOOL_LABEL", i18n( "iconv tool for text import" ) );
  tmpl.setValue( "ICONV_TOOL", DefaultProvider::self()->iconvTool() );

  tmpl.setValue( "WEASYPRINT_TOOL_LABEL", i18n( "<a href=\"https://weasyprint.org/\">weasyprint</a> for PDF generation" ) );

  QString wp = DefaultProvider::self()->locateBinary("weasyprint");
  if (wp.isEmpty()) {
      wp = i18n("not available");
  }
  tmpl.setValue( "WEASYPRINT_TOOL", wp);

  // aknowledgement
  tmpl.setValue( "ICON_ACKNOWLEDGEMENT_LABEL", i18n("Some Icons are made by") );
  tmpl.setValue( "ACKNOWLEGEMENT_LABEL", i18n( "Acknowledgements" ) );

  return tmpl.expand();
}

void PortalView::fillSystemDetails()
{
  const QString html = systemView( QString() );
  mSystemBrowser->displayContent( html );
}

void PortalView::systemInitError( const QString& htmlMsg )
{
  const QString html = systemView( htmlMsg );
  mSystemBrowser->displayContent( html );
}

QWidget* PortalView::documentDigests()
{
  QWidget *w = new QWidget;

  QBoxLayout *b = new QHBoxLayout;
  b->setContentsMargins( 0, 0, 0, 0 );
  w->setLayout( b );

  _allDocsView = new AllDocsView( w );
  b->addWidget( _allDocsView );

  connect( _allDocsView, SIGNAL( createDocument() ),
           this, SLOT( slotCreateDocument() ) );
  connect( _allDocsView, SIGNAL( openDocument( const QString& ) ),
           SIGNAL( openDocument( const QString& ) ) );
  connect( _allDocsView, SIGNAL( viewDocument( const QString& ) ),
           SIGNAL( viewDocument( const QString& ) ) );
  connect( _allDocsView, SIGNAL( copyDocument( const QString& ) ),
           SIGNAL( copyDocument( const QString& ) ) );
  connect( _allDocsView, SIGNAL( openArchivedDocument( const ArchDocDigest& ) ),
           SIGNAL( openArchivedDocument( const ArchDocDigest& ) ) );
  connect( _allDocsView, SIGNAL( docSelected( const QString& ) ),
           SIGNAL( documentSelected( const QString& ) ) );
  connect( _allDocsView, SIGNAL( openArchivedDocument( const ArchDocDigest& ) ),
           SIGNAL( archivedDocSelected( const ArchDocDigest& ) ) );

  return w;
 }

void PortalView::slotCreateDocument()
{
  // this slot is called if the user wants to initiate the creation of a new doc
  // It is routed to higher layers.
  emit createDocument();
}

void PortalView::slotBuildView()
{

  // QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  _allDocsView->slotBuildView();
  // QApplication::restoreOverrideCursor();
}

/* END */


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

PortalView::PortalView(QWidget *parent, const char*)
    : QWidget( parent ),
      _allDocsView( 0 ),
      mCatalogBrowser( 0 ),
      mSystemBrowser( 0 )
{
    _contentsWidget = new QListWidget;
    _contentsWidget->setViewMode(QListView::IconMode);
    _contentsWidget->setIconSize(QSize(96, 84));
    _contentsWidget->setMovement(QListView::Static);
    _contentsWidget->setMaximumWidth(128);
    _contentsWidget->setSpacing(12);

    _pagesWidget = new QStackedWidget;
    _pagesWidget->addWidget(documentDigests());
    _pagesWidget->addWidget(new QWidget());  // doc timeline
    _pagesWidget->addWidget(katalogDetails()); // catalogs
    _sysPageIndx = _pagesWidget->addWidget(systemDetails()); // system

    createIcons();
    _contentsWidget->setCurrentRow(0);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(_contentsWidget);

    QPushButton *pb = new QPushButton(i18n("Information"));
    pb->setIcon(QIcon::fromTheme("kraft"));
    vbox->addWidget(pb);
    connect(pb, SIGNAL(clicked(bool)), this, SLOT(displaySystemsTab()));
    horizontalLayout->addLayout(vbox);
    horizontalLayout->addWidget(_pagesWidget, 1);
    setLayout(horizontalLayout);
}

void PortalView::createIcons()
{
    QListWidgetItem *documentsButton = new QListWidgetItem(_contentsWidget);
    documentsButton->setIcon(QIcon::fromTheme("document-new"));
    documentsButton->setText(i18n("Documents"));
    documentsButton->setTextAlignment(Qt::AlignHCenter);
    documentsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *timeLineButton = new QListWidgetItem(_contentsWidget);
    timeLineButton->setIcon(QIcon::fromTheme("document-open-recent"));
    timeLineButton->setText(tr("Timeline"));
    timeLineButton->setTextAlignment(Qt::AlignHCenter);
    timeLineButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *catButton = new QListWidgetItem(_contentsWidget);

    catButton->setIcon(QIcon::fromTheme("catalogue"));
    catButton->setText(tr("Catalogs"));
    catButton->setTextAlignment(Qt::AlignHCenter);
    catButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(_contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void PortalView::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
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

  html = "<html><h2>" + i18n("Available Catalogs") + "</h2>";
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

    html = "<html><h2>" + i18n("Available Catalogs") + "</h2>";
    html += "<div>\n";
    html += "<table border=\"0\">";

    int cnt = 0;
    for(QStringList::ConstIterator namesIt = katalogNamen.begin();
        namesIt != katalogNamen.end(); ++namesIt )
    {
        QString katName = *namesIt;
        html += printKatLine( katName, cnt++ );
    }

    html += "</table></div></html>\n";

    mCatalogBrowser->displayContent( html );
}

QString PortalView::printKatLine( const QString& name, int cnt ) const
{
    QString urlName = Qt::escape( name );

    // qDebug () << "Converted Katalog name: " << urlName << endl;
    QString html;

    html += "<tr";
    if ( cnt % 2 ) {
      html += " class=\"odd\"";
    }
    html += ">\n";

    html += "<td><b>"+urlName+"</b></td>";
    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=open\">";
    html += i18n("Open");
    html += "</td></tr>";

    KatalogMan::CatalogDetails details = KatalogMan::self()->catalogDetails(name);
    html += "<tr";
    if ( cnt % 2 ) {
      html += " class=\"odd\"";
    }
    html += ">\n";

    if( details.countEntries == 0 ) {
        html += "<td colspan=\"2\"><span style=\"font-size:75%;\">No templates yet.</span></td>";
    } else {
        QLocale *locale = DefaultProvider::self()->locale();
        QString dateStr = locale->toString(details.maxModDate);
        html += QString("<td colspan=\"2\"><span style=\"font-size:75%;\">%1 templates in %2 chapters, last modified at %3</span></td>").
                arg(details.countEntries).arg(details.countChapters).arg(dateStr);
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
  mSystemBrowser->setStylesheetFile( "catalogview.css" ); //, "mucki_en_oS.png",

  // browser->setNotifyClick(false);
  return w;
}

QString PortalView::systemViewHeader() const
{

  QString html( "" );

  QString logoFile = DefaultProvider::self()->locateFile("pics/kraftapp_logo.png" );
  html += i18n( "<h2>Welcome to Kraft</h2>" );
  html += "<div><table width=\"100%\" border=\"0\"><tr><td>";
  html += i18n("Kraft Version: %1</td>").arg( KRAFT_VERSION );
  html += "<td align=\"right\" rowspan=\"3\">";
  if ( ! logoFile.isEmpty() ) {
    html += QString( "<img src=\"%1\"/><br/>" ).arg( logoFile );
  }
  html += QString("<a href=\"http://www.volle-kraft-voraus.de\">%1</a>&nbsp;").arg(i18n("Kraft Website"));
  html += "</td></tr>";

  html += QString( "<tr><td>Codename: <i>%1</i></td></tr>" ).arg( KRAFT_CODENAME );
  QString h1 = DefaultProvider::self()->locale()->nativeCountryName();
  html += QString( "<tr><td>" ) + i18n( "Country Setting: " ) +
          QString( "<i>%1 (%2)</i></td></tr>" ).arg( h1 ).arg( DefaultProvider::self()->locale()->country() );
  h1 = DefaultProvider::self()->locale()->nativeLanguageName();
  html += QString( "<tr><td>" ) + i18n( "Language Setting: " ) +
          QString( "<i>%1 (%2)</i></td></tr>" ).arg( h1 ).arg( DefaultProvider::self()->locale()->language() );
  html += "</table></div>";

  return html;
}

void PortalView::fillSystemDetails()
{
  QString html;
  if ( ! mSystemBrowser ) return;

  html = systemViewHeader();

  html += "<h3>" + i18n("Database Information") + "</h3>";
  html += "<div><table>";
  html += "<tr><td>" + i18n( "Kraft database name:" ) + "</td>";
  html += "<td>" + KraftDB::self()->databaseName() + "</td></tr>";

  html += "<tr><td>" + i18n( "Database schema version:" ) + "</td>";
  html += "<td>" + QString::number( KraftDB::self()->currentSchemaVersion() );
  if ( KraftDB::self()->currentSchemaVersion() != KRAFT_REQUIRED_SCHEMA_VERSION ) {
    html += "&nbsp;-&nbsp;" + QString( "<font color=\"red\">Required Version: %1</font>" )
            .arg( KRAFT_REQUIRED_SCHEMA_VERSION );
  }
  html += "</td></tr>";

  html += "<tr><td>" + i18n( "Qt database driver:" ) + "</td>";
  html += "<td>" +  KraftDB::self()->qtDriver() + "</td></tr>";

  html += "<tr><td>" + i18n( "Database connection:" ) + "</td>";
  html += "<td>";

  bool dbOk = false;
  if( KraftDB::self()->getDB()->isOpen() ) {
    dbOk = true;
    html += i18n("established");
  } else {
    html += i18n("<font color=\"red\">NOT AVAILABLE!</font>");
  }
  html += "</td></tr>";

  if( dbOk ) {
    QSqlQuery q("SHOW VARIABLES like 'version';");
    if( q.isActive() ) {
      q.next();
      QString version = q.value(1).toString();
      html += "<tr><td>" + i18n( "Database Version:" ) + "</td>";
      html += "<td>" +  version + "</td></tr>";
    }
  }
  html += "</table></div>";
  // external tools
  html += "<h3>" + i18n( "External Tools" ) + "</h3>";
  html += "<div><table>";
  html += "<tr><td>" + i18n( "RML to PDF conversion tool:" ) + "</td><td>";
  QStringList trml2pdf = ReportGenerator::self()->findTrml2Pdf();
  if( trml2pdf.count() ) {
    html += trml2pdf.join(" ")+ "</td></tr>";
  } else {
    html += i18n("not found!") + "</td></tr>";
  }
  html += "<tr><td>" + i18n( "iconv tool for text import:" ) + "</td><td>";
  html += DefaultProvider::self()->iconvTool() + "</td></tr>";

  html += "</table></div>";

  html += "<h3>" + i18n( "Acknowledgements" ) + "</h3>";
  html += "<p><div>Some Icons are made by <a href=\"https://www.flaticon.com/authors/madebyoliver\" "
          "title=\"Madebyoliver\">Madebyoliver</a> from <a href=\"https://www.flaticon.com/\" title=\"Flaticon\">www.flaticon.com</a> "
          ", licensed by <a href=\"http://creativecommons.org/licenses/by/3.0/\" "
          "title=\"Creative Commons BY 3.0\">CC 3.0 BY</a></div><p>";

  mSystemBrowser->displayContent( html );
}

void PortalView::systemInitError( const QString& htmlMsg )
{
  QString html = systemViewHeader(); // "<h2>" + i18n("Kraft System Information") + "</h2>";

  html += "<h2 class=\"error\">Kraft Initialisation Problem:</h2>";
  html += "<div class=\"error\">";
  html += ptag( i18n( "There is a initialisation error on your system. "
                       "Kraft will not work that way." ) );
  html += ptag( htmlMsg );
  html += "</div>";

  mSystemBrowser->displayContent( html ); // , "error" );
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
  // connect( mDocDigestView, SIGNAL( printDocument( const QString& ) ),
  //         SIGNAL( printDocument( const QString& ) ) );
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

PortalView::~PortalView( )
{
  delete _allDocsView;
}

/* END */


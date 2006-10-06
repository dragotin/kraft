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
#include <qvbox.h>
#include <qsqlquery.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <ktextbrowser.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <klistview.h>

#include "version.h"
#include "kraftdb.h"
#include "portalview.h"
#include "katalogman.h"
#include "docdigestview.h"
#include "documentman.h"

PortalView::PortalView(QWidget *parent, const char *name, int face)
    : KJanusWidget( parent, name, face ),
      m_docBox(0),
      m_katalogBox(0),
      mArchiveBox( 0 )
{
    m_docBox     = addVBoxPage( i18n("Documents"),
                                i18n("Create and Edit Documents"),
                                DesktopIcon("folder_outbox"));
    documentDigests( m_docBox );
#if 0
    mArchiveBox  = addVBoxPage( i18n( "Archive" ),
                                i18n( "Archived Documents" ),
                                DesktopIcon( "vcs_commit" ) );
    archiveDetails( mArchiveBox );
#endif
    m_katalogBox = addVBoxPage( i18n("Catalogs"),
                                i18n("Available Catalogs"),
                                DesktopIcon("folder_green"));
    katalogDetails(m_katalogBox);

    m_sysBox     = addVBoxPage( i18n("System"),
                                i18n("Information about the Kraft System"),
                                DesktopIcon("server"));
    systemDetails( m_sysBox );
}

void PortalView::katalogDetails(QWidget *parent)
{
    QStringList katalogNamen = KatalogMan::self()->allKatalogs();
    QString html;

    html = "<h2>" + i18n("Available Catalogs") + "</h2>";
    html += "<p align=\"center\"><table width=\"80%\" border=\"0\">";

    for(QStringList::ConstIterator namesIt = katalogNamen.begin();
        namesIt != katalogNamen.end(); ++namesIt )
    {
        QString katName = *namesIt;
        html += printKatLine( katName );
    }

    html += "</table></p>";

    KTextBrowser *katBrowser = new KTextBrowser(parent);
    katBrowser->setText(html);
    katBrowser->setNotifyClick(true);
    connect( katBrowser, SIGNAL(urlClick(const QString&)),
             this, SLOT(slUrlClicked(const QString&)));
}

void PortalView::archiveDetails( QWidget *  )
{

}

QString PortalView::printKatLine( const QString& name ) const
{
    QString urlName(name); //  = KURL::encode_string(name);

    kdDebug() << "Converted Katalog name: " << urlName << endl;
    QString html;

    html += "<tr>";

    html += "<td><b>"+urlName+"</b></td>";
    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=open\">";
    html += i18n("Open");
    html += "</td>";

    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=xml\">";
    html += i18n("XML Export");
    html += "</td>";

    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=delete\">";
    html += i18n("Remove");
    html += "</td>";

    html += "</tr>\n";
    return html;
}

void PortalView::slUrlClicked( const QString& urlStr )
{
    KURL url( urlStr );

    kdDebug() << "URL: " << url.path() << endl;
    if( url.path().startsWith( "/katalog.cgi") )
    {
        QString action = url.queryItem("action");
        QString kat    = url.queryItem("kat");
        // Katalog editieren
        if( action == "open" )
        {
            emit openKatalog(kat);
        }
        else if( action == "delete" )
        {
            emit deleteKatalog(kat);
        }
        else if( action == "xml" )
        {
            emit( katalogToXML(kat));
        }
        else
        {
            // unknown query
            kdDebug() << "Can not handle Query: " << url.query() << endl;
        }
    }
    else
    {
        // weitere
    }
}

void PortalView::systemDetails(QWidget *parent)
{
    QString html;
    const QString ptag = "<p class=\"infoline\">";

    KStandardDirs stdDirs;
    QString logoFile = stdDirs.findResource( "data",  "kraft/pics/muckilogo_oS.png" );


    html = ""; // "<h2>" + i18n("Kraft System Information") + "</h2>";

    html += "<table width=\"100%\"><tr><td>";
    html += ptag + i18n("Kraft Version: ") + KRAFT_VERSION +  "</p></td>";
    html += "<td align=\"right\" rowspan=\"2\">";
    if ( ! logoFile.isEmpty() ) {
      html += QString( "<img src=\"%1\"/>" ).arg( logoFile );
    } else {
      html += "&nbsp;";
    }
    html += "</td></tr>";
    html += QString( "<tr><td>Codename <i>%1</i></td></tr>" ).arg( KRAFT_CODENAME );
    html += "</table>";

    html += "<h2>" + i18n("Database Information") + "</h2>";
    html += ptag + i18n("Qt Database Driver: ") + KraftDB::qtDriver() +  "</p>";

    html += ptag + i18n("Database connection ");
    bool dbOk = false;
    if( KraftDB::getDB() ) {
        dbOk = true;
        html += i18n("established");
    } else {
        html += i18n("<font color=\"red\">NOT AVAILABLE!</font>");
    }
    html += "</p>";
    if( dbOk ) {
        QSqlQuery q("SHOW VARIABLES like 'version';");
        if( q.isActive() ) {
            q.next();
            QString version = q.value(1).toString();
            html += ptag + i18n("Database Version: %1").arg( version );
        }
    }

    KTextBrowser *browser = new KTextBrowser(parent);
    kdDebug() << "This is the html: " << html << endl;
    browser->setText(html);
    // browser->setNotifyClick(false);
}

void PortalView::documentDigests( QWidget *parent )
{
  mDocDigestView = new DocDigestView( parent );

  connect( mDocDigestView, SIGNAL( createDocument() ),
             SIGNAL( createDocument() ) );
  connect( mDocDigestView, SIGNAL( openDocument( const QString& ) ),
             SIGNAL( openDocument( const QString& ) ) );
  connect( mDocDigestView, SIGNAL( printDocument( const QString& ) ),
             SIGNAL( printDocument( const QString& ) ) );
  connect( mDocDigestView, SIGNAL( selectionChanged( const QString& ) ),
             SIGNAL( documentSelected( const QString& ) ) );
  slotBuildView();
}

void PortalView::slotBuildView()
{
  DocumentMan *docman = DocumentMan::self();
  mDocDigestView->listview()->clear();
  mDocDigestView->addLatestDocs( docman->latestDocs( 10 ) );
}

PortalView::~PortalView( )
{

}

/* END */


#include "portalview.moc"

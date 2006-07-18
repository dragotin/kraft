/***************************************************************************
              brunsviewer.cpp  - catalog viewer for bruns catalogs
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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

// include files for QT
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qtimer.h>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kdebug.h>

// application specific includes
#include "brunsviewer.h"
#include "brunskatalogview.h"

#define ID_STATUS_MSG 1

Brunsviewer::Brunsviewer(QWidget* , const char* name):KMainWindow(0, name)
{
  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  // initActions();
  initView();

  // check for database init
  // KraftDB::checkInit();
  QTimer::singleShot( 0, this, SLOT( slotStartupChecks() ) );
}

Brunsviewer::~Brunsviewer()
{

}

void Brunsviewer::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  // STATUSBAR
  // TODO: add your own items you need for displaying current application status.
  statusBar()->insertItem(i18n("Ready."), ID_STATUS_MSG);
}


void Brunsviewer::initView()
{
    const QString kat = i18n("BRUNS Pflanzenkatalog 2005");
    
    BrunsKatalogView *lv = new BrunsKatalogView( );
    lv->init( kat );
    lv->show();

    setCentralWidget( lv );

    setCaption( kat, false );

}

void Brunsviewer::slotStartupChecks()
{

}

void Brunsviewer::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

#if 0
void Brunsviewer::slotKatalogToXML(const QString& katName)
{
    kdDebug() << "Generating XML for catalog " << katName << endl;

    Katalog *kat = KatalogMan::getKatalog(katName);

    if(kat) {
        kat->writeXMLFile();
    }
}
#endif

QString Brunsviewer::textWrap( const QString& t, unsigned int width )
{
    QString re;

    if( t.length() < width )
    {
        re = t;
    }
    else
    {
        unsigned int start = 0;
        int pos = width;
        while( pos < (int) t.length() )
        {
            pos = t.find( ' ', start+width );
            if( pos > -1 ) {
                re += t.mid( start, pos-start)+'\n';
                start = pos;
            } else {
                re += t.mid( start );
                pos = t.length();
            }
        }

    }

    return re;
}


#include "brunsviewer.moc"

/***************************************************************************
                        portalhtmlview.cpp  - show a html page in the portal
                             -------------------
    begin                : Jan 2007
    copyright            : (C) 2007 Klaas Freitag <freitag@kde.org>
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "portalhtmlview.h"

#include <klocale.h>
#include <kdebug.h>

PortalHtmlView::PortalHtmlView( QWidget *parent )
  : HtmlView( parent )
{

}

bool PortalHtmlView::urlSelected( const QString &url, int, int,
                                  const QString &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &)
{
  kDebug() << "HtmlView::urlSelected(): " << url << endl;

  KUrl kurl( url );
  const QString katName = kurl.queryItem( "kat" );
  const QString action = kurl.queryItem( "action" );

  kDebug() << "Action " << action;

  if ( action == "open" ) {
    kDebug() << "open catalog " << katName << endl;
    emit( openCatalog( katName ) );
    return true;
  } else {
    kDebug() << "unknown action " << action << endl;
  }
  return false;
}





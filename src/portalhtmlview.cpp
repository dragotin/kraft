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

#include <QDebug>
#include <QUrlQuery>

PortalHtmlView::PortalHtmlView( QWidget *parent )
  : HtmlView( parent )
{
    connect( this, SIGNAL(openUrl(QUrl)), this, SLOT(slotLinkClicked(QUrl)));
}

void PortalHtmlView::slotLinkClicked(const QUrl& url)
{
    QUrlQuery q(url);

    const QString katName = q.queryItemValue(QLatin1String("kat"));
    const QString action  = q.queryItemValue(QLatin1String("action"));
    if ( action == QLatin1String("open") ) {
      // qDebug () << "open catalog " << katName << endl;
      emit( openCatalog( katName ) );
    } else {
        if( url.isValid() ) {
            QDesktopServices::openUrl(url);
        }
    }
}





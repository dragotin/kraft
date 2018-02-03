 /***************************************************************************
              portalhtmlview.h  - show a html page in the portal
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

#ifndef PORTALHTMLVIEW_H
#define PORTALHTMLVIEW_H

#include "htmlview.h"

class PortalHtmlView : public HtmlView
{
    Q_OBJECT

public:
    PortalHtmlView( QWidget *parent );
signals:
    void openCatalog( const QString& );

protected slots:
    void slotLinkClicked(const QUrl& url);

};

#endif

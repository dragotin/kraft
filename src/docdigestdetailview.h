/***************************************************************************
             docdigestdetailview.cpp  - Details of a doc digest
                             -------------------
    begin                : februry 2011
    copyright            : (C) 2011 by Klaas Freitag
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
#ifndef DOCDIGESTDETAILVIEW_H
#define DOCDIGESTDETAILVIEW_H

#include <QWidget>

#include "docdigest.h"

class HtmlView;


class DocDigestDetailView : public QWidget
{
    Q_OBJECT
public:
    explicit DocDigestDetailView(QWidget *parent = 0);

signals:

public slots:
  void slotShowDocDetails( DocDigest );

private:
  HtmlView *mHtmlCanvas;
  QString   mTemplFile;
};

#endif // DOCDIGESTDETAILVIEW_H

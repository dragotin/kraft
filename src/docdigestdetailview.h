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
#include "htmlview.h"

class dbID;

class DocDigestHtmlView : public HtmlView
{
    Q_OBJECT

public:
    DocDigestHtmlView( QWidget *parent );

signals:
    void showLastPrint( const dbID& );

protected slots:
    void slotLinkClicked(const QUrl& url);

};

class DocDigestDetailView : public QWidget
{
  Q_OBJECT
public:
  explicit DocDigestDetailView(QWidget *parent = 0);

signals:
  void showLastPrint( const dbID& );

public slots:
  void slotShowDocDetails( DocDigest );
  void slotClearView();

  void slotShowMonthDetails( int year, int month );
  void slotShowYearDetails( int year);

private:
  DocDigestHtmlView *mHtmlCanvas;
  QString   _docTemplFileName;
  QString   _monthTemplFileName;
  QString   _yearTemplFileName;
};

#endif // DOCDIGESTDETAILVIEW_H

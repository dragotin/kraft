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

  protected:
    // virtual void writeBottomFrame();
    bool urlSelected( const QString&, int, int,
                      const QString &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments & );
  private:
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

private:
  DocDigestHtmlView *mHtmlCanvas;
  QString   mTemplFile;
};

#endif // DOCDIGESTDETAILVIEW_H

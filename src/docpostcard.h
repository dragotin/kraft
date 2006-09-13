/***************************************************************************
                 DocPostCard - a postcard version of the document
                             -------------------
    begin                : Aug 2006
    copyright            : (C) 2006 by Klaas Freitag
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

#ifndef DOCPOSTCARD_H
#define DOCPOSTCARD_H

#include "htmlview.h"
#include "kraftdoc.h"

class KURL;

class DocPostCard : public HtmlView
{
    Q_OBJECT
  public:
    enum DisplayMode { Full, Mini };
    enum PageId { HeaderId = 0, PositionId, FooterId };
    DocPostCard( QWidget *parent );

  signals:
    void selectPage( int );

  public slots:
    void setHeaderData( const QString&, const QString&, const QString&, const QString&, const QString& );
    void setPositions( DocPositionList );
    void setFooterData( const QString&,  const QString& );
    void renderDoc();
    void slotSetMode( DisplayMode );
  protected:
    void urlSelected( const QString &, int , int ,
                      const QString &, KParts::URLArgs  );
    void writeTopFrame();
    QString renderDocMini() const;
    QString renderDocFull() const;
  private:
    DocGuardedPtr mDoc;
    QString mType;
    QString mId;
    QString mPreText;
    QString mPostText;
    QString mDate;
    QString mAddress;
    QString mPositions;
    QString mGoodbye;
    QString mTotal;
    int mPositionCount;
    DisplayMode mMode;
};

#endif

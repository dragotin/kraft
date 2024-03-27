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

#include <qstring.h>

#include "docposition.h"
#include "kraftdoc.h"
#include "htmlview.h"
#include "docguardedptr.h"

class QUrl;

class DocPostCard : public HtmlView
{
  Q_OBJECT
public:
  enum DisplayMode { Full, Mini };

  DocPostCard( QWidget *parent = 0 );

signals:
  void selectPage( KraftDoc::Part );

public slots:
  void setHeaderData( const QString&, const QString&, const QString&, const QString&, const QString& );
  void setPositions( DocPositionList, DocPositionBase::TaxType, double, double );
  void setFooterData( const QString&,  const QString& );
  void renderDoc( KraftDoc::Part p );
  void slotSetMode( DisplayMode, KraftDoc::Part p);
  void slotShowPrices( bool showIt );

protected:
  QString renderDocMini(KraftDoc::Part p) const;
  QString renderDocFull( KraftDoc::Part p);
  QString header(bool, const QString&, const QString&, const QString& protocol,
                  const QString& = QString() ) const;

private slots:
  void slotUrlSelected( const QUrl& kurl);

private:
  QString htmlify( const QString& ) const;

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
  bool mShowPrices;
};

#endif

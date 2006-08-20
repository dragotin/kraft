/***************************************************************************
       docoverviewwidget  - A widget to show an overview at top of the
                    document editor window
                             -------------------
    begin                : 2006-08-12
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

#ifndef DOCOVERVIEWWIDGET_H
#define DOCOVERVIEWWIDGET_H


// include files
#include <qvbox.h>

#include "kraftdoc.h"

class QLabel;
class QButtonGroup;
/**
 *
 */
class DocOverviewWidget : public QVBox
{
  Q_OBJECT

public:
  enum PageId { HeaderId = 0, PositionId, FooterId };
  DocOverviewWidget( QWidget* );
  ~DocOverviewWidget();

  void setDocPtr( DocGuardedPtr doc );

public slots:
  void slotSelectPageButton( int );
  void slotSetSums(  Geld, double );
signals:
  void switchToPage( int );

private:
  QLabel *mDocShort;
  QLabel *mNettoSum;
  QLabel *mVatLabel;
  QLabel *mVat;
  QLabel *mBrutto;
  DocGuardedPtr mDoc;
  QButtonGroup *mButtonGroup;
};

#endif

/* END */


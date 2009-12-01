/***************************************************************************
      dbinitdialog.h  - Dialog to init and update the database schema
                             -------------------
    begin                : Tue Nov 24 2009
    copyright            : (C) 2009 by Klaas Freitag
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
#ifndef DBINITDIALOG_H
#define DBINITDIALOG_H

#include <QObject>
#include <QWidget>
#include <kdialog.h>

#include "ui_dbinit.h"

class DbInitDialog : public KDialog, protected Ui::dbInitWidget
{
  Q_OBJECT

public:
  DbInitDialog( QWidget *parent = 0 );

public slots:

  void slotSetStatusText( const QString& );
  void slotProcessedOneCommand( bool );
  void setOverallCount( int );
  void setCurrentOverallCount( int );
  void setDetailOverallCnt( int );
  void setCurrentDetailCount( int );

};

#endif // DBINITDIALOG_H

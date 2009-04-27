/***************************************************************************
                   numbercycledialog.h  - edit number cycles
                             -------------------
    begin                : Jan 15 2009
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

#ifndef NUMBERCYCLEDIALOG_H
#define NUMBERCYCLEDIALOG_H

#include <kdialogbase.h>

#include <qmap.h>

#include "dbids.h"
#include "numbercycle.h"
#include "numbercycleseditbase.h"

class QLineEdit;
class QLabel;
class QTextEdit;
class QPushButton;
class QComboBox;
class QCheckBox;


/**
 *  @author Klaas Freitag
 */

// ################################################################################

class NumberCycleDialog: public KDialogBase
{
  Q_OBJECT

public:
  NumberCycleDialog( QWidget *parent, const QString& initType = QString() );

public slots:

protected slots:

  void slotAddCycle();
  void slotRemoveCycle();
  void slotNumberCycleSelected( int );
  void slotTemplTextChanged( const QString& );
  void slotOk();
  void slotUpdateExample();

private:
  void updateField( int, const QString&, const QString& );
  void loadCycles();
  void updateCycleDataFromGUI();
  bool dropOfNumberCycleOk( const QString& );

  NumberCycleEditBase *mBaseWidget;
  QStringList mRemovedCycles;
  QMap<QString,NumberCycle> mNumberCycles;
  QString mSelectedCycle;
  
};

#endif
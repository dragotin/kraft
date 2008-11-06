/***************************************************************************
  importitemdialog.h  - small dialog to import items into the document
                             -------------------
    begin                : Nov 2008
    copyright            : (C) 2008 Klaas Freitag
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
#ifndef IMPORTITEMDIALOG_H
#define IMPORTITEMDIALOG_H

#include <kdialogbase.h>
#include <qmap.h>

#include "templtopositiondialogbase.h"
#include "docposition.h"

class importToDocBase;

class ImportItemDialog: public KDialogBase
{
  Q_OBJECT

public:
  ImportItemDialog( QWidget* );
  ~ImportItemDialog();

  void setPositionList( DocPositionList, int );
  
protected:
  QComboBox *getPositionCombo();

private:
  importToDocBase *mBaseWidget;

  QMap<int, QString> mTagMap;
};

#endif

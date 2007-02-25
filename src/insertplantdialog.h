/***************************************************************************
 insertplantdialog.h  - small dialog to insert plants into documents
                             -------------------
    begin                : Feb 2007
    copyright            : (C) 2007 Klaas Freitag
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
#ifndef INSERTPLANTDIALOG_H
#define INSERTPLANTDIALOG_H

#include <qmap.h>
#include <kdialogbase.h>

#include "brunsrecord.h"
#include "templtopositiondialogbase.h"

class insertPlantBase;
class BrunsRecord;
class BrunsSize;
class DocPosition;
class KListViewItem;

class InsertPlantDialog: public TemplToPositionDialogBase
{
    Q_OBJECT

public:
    InsertPlantDialog( QWidget* );
  ~InsertPlantDialog();

  void setSelectedPlant( BrunsRecord* );

  void setDocPosition( DocPosition* );

  DocPosition docPosition();

protected slots:
  void slotSizeListSelectionChanged();

protected:
  QComboBox *getPositionCombo();

private:
  insertPlantBase *mBaseWidget;
  QMap<KListViewItem*, QString> mSizeMap;
  QString mLtName;
  QString mDtName;

};

#endif

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
#include <qsqlquery.h>
#include <qdatetime.h>

#include <kdialog.h>

#include "brunsrecord.h"
#include "templtopositiondialogbase.h"
#include "geld.h"
#include "ui_insertplantbase.h"

class insertPlantBase;
class BrunsRecord;
class BrunsSize;
class DocPosition;
class QTreeViewItem;
class Geld;


class PlantPriceInfo
{
public:
  PlantPriceInfo( double price, QDate date )
    : mPrice( price ), mLastUpdateDate( date ) {
    
  }
  PlantPriceInfo() {

  }

  Geld price() {
    return mPrice;
  }

  QDate lastUpdateDate() {
    return mLastUpdateDate;
  }

private:
  Geld  mPrice;
  QDate mLastUpdateDate;
};


class InsertPlantDialog: public TemplToPositionDialogBase
{
    Q_OBJECT

public:
    InsertPlantDialog( QWidget* );
  ~InsertPlantDialog();

  void setSelectedPlant( BrunsRecord* );

  void setDocPosition( DocPosition*, bool, bool );

  DocPosition docPosition();

  void setCatalogChapters( const QList<CatalogChapter>& );
  QString chapter() const;

protected slots:
  void slotSizeListSelectionChanged();
  void slotOk();

protected:
  QComboBox *getPositionCombo();

private:
  PlantPriceInfo getPriceInfo( const QString& );
  void setPrice( const QString&, double );

  Ui::insertPlantBase *mBaseWidget;
  QMap<QTreeWidgetItem*, QString> mSizeMap;
  QMap<QTreeWidgetItem*, PlantPriceInfo> mPriceMap;
  QString mLtName;
  QString mDtName;
  QSqlQuery mPriceQuery;
};

#endif

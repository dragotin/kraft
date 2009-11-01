/***************************************************************************
              postiontagdialog.h  - Edit tags of positions
                             -------------------
    begin                : Aug 2008
    copyright            : (C) 2008 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *
 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef POSITIONTAGDIALOG_H
#define POSITIONTAGDIALOG_H

#include <QMap>

#include <kdialog.h>

class QWidget;
class QStringList;
class QTreeWidget;
class QTreeWidgetItem;

class FilterHeader;

class PositionTagDialog: public KDialog
{
  Q_OBJECT

public:
  PositionTagDialog( QWidget* );
  virtual ~PositionTagDialog( );

  void setPositionTags( const QStringList& checkedTags);
  QStringList getSelectedTags();

private:
  QTreeWidget* mListView;
  FilterHeader *mFilterHeader;
  QMap<QString, QTreeWidgetItem*> mItemMap;
};

#endif

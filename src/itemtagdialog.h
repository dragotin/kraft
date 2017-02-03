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


#ifndef ITEMTAGDIALOG_H
#define ITEMTAGDIALOG_H

#include <QMap>

#include <QDialog>

class QWidget;
class QStringList;
class QTreeWidget;
class QTreeWidgetItem;

class ItemTagDialog: public QDialog
{
  Q_OBJECT

public:
  ItemTagDialog( QWidget* );
  virtual ~ItemTagDialog( );

  void setPositionTags( const QStringList& checkedTags);
  QStringList getSelectedTags();

private:
  QTreeWidget* mListView;
  QMap<QString, QTreeWidgetItem*> mItemMap;
};

#endif

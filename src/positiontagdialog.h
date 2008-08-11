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

#include <qmap.h>

#include <kdialogbase.h>

class QWidget;
class QStringList;
class QListViewItem;
class KListView;
class FilterHeader;


class PositionTagDialog: public KDialogBase
{
  Q_OBJECT

public:
  PositionTagDialog( QWidget* );
  ~PositionTagDialog( );

  void setTags( const QStringList& );
  void setPositionTags( const QStringList& );
  QStringList getSelectedTags();

private:
  KListView *mListView;
  FilterHeader *mFilterHeader;
  QMap<QString, QCheckListItem*> mItemMap;
};

#endif

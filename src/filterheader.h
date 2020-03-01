/***************************************************************************
                            filterheader.h
                             -------------------
    copyright            : (C) 2005 by Cornelius Schumacher
                           (C) 2005 by Klaas Freitag
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

#ifndef FILTERHEADER_H
#define FILTERHEADER_H

#include <QWidget>
#include <QLineEdit>
#include <QTreeWidgetItem>

#include "kraftcat_export.h"

class QTreeWidget;
class QLabel;
class QString;


class KRAFTCAT_EXPORT FilterHeader : public QWidget
{
    Q_OBJECT
  public:
    FilterHeader(QWidget *parent = 0, QTreeWidget *tree = 0);

  public slots:
    void clear();
    void setListView( QTreeWidget* );

private slots:
    void slotTextChanged( const QString& filter );

  private:
    QLineEdit   *mSearchLine;
    QLabel      *mTitleLabel;
    QTreeWidget *_treeWidget;
    QHash<QTreeWidgetItem*, int> _openedItems;
};

#endif

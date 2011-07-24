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

#include "kraftcat_export.h"

#include <ktreewidgetsearchline.h>

class QTreeWidget;
class QLabel;
class QString;

class KRAFTCAT_EXPORT CountingSearchLine : public KTreeWidgetSearchLine
{
    Q_OBJECT
  public:
    CountingSearchLine( QWidget *parent, QTreeWidget *listView );
    CountingSearchLine( QWidget *parent, const QList< QTreeWidget * > &treeWidgets );
    int searchCount();

  protected:
    void searchUpdate( const QString &s = QString::null );

  signals:
    void searchCountChanged();
};


class KRAFTCAT_EXPORT FilterHeader : public QWidget
{
    Q_OBJECT
  public:
    FilterHeader( QTreeWidget *tree, QWidget *parent = 0 );
    FilterHeader( QList<QTreeWidget *> &treewidgets, QWidget *parent = 0);

    void setItemName( const QString &none, const QString &one,
    const QString &multiple );

    void showCount( bool );

  public slots:
    void setTitleLabel();
    void clear();
    void setListView( QTreeWidget*  );

  private:
    CountingSearchLine *mSearchLine;
    QLabel *mTitleLabel;

    QString mItemNameNone;
    QString mItemNameOne;
    QString mItemNameMultiple;
};

#endif

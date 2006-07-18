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

#include <klistviewsearchline.h>

#include <qwidget.h>

class KListView;
class QLabel;

class CountingSearchLine : public KListViewSearchLine
{
    Q_OBJECT
  public:
    CountingSearchLine( QWidget *parent, KListView *listView );

    void updateSearch( const QString &s = QString::null );
    
    int searchCount();
    
  signals:
    void searchCountChanged();
};


class FilterHeader : public QWidget
{
    Q_OBJECT
  public:
    FilterHeader( KListView *, QWidget *parent );

    void setItemName( const QString &none, const QString &one,
      const QString &multiple );

    void showCount( bool );

  public slots:
    void setTitleLabel();
    void clear();

  private:
    KListView *mListView;
    
    CountingSearchLine *mSearchLine;
    QLabel *mTitleLabel;
    
    QString mItemNameNone;
    QString mItemNameOne;
    QString mItemNameMultiple;
};

#endif

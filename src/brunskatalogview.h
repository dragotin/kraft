/***************************************************************************
                          brunskatalogview.h
                             -------------------
    begin                : 2005-07-26
    copyright            : (C) 2005 by Klaas Freitag
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
#ifndef BRUNSKATALOGVIEW_H
#define BRUNSKATALOGVIEW_H

#include <katalogview.h>

#include "brunskataloglistview.h"

#include <QLabel>


class QBoxLayout;
class BrunsKatalogListView;
class QLabel;

/**
@author Klaas Freitag
*/
class KRAFTCAT_EXPORT BrunsKatalogView : public KatalogView
{
    Q_OBJECT
public:
    BrunsKatalogView();

    virtual ~BrunsKatalogView();

    void createCentralWidget(QBoxLayout*, QWidget *w);
    KatalogListView* getListView() { return m_brunsListView; }

protected slots:
    void slPlantSelected( QTreeWidgetItem*, QTreeWidgetItem*);
    
protected:
    Katalog* getKatalog( const QString& );
    
    BrunsKatalogListView *m_brunsListView;
    QLabel               *m_detailLabel;
    QTreeWidget          *m_details;
};

#endif

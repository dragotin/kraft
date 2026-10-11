/***************************************************************************
         kraftdocpositionsedit.h - Doc item editor widget
                             -------------------
    begin                :
    copyright            : (C) 2003 by Klaas Freitag
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

#ifndef KRAFTDOCPOSITIONSEDIT_H
#define KRAFTDOCPOSITIONSEDIT_H

#include "kraftdocedit.h"

#include <QScrollArea>

class KraftViewScroll;

class PositionViewWidget;

class KraftViewScroll : public QScrollArea
{
    Q_OBJECT

public:
    KraftViewScroll( QWidget* );
    ~KraftViewScroll();

    void addChild( QWidget *child, int index );
    void showHelpWidget( QWidget *help );
    void hideHelpWidget();
    void removeChild( PositionViewWidget *child );
    void moveChild( PositionViewWidget *child, int index);
    int indexOf( PositionViewWidget *child);

    void setMyWidget(QWidget *newMyWidget);

private:
    QWidget *_myWidget;
    QVBoxLayout *_layout;
};

// ###########################################################################

class KraftDocPositionsEdit : public KraftDocEdit
{
    Q_OBJECT
public:
    KraftDocPositionsEdit( QWidget* );

    // FIXME: Remove access to internal widgets
    KraftViewScroll *positionScroll() { return _positionScroll; }

    void setDiscountButtonVisible( bool visible );

Q_SIGNALS:
    void addPositionClicked();
    void addExtraClicked();

private:
    KraftViewScroll *_positionScroll;
    QPushButton *_discountBtn;

};

#endif


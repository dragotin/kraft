/***************************************************************************
        postionviewwidget - inherited class for doc position views.
                             -------------------
    begin                : 2006-02-20
    copyright            : (C) 2006 by Klaas Freitag
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


#ifndef POSITIONVIEWWIDGET_H
#define POSITIONVIEWWIDGET_H

#include <QMap>
#include <QMenu>
#include <QAction>
#include <QPaintEvent>
#include <QObject>

#include "geld.h"
#include "ui_positionwidget.h"
#include "docposition.h"

/**
	@author Klaas Freitag <freitag@kde.org>
*/
class KMenu;
class QAction;
class Geld;
class QLocale;
class DosPositionGuardedPtr;

class PositionViewWidget : public QWidget, public Ui_positionWidget
{
    Q_OBJECT
public:
    enum State { Active, New, Deleted, Locked };

    PositionViewWidget( );
    PositionViewWidget( int );

    void setDocPosition(DocPosition*pos);
    virtual ~PositionViewWidget();
    bool modified() { return mModified; }
    int ordNumber() { return mOrdNumber; }
    void setOrdNumber( int  );

    bool deleted() { return mToDelete; }
    DocPositionGuardedPtr position(){ return mPositionPtr; }
    State state() { return mState; }
    DocPosition::Type  kind()  { return mKind; }

    static QString techKindString(DocPosition::Type kind);
    static DocPosition::Type techStringToKind( const QString& kindStr );
    static QString kindLabel(DocPosition::Type);

    QString stateString( const State& state ) const;
    QString cleanKindString(const QString &src);
    Geld currentPrice();
    bool priceValid();
    void setCurrentPrice( Geld );
    Geld unitPrice();
    QString extraDiscountTagRestriction();
    DocPosition::Tax taxType() const;

public Q_SLOTS:
    void slotSetOverallPrice( Geld );
    void slotRefreshPrice();
    void slotModified();
    void slotExecButtonPressed();
    void slotTaggingButtonPressed();
    void slotMenuAboutToHide();
    void slotMenuAboutToShow();
    void slotSetState( State );
    void slotSetEnabled( bool );
    void slotEnableKindMenu( bool );
    void slotAllowIndividualTax( bool );
    void slotSetTax( DocPosition::Tax);
    void slotShowPrice( bool show );  // hide the price entries for certain doc types.

protected Q_SLOTS:
    void slotLockPosition();
    void slotUnlockPosition();
    void slotSetPositionKind(DocPosition::Type kind, bool alterText);
    void slotUpdateTagToolTip();
    void paintEvent ( QPaintEvent* );

    void slotSetNilTax();
    void slotSetReducedTax();
    void slotSetFullTax();

Q_SIGNALS:
    void positionModified();
    void deletePosition();
    void moveUp();
    void moveDown();
    void lockPosition();
    void unlockPosition();
    void priceChanged( const Geld& );
    void positionStateNormal();
    void positionStateAlternative();
    void positionStateDemand();

private:
    bool mModified;
    bool mToDelete;
    int  mOrdNumber;

    DocPositionGuardedPtr mPositionPtr;
    QMenu *mExecPopup;
    QMenu *mStateSubmenu;
    QMenu *mTaxSubmenu;

    QAction * mDeleteId;
    QAction * mLockId;
    QAction * mUnlockId;
    QAction * mNilTaxAction;
    QAction * mRedTaxAction;
    QAction * mFullTaxAction;

    Geld mPositionPrice;  // only used for Discount items to store the result
    State mState;
    DocPosition::Type  mKind;
    bool mPositionPriceValid;
    QLocale *mLocale;
    DocPosition::Tax mTax;
};

class PositionViewWidgetList : public QList<PositionViewWidget*>
{
  public:
    PositionViewWidgetList();
    PositionViewWidget* widgetFromPosition( DocPositionGuardedPtr );

    Geld nettoPrice();
};


typedef QListIterator<PositionViewWidget*> PositionViewWidgetListIterator;

#endif

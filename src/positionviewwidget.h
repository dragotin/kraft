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
    enum Kind  { Normal, Demand, Alternative, Invalid };

    PositionViewWidget( );
    PositionViewWidget( int );

    void setDocPosition(DocPositionBase*);
    virtual ~PositionViewWidget();
    bool modified() { return mModified; }
    int ordNumber() { return mOrdNumber; }
    void setOrdNumber( int  );

    bool deleted() { return mToDelete; }
    DocPositionGuardedPtr position(){ return mPositionPtr; }
    State state() { return mState; }
    Kind  kind()  { return mKind; }
    QString kindString( Kind = Invalid ) const;
    QString stateString( const State& state ) const;
    QString kindLabel( Kind ) const;
    void cleanKindString();
    Geld currentPrice();
    bool priceValid();
    void setCurrentPrice( Geld );
    Geld unitPrice();
    QStringList tagList() { return mTags; }
    QString extraDiscountTagRestriction();
    DocPositionBase::TaxType taxType() const;

public slots:
    void slotSetOverallPrice( Geld );
    void slotRefreshPrice();
    void slotModified( bool emitSignal = true );
    void slotExecButtonPressed();
    void slotTaggingButtonPressed();
    void slotMenuAboutToHide();
    void slotMenuAboutToShow();
    void slotSetState( State );
    void slotSetEnabled( bool );
    void slotEnableKindMenu( bool );
    void slotAllowIndividualTax( bool );
    void slotSetTax( DocPosition::TaxType );
    void slotShowPrice( bool show );  // hide the price entries for certain doc types.

protected slots:
    void slotLockPosition();
    void slotUnlockPosition();
    void slotSetPositionNormal();
    void slotSetPositionAlternative();
    void slotSetPositionDemand();
    void slotUpdateTagToolTip();
    void paintEvent ( QPaintEvent* );

    void slotSetNilTax();
    void slotSetReducedTax();
    void slotSetFullTax();

signals:
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
    bool m_skipModifiedSignal;
    bool mToDelete;
    int  mOrdNumber;

    DocPositionGuardedPtr mPositionPtr;
    QMenu *mExecPopup;
    QMenu *mStateSubmenu;
    QMenu *mTaxSubmenu;

    QStringList mTags;
    QAction * mDeleteId;
    QAction * mLockId;
    QAction * mUnlockId;
    QAction * mNilTaxAction;
    QAction * mRedTaxAction;
    QAction * mFullTaxAction;

    Geld mPositionPrice;  // only used for Discount items to store the result
    State mState;
    Kind  mKind;
    bool mPositionPriceValid;
    QLocale *mLocale;
    DocPosition::TaxType mTax;
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

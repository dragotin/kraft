/***************************************************************************
        positionviewwidget - inherited class for doc position views.
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

#include <qptrlist.h>

#include "positionwidget.h"

/**
	@author Klaas Freitag <freitag@kde.org>
*/
class DocPosition;
class KPopupMenu;
class Geld;

class PositionViewWidget : public positionWidget
{
    Q_OBJECT
public:
    enum State { Active, New, Deleted, Locked };
    PositionViewWidget();
    PositionViewWidget( int );
    void setDocPosition( DocPositionBase* );
    ~PositionViewWidget();
    bool modified() { return mModified; }
    int ordNumber() { return mOrdNumber; }
    void setOrdNumber( int  );
    bool deleted() { return mToDelete; }
    DocPositionGuardedPtr position(){ return mPositionPtr; }
    State state() { return mState; }
    QString stateString( const State& state ) const;
    Geld currentPrice();

public slots:
    void slotSetOverallPrice( Geld );
    void slotRefreshPrice();
    void slotModified();
    void slotExecButtonPressed();
    void slotMenuAboutToHide();
    void slotMenuAboutToShow();
    void slotSetState( State );
    void slotSetEnabled( bool );

protected slots:
    void slotLockPosition();
    void slotUnlockPosition();
    
    void slotSetPositionNormal();
    void slotSetPositionAlternative();
    void slotSetPositionDemand();

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
    KPopupMenu *mExecPopup;
    int  mDeleteId;
    int  mLockId;
    int  mUnlockId;
    State mState;
};

class PositionViewWidgetList : public QPtrList<PositionViewWidget>
{
  public:
    PositionViewWidgetList();
    PositionViewWidget* widgetFromPosition( DocPositionGuardedPtr );

    Geld nettoPrice();
};

#endif

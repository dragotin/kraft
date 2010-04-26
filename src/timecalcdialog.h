/***************************************************************************
             Timecalcdialog  -
                             -------------------
    begin                : 2004-23-09
    copyright            : (C) 2004 by Klaas Freitag
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

#ifndef _TimeCALCDIALOG_H
#define _TimeCALCDIALOG_H

// include files
#include "ui_timepart.h"  // designer file Timepartui.ui

/**
 *
 */

class TimeCalcPart;

class TimeCalcDialog : public KDialog, protected Ui::calcdetailTime
{
    Q_OBJECT

public:
    TimeCalcDialog(QWidget *parent=0, bool modal=FALSE );
    TimeCalcDialog(TimeCalcPart*, QWidget *parent=0, bool modal=FALSE );
    virtual ~TimeCalcDialog();

    QString getName();
    QString getStundensatzName();
    int     getDauer();
    bool    allowGlobal();
signals:
    void timeCalcPartChanged(TimeCalcPart*);
protected slots:
    void accept();

private:
    TimeCalcPart *m_part;
};

#endif

/* END */


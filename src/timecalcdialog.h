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
#include "calcdialogbase.h"


#include "ui_timepart.h"

/**
 *
 */

class TimeCalcPart;

class TimeCalcDialog : public CalcDialogBase
{
    Q_OBJECT

public:
    TimeCalcDialog(QWidget *parent=0);

    void setTimeCalcPart(TimeCalcPart *cp);
    QString getName();
    QString getStundensatzName();
    int     getDauer();
    bool    allowGlobal();
    QString unitStr() const;
signals:
    void timeCalcPartChanged(TimeCalcPart*);
protected slots:
    void accept();

private:
    Ui_calcdetailTime *_timeWidget;
    TimeCalcPart      *_part;
};

#endif

/* END */


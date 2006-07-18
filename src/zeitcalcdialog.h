/***************************************************************************
             zeitcalcdialog  -
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

#ifndef _ZEITCALCDIALOG_H
#define _ZEITCALCDIALOG_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files
#include "zeitpartui.h"  // designer file zeitpartui.ui

/**
 *
 */

class ZeitCalcPart;

class ZeitCalcDialog : public calcdetailTime
{
    Q_OBJECT

public:
    ZeitCalcDialog(QWidget *parent=0, const char* name=0, bool modal=FALSE );
    ZeitCalcDialog(ZeitCalcPart*, QWidget *parent=0, const char* name=0, bool modal=FALSE );
    ~ZeitCalcDialog();

    QString getName();
    QString getStundensatzName();
    int     getDauer();
    bool    allowGlobal();
signals:
    void timeCalcPartChanged(ZeitCalcPart*);
protected slots:
    void accept();

private:
    ZeitCalcPart *m_part;
};

#endif

/* END */


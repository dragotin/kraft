/***************************************************************************
             fixcalcdialog  -
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

#ifndef _FIXCALCDIALOG_H
#define _FIXCALCDIALOG_H

#include <QDialog>

#include "ui_fixpartui.h"  // designer file zeitpartui.ui

/**
 *
 */

class FixCalcPart;

class FixCalcDialog : public QDialog, protected Ui::calcdetailFix
{
    Q_OBJECT

public:
    FixCalcDialog(QWidget *parent=0, bool modal=false );
    FixCalcDialog(FixCalcPart*, QWidget *parent=0, bool modal=false );
    virtual ~FixCalcDialog();

    QString getName();
    double  getMenge();
    double  getPreis();
    void setCalcPart( FixCalcPart* );

signals:
    void fixCalcPartChanged(FixCalcPart*);

protected slots:
    void accept();
private:

    FixCalcPart *m_part;
};

#endif

/* END */


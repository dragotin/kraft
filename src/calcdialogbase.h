/***************************************************************************
         CalcDialogBase  - base class for calculation detail dialogs
                             -------------------
    begin                : 2017-01-31
    copyright            : (C) 2017 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CALCDIALOGBASE_H
#define _CALCDIALOGBASE_H

#include <QDialog>
#include <QWidget>

/**
 *
 */

class CalcDialogBase: public QDialog
{
    Q_OBJECT

public:
    CalcDialogBase(QWidget *parent);

protected:
    QWidget *_centralWidget;
};

#endif

/* END */

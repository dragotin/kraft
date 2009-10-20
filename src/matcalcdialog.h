/***************************************************************************
             matcalcdialog  -
                             -------------------
    begin                : 2005-03-00
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

#ifndef _MATCALCDIALOG_H
#define _MATCALCDIALOG_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files
#include <kdialog.h>

#include "ui_matpartui.h"

class StockMaterial;
/**
 *
 */

class MatCalcDialog : public KDialog, protected Ui::calcdetailMat
{
    Q_OBJECT

public:
    MatCalcDialog( StockMaterial *mat=0, QWidget *parent=0, bool modal=FALSE );
    MatCalcDialog( double amount, StockMaterial *mat, QWidget *parent=0, bool modal=FALSE );
    virtual ~MatCalcDialog();

    double getAmount();
protected slots:
    void accept();
    void reject();
    
  signals:
    void matCalcPartChanged(StockMaterial*, double);
private:
    void init(double);
    StockMaterial *m_material;
};

#endif

/* END */


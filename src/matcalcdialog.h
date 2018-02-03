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

// include files
#include <QDialog>

#include "ui_matpartui.h"
#include "calcdialogbase.h"


class StockMaterial;
class MaterialCalcPart;
/**
 *
 */

class MatCalcDialog : public CalcDialogBase
{
    Q_OBJECT

public:
    MatCalcDialog(MaterialCalcPart *mc, QWidget *parent=0);
    virtual ~MatCalcDialog();

protected slots:
    void accept();
    
  signals:
    void matCalcPartChanged(MaterialCalcPart *mc);
private:
    void init(double);
    Ui::calcdetailMat *_matWidget;
    MaterialCalcPart *m_mc;
};

#endif

/* END */


/***************************************************************************
                   taxeditdialog.h  - edit tax rates
                             -------------------
    begin                : Apr 9 2009
    copyright            : (C) 2009 by Klaas Freitag
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

#ifndef TAXEDITDIALOG_H
#define TAXEDITDIALOG_H


#include <qdatetime.h>
#include <qvaluelist.h>

#include <kdialogbase.h>

#include "taxeditbase.h"

class QLabel;

/**
 *  @author Klaas Freitag
 */

// ################################################################################

struct TaxRecord
{
  typedef QValueList<TaxRecord> List;

  double fullTax;
  double reducedTax;
  QDate  date;
};

class TaxEditDialog: public KDialogBase
{
  Q_OBJECT

public:
  TaxEditDialog( QWidget *parent );

  TaxRecord newTaxRecord();

private:
  TaxEditBase *mBaseWidget;
  
};

#endif

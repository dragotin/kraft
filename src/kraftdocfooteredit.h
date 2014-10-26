/***************************************************************************
  kraftdocfooteredit.h  - inherited class from designer generated class
                             -------------------
    begin                : Sept. 2006
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
#ifndef KRAFTDOCFOOTEREDIT_H
#define KRAFTDOCFOOTEREDIT_H

#include "ui_docfooter.h"
#include "kraftdocedit.h"

class KraftDocFooterEdit : public KraftDocEdit
{
    Q_OBJECT
public:
  KraftDocFooterEdit( QWidget *parent=0 );

  // FIXME: Remove access to internal widgets
  Ui::DocFooterEdit *ui() { return mDocFooterEdit; }

  QString greeting();

public slots:
  void slotGreeterIndexChanged(int newIndex);
  void slotGreeterEditTextChanged(const QString& newText);
  void slotSetGreeting( const QString& newText );

private:
  Ui::DocFooterEdit *mDocFooterEdit;
  QString            mGreeting;
  int                mCustomGreetingIndex;
};

#endif


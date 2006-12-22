/***************************************************************************
  kraftdocheaderview.h  - inherited class from designer generated class
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
#ifndef KRAFTDOCHEADEREDIT_H
#define KRAFTDOCHEADEREDIT_H

#include "docheader.h"
#include "kraftdocedit.h"

class KraftDocHeaderEdit : public KraftDocEdit
{
public:
  KraftDocHeaderEdit( QWidget* );

  // FIXME: Remove access to internal widgets
  DocHeaderEdit *docHeaderEdit() { return mDocHeaderEdit; }

private:
  DocHeaderEdit *mDocHeaderEdit;
};

#endif


/***************************************************************************
   kraftdocheaderview.cpp  - inherited class from designer generated class
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

#include "kraftdocheaderedit.h"

KraftDocHeaderEdit::KraftDocHeaderEdit( QWidget *parent )
  :DocHeaderEdit( parent )
{

}

void KraftDocHeaderEdit::slotModified()
{
  emit modified();
}


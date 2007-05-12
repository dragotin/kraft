/***************************************************************************
              texteditdialog.h  - Edit document text templates
                             -------------------
    begin                : Apr 2007
    copyright            : (C) 2007 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *
 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef TEXTEDITDIALOG_H
#define TEXTEDITDIALOG_H

#include <kdialogbase.h>

#include "doctext.h"

class QWidget;
class QComboBox;
class DocText;
class TextEditBase;

class TextEditDialog: public KDialogBase
{
  Q_OBJECT

public:
  TextEditDialog( QWidget* );
  ~TextEditDialog( );

  virtual void setDocText( DocText );
  DocText docText();

private:
  TextEditBase *mBaseWidget;
  DocText       mOriginalText;
};

#endif

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

#include <QDialog>

#include "ui_texteditbase.h"

#include "doctext.h"
#include "kraftdoc.h"

class QWidget;
class QComboBox;
class DocText;
class TextEditBase;

class TextEditDialog: public QDialog
{
  Q_OBJECT

public:
  TextEditDialog( QWidget*, KraftDoc::Part );
  ~TextEditDialog( );

  virtual void setDocText( DocText );
  DocText docText();

private:
  Ui::TextEditBase *mBaseWidget;
  DocText       mOriginalText;
};

#endif

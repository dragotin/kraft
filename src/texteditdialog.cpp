/***************************************************************************
              texteditdialog.cpp  - Edit document text templates
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

#include <qcombobox.h>
#include <qwidget.h>
#include <qvbox.h>

#include <kdialogbase.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <ktextedit.h>
#include <klineedit.h>

#include "templtopositiondialogbase.h"
#include "texteditdialog.h"
#include "texteditbase.h"
#include "doctext.h"
#include "defaultprovider.h"


TextEditDialog::TextEditDialog( QWidget *parent )
  : KDialogBase( parent, "TEMPL_DIALOG", true, i18n( "Edit Text Templates" ),
                 Ok | Cancel )
{
  QWidget *w = makeVBoxMainWidget();
  mBaseWidget = new TextEditBase( w );

  mBaseWidget->mCbDocType->insertStringList( DefaultProvider::self()->docTypes() );

  QStringList strList;
  strList << DocText::textTypeToString( KraftDoc::Header ) <<
    DocText::textTypeToString( KraftDoc::Footer );
  mBaseWidget->mCbTextType->insertStringList( strList );
}

TextEditDialog::~TextEditDialog()
{

}

void TextEditDialog::setDocText( DocText dt )
{
  QString name = i18n( "Template" );
  if ( ! dt.name().isEmpty() ) {
    name = dt.name();
  }
  mBaseWidget->mEditName->setText( name );

  mBaseWidget->mEditDescription->setText( dt.description() );
  mBaseWidget->mEditText->setText( dt.text() );

  mBaseWidget->mCbTextType->setCurrentText( dt.textTypeString() );
  mBaseWidget->mCbDocType->setCurrentText( dt.docType() );

  mOriginalText = dt;
}

DocText TextEditDialog::docText()
{
  DocText dt;
  dt = mOriginalText;

  dt.setName( mBaseWidget->mEditName->text() );
  dt.setDescription( mBaseWidget->mEditDescription->text() );
  dt.setText( mBaseWidget->mEditText->text() );
  dt.setDocType( mBaseWidget->mCbDocType->currentText() );
  dt.setTextType( DocText::stringToTextType( mBaseWidget->mCbTextType->currentText() ) );

  return dt;
}

#include "texteditdialog.moc"

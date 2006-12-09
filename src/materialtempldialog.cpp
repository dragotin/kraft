/***************************************************************************
             materialtempldialog  - dialog to edit material templates
                             -------------------
    begin                : 2006-12-03
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

// include files for Qt
#include <qcombobox.h>
#include <qtextedit.h>
#include <qstring.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include "materialtempldialog.h"
#include "katalogman.h"
#include "unitmanager.h"
#include "geld.h"

MaterialTemplDialog::MaterialTemplDialog( QWidget *parent, const char* name, bool modal, WFlags fl)
    : MaterialDialogBase(parent, name, modal, fl)
{
    /* connect a value Changed signal of the manual price field */
}

void MaterialTemplDialog::setMaterial( StockMaterial *t, const QString& katalogname, bool newTempl )
{
  if( ! t ) return;
  mSaveMaterial = t;

  m_templateIsNew = newTempl;

  m_katalog = KatalogMan::self()->getKatalog(katalogname);

  if( m_katalog == 0 ) {
    kdDebug() << "ERR: Floskel Dialog called without valid Katalog!" << endl;
    return;
  }

  mCbChapter->insertStringList( m_katalog->getKatalogChapters() );
  int chapID = t->chapter();
  QString chap = m_katalog->chapterName(dbID(chapID));
  mCbChapter->setCurrentText(chap);

}


MaterialTemplDialog::~MaterialTemplDialog( )
{
}

void MaterialTemplDialog::accept()
{
    kdDebug() << "*** Saving finished " << endl;
    mSaveMaterial->setName( mEditMaterial->text() );
    mSaveMaterial->setAmountPerPack( mDiPerPack->value() );

    const QString str = mCbUnit->currentText();

    mSaveMaterial->setUnit( UnitManager::getUnitIDSingular( str ) );

    const QString str2 = mCbChapter->currentText();
    int chapId = m_katalog->chapterID( str2 );
    if ( !templateIsNew() && chapId != mSaveMaterial->chapter() ) {
      if( askChapterChange( mSaveMaterial, chapId )) {
        mSaveMaterial->setChapter( chapId );
        emit( chapterChanged( chapId ));
      }
    }

    mSaveMaterial->setChapter( m_katalog->chapterID( str2 ) );

    double db = mInPurchasePrice->value();
    mSaveMaterial->setPurchPrice( Geld( db ) );

    db = mInSalePrice->value();
    mSaveMaterial->setSalesPrice( Geld( db ) );

    mSaveMaterial->save();

    emit editAccepted( mSaveMaterial );

    MaterialDialogBase::accept();
}

bool MaterialTemplDialog::askChapterChange( StockMaterial*, int )
{
    if( KMessageBox::questionYesNo( this,
        i18n( "The catalog chapter was changed for this template.\nDo you really want to move the template to the new chapter?"),
        i18n("Changed Chapter"), KStdGuiItem::yes(), KStdGuiItem::no(),
        "chapterchange" ) == KMessageBox::Yes )
    {
        return true;

    } else {
        return false;
    }
}

void MaterialTemplDialog::reject()
{
    if ( m_templateIsNew ) {
      // remove the listview item if it was created newly
      emit editRejected();
    }
    MaterialDialogBase::reject();
}


#include "materialtempldialog.moc"

/***************************************************************************
                     extendedcombo - a better combo box
                             -------------------
    begin                : April 2008
    copyright            : (C) 2008 by Klaas Freitag
                           (C) 2007, 2008 by Andre Duffeck
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

#include <qlistbox.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kcombobox.h>

#include "extendedcombo.h"


ExtendedComboItem::ExtendedComboItem( const QString &name, const QString &description, ExtendedCombo *combo )
  : QListBoxItem( combo->listBox() ), mDescription( description )
{
  setText( name );
}

int ExtendedComboItem::width(const QListBox *lb) const
{
  QFontMetrics descFm( descriptionFont() );
  return QMAX( lb->fontMetrics().width(text()), descFm.width(mDescription) + 4 ) + 6;
}

int ExtendedComboItem::height(const QListBox *lb) const
{
  QFontMetrics fm(lb->fontMetrics());
  QFontMetrics descFm(  this->descriptionFont() );

  return fm.lineSpacing()+descFm.lineSpacing() + 4;
}

QFont ExtendedComboItem::descriptionFont() const
{
  QFont f;
  if ( listBox() ) {
    f = listBox()->font();
    // kdDebug() << "Pointsize is " << f.pointSize() << endl;
    // decrease font size by 25%
    f.setPointSize( f.pointSize()- f.pointSize()/4 );
  }
  return f;
}

void ExtendedComboItem::paint(QPainter *p)
{
  // evil trick: find out whether we are painted onto our listbox
  bool in_list_box = listBox() && listBox()->viewport() == p->device();

  QFontMetrics fm(p->fontMetrics());
  if( in_list_box ) {
    QFont descFont = descriptionFont();
    QFontMetrics descFm(  descriptionFont() );
    p->drawLine( 0, 0, listBox()->width(), 0 );
    // paint the pixmap
    int pixOffset = 0;
    if ( mPixmap.width() && mPixmap.height() ) {
      int y = ( fm.height()-mPixmap.height() );
      if ( mPixmap.height() > fm.height() ) y = 0;
      p->drawPixmap( 3, y , mPixmap );
      pixOffset = mPixmap.width()+3;
    }
    p->drawText(pixOffset + 3, 2 + fm.ascent() + fm.leading() / 2, text());

    // drawing the description
    p->setFont( descFont );
    p->setPen( Qt::darkGray );
    p->drawText(13, 4 + descFm.ascent() + descFm.leading() / 2 + descFm.lineSpacing(), mDescription);
  } else {
    int pixOffset = 0;

    if ( mPixmap.width() && mPixmap.height() ) {
      int y = 15; // ( listBox().height()-mPixmap.height() );
      if ( y  < 0 ) y = 0;
      p->drawPixmap( 3, y , mPixmap );
      pixOffset = mPixmap.width()+3;
    }
    p->drawText(pixOffset+3, 0, width(listBox())-3, height(listBox()), Qt::AlignLeft | Qt::AlignVCenter, text());
  }
}

void ExtendedComboItem::setPixmap( const QPixmap& pix )
{
  mPixmap = pix;
}
// ################################################################################


ExtendedCombo::ExtendedCombo(QWidget *parent, const char *name)
  : KComboBox( parent, name )
{
}

ExtendedCombo::~ExtendedCombo()
{

}

ExtendedComboItem* ExtendedCombo::insertEntry( const QString &name, const QString &description )
{
  return new ExtendedComboItem( name, description, this );
}


// end



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

  if( in_list_box ) {
    QFontMetrics fm(p->fontMetrics());
    QFont descFont = descriptionFont();
    QFontMetrics descFm(  descriptionFont() );
    p->drawLine( 0, 0, listBox()->width(), 0 );
    p->drawText(3, 2 + fm.ascent() + fm.leading() / 2, text());

    // drawing the description
    p->setFont( descFont );
    p->setPen( Qt::darkGray );
    p->drawText(13, 4 + descFm.ascent() + descFm.leading() / 2 + descFm.lineSpacing(), mDescription);
  } else {
    p->drawText(3, 0, width(listBox())-3, height(listBox()), Qt::AlignLeft | Qt::AlignVCenter, text());
  }
}


// ################################################################################


ExtendedCombo::ExtendedCombo(QWidget *parent, const char *name)
  : KComboBox( parent, name )
{
}

void ExtendedCombo::insertEntry( const QString &name, const QString &description )
{
  new ExtendedComboItem( name, description, this );
}


// end



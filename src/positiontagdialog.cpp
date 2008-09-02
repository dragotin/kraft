/***************************************************************************
              postiontagdialog.h  - Edit tags of positions
                             -------------------
    begin                : Aug 2008
    copyright            : (C) 2008 by Klaas Freitag
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
#include <qlabel.h>
#include <qlistview.h>
#include <qdrawutil.h>
#include <qheader.h>

#include <kdialogbase.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <ktextedit.h>
#include <klineedit.h>

#include "positiontagdialog.h"
#include "defaultprovider.h"
#include "tagman.h"


class TagItem:public QCheckListItem
{

  public:
    TagItem( KListView*, const QString&, QCheckListItem::Type );
    void paintCell ( QPainter*, const QColorGroup&, int, int, int );
    void setColorGroup( QColorGroup );
private:
    QColorGroup mColorGroup;
};


TagItem::TagItem( KListView* view, const QString& name, QCheckListItem::Type t )
  :QCheckListItem( view, name, t )
{

}
void TagItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
  if ( column == 1 ) {
    QBrush b( mColorGroup.mid() );
    qDrawShadeRect( p, 5, 4, width-10, height()-8, mColorGroup, false, 1, 4, &b );
  }  else {
    QCheckListItem::paintCell( p, cg, column, width, align );
  }
}

void TagItem::setColorGroup( QColorGroup cg )
{
  mColorGroup = cg;
}

// ################################################################################

PositionTagDialog::PositionTagDialog( QWidget *parent )
  : KDialogBase( parent, "POSITION_TAG_DIALOG", true, i18n( "Edit Item Tags" ),
                 Ok | Cancel )
{
  QWidget *w = makeVBoxMainWidget();
  ( void ) new QLabel( QString::fromLatin1( "<h2>" )
                       + i18n( "<h2>Item Tags</h2>" ) + QString::fromLatin1( "</h2>" ), w );
  ( void ) new QLabel( i18n( "Select all tags for the item should be tagged with." ), w );

  mListView = new KListView( w );
  mListView->setItemMargin( 3 );
  mListView->setAlternateBackground( QColor( "#dffdd0" ) );
  mListView->header()->hide();
  mListView->setRootIsDecorated( false );
  mListView->setSelectionMode( QListView::Single );
  mListView->addColumn( i18n( "Tag" ) );
  mListView->addColumn( i18n( "Color" ) );
  mListView->addColumn( i18n( "Description" ) );

  mListView->setSelectionMode( QListView::NoSelection );
}

PositionTagDialog::~PositionTagDialog()
{

}

void PositionTagDialog::setTags()
{
  QStringList tags = TagTemplateMan::self()->allTagTemplates();

  for ( QStringList::ConstIterator it = tags.begin(); it != tags.end(); ++it ) {
    TagTemplate templ = TagTemplateMan::self()->getTagTemplate( *it );

    TagItem *item = new TagItem( mListView, templ.name(), QCheckListItem::CheckBox );
    item->setColorGroup( templ.colorGroup() );
    item->setText( 2, templ.description() );

    mItemMap[*it] = item;
  }
}

void PositionTagDialog::setPositionTags( const QStringList& tags )
{
  setTags();
  for ( QStringList::ConstIterator it = tags.begin(); it != tags.end(); ++it ) {
    if ( mItemMap.contains( *it ) ) {
      QCheckListItem *item = mItemMap[*it];
      if( item ) {
        item->setOn( true );
      }
    }
  }
}

QStringList PositionTagDialog::getSelectedTags()
{
  QStringList re;

   QListViewItemIterator it( mListView, QListViewItemIterator::Checked );
   while ( it.current() ) {
     re << ( *it )->text( 0 );
     ++it;
   }
   return re;
}


#include "positiontagdialog.moc"

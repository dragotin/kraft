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

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>
#include <qdrawutil.h>
#include <q3header.h>

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <ktextedit.h>
#include <klineedit.h>
#include <kvbox.h>

#include "positiontagdialog.h"
#include "defaultprovider.h"
#include "tagman.h"


class TagItem:public QTreeWidgetItem
{

  public:
    TagItem( QTreeWidget*, const QString& );
    void setColorGroup( QColorGroup );
private:
    QColorGroup mColorGroup;
};


TagItem::TagItem( QTreeWidget* view, const QString& name )
  :QTreeWidgetItem( view )
{
  // setText( name );
  setFlags( Qt::ItemIsUserCheckable );
  setText( 1, name );
}
#if 0
void TagItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
  if ( column == 1 ) {
    QBrush b( mColorGroup.mid() );
    qDrawShadeRect( p, 5, 4, width-10, height()-8, mColorGroup, false, 1, 4, &b );
  }  else {
    Q3CheckListItem::paintCell( p, cg, column, width, align );
  }
}
#endif
void TagItem::setColorGroup( QColorGroup cg )
{
  mColorGroup = cg;
}

// ################################################################################

PositionTagDialog::PositionTagDialog( QWidget *parent )
  : KDialog( parent )

{
  setObjectName( "POSITION_TAG_DIALOG" );
  setModal( true );
  setCaption( i18n("Edit Item Tags" ) );
  setButtons( KDialog::Ok | KDialog::Cancel );

  KVBox *w = new KVBox( this );
  setMainWidget( w );


  ( void ) new QLabel( QString::fromLatin1( "<h2>" )
                       + i18n( "<h2>Item Tags</h2>" ) + QString::fromLatin1( "</h2>" ), w );
  ( void ) new QLabel( i18n( "Select all tags for the item should be tagged with." ), w );

  mListView = new QTreeWidget( w );
  mListView->setAlternatingRowColors( true );

  // mListView->setItemMargin( 3 );
  // mListView->setAlternateBackground( QColor( "#dffdd0" ) );
  // mListView->header()->hide();
  // mListView->setRootIsDecorated( false );

  mListView->setColumnCount( 3 );
  QStringList headers;
  headers << i18n( "Tag" );
  headers << i18n( "Color" );
  headers << i18n( "Description" );
  mListView->setHeaderLabels( headers );
  mListView->setSelectionMode( QAbstractItemView::NoSelection );
}

PositionTagDialog::~PositionTagDialog()
{

}

void PositionTagDialog::setTags()
{
  QStringList tags = TagTemplateMan::self()->allTagTemplates();

  for ( QStringList::ConstIterator it = tags.begin(); it != tags.end(); ++it ) {
    TagTemplate templ = TagTemplateMan::self()->getTagTemplate( *it );

    TagItem *item = new TagItem( mListView, templ.name() );
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
      QTreeWidgetItem *item = mItemMap[*it];
      if( item ) {
        item->setCheckState( 0, Qt::Checked );
      }
    }
  }
}

QStringList PositionTagDialog::getSelectedTags()
{
  QStringList re;

  QTreeWidgetItem *item = mListView->topLevelItem( 0 );
  while ( item ) {
    if( item->checkState( 0 ) == Qt::Checked ) {
      re << item->text( 0 );
    }
    item = mListView->itemBelow( item );
  }
  return re;
}


#include "positiontagdialog.moc"

/***************************************************************************
  textselection  - widget to select header- and footer text data for the doc
                             -------------------
    begin                : 2007-06-01
    copyright            : (C) 2007 by Klaas Freitag
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
#include "textselection.h"
#include "filterheader.h"
#include "defaultprovider.h"
#include "kraftdoc.h"
#include "doctype.h"
#include "doctext.h"

#include <QLocale>
#include <QDebug>
#include <QDialog>
#include <QAction>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QListView>
#include <QTextEdit>
#include <QLabel>
#include <QStringListModel>
#include <QMenu>

#include <KLocalizedString>

TextSelection::TextSelection( QWidget *parent, KraftDoc::Part part )
  :QWidget( parent ),
    mPart( part )
{
  mGroupBox = new QGroupBox(i18n("Template Collection"));

  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget( mGroupBox );

  /* a view for the entry text repository */
  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->setMargin(0);

  mTextNameView = new QListView;
  vbox->addWidget(mTextNameView);
  mTextNameView->setSelectionMode( QAbstractItemView::SingleSelection );
  mTextNameView->setMaximumHeight(120 );
  mTextNameView->setEditTriggers( QAbstractItemView::NoEditTriggers );

  connect( mTextNameView, SIGNAL(doubleClicked(QModelIndex)),
           this, SIGNAL(editCurrentTemplate()));

  mTextDisplay = new QTextEdit;
  mTextDisplay->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  mTextDisplay->setLineWidth( 1 );
  mTextDisplay->setReadOnly(true);
  QPalette p = mTextDisplay->palette();
  p.setColor( QPalette::Active, QPalette::Base, p.color(QPalette::Window));
  p.setColor( QPalette::Inactive, QPalette::Base, p.color(QPalette::Window));
  mTextDisplay->setPalette(p);
  vbox->addWidget( mTextDisplay, 3 );

  mHelpDisplay = new QLabel;
  mHelpDisplay->setStyleSheet("background-color: #ffcbcb;");
  mHelpDisplay->setAutoFillBackground(true);
  mHelpDisplay->setWordWrap( true );

  QFontMetrics fm( mHelpDisplay->font() );
  int minHeight = 1.5 * fm.height();
  mHelpDisplay->setMinimumHeight( minHeight );
  mHelpDisplay->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
  mHelpDisplay->hide();

  vbox->addWidget( mHelpDisplay );

  mGroupBox->setLayout( vbox );

  mTemplNamesModel = new QStringListModel;
  mTextNameView->setModel( mTemplNamesModel );
  connect( mTextNameView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( slotTemplateNameSelected( const QModelIndex&, const QModelIndex& ) ) );

#if 0
  connect( mTextsView, SIGNAL( currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*) ),
           this, SLOT( slotSelectionChanged( QTreeWidgetItem* ) ) );
  connect( mTextsView, SIGNAL(doubleClicked(QModelIndex) ),
           this, SLOT( slotSelectionChanged( QTreeWidgetItem* ) ) );
#endif

  // Context Menu
  mMenu = new QMenu( this );
  mMenu->setTitle( i18n("Template Actions") );
#if 0
  mTextsView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect( mTextsView, SIGNAL(customContextMenuRequested(QPoint) ),
            this, SLOT( slotRMB( QPoint ) ) );
#endif

  initActions();
}

/* selected the name of a template in the listview of template names */
void TextSelection::slotTemplateNameSelected( const QModelIndex& current, const QModelIndex& )
{
  if( current.isValid() ) {
    mCurrTemplateName = mTemplNamesModel->data( current, Qt::DisplayRole ).toString();
    // qDebug () << "New selected template name: " << mCurrTemplateName;
    showHelp();

    DocText dt = currentDocText();
    showDocText( dt );
  } else {
    mCurrTemplateName.clear();
  }
  emit validTemplateSelected( );
}

void TextSelection::showDocText( DocText dt )
{
  if( dt.type() != KraftDoc::Unknown && dt.isStandardText() ) {
    showHelp(i18n("This is the standard text used in new documents."));
  }

  mTextDisplay->setText( dt.text() );
}

void TextSelection::slotSelectDocType( const QString& doctype )
{
  QString partStr = KraftDoc::partToString( mPart );
  QString t = QString( i18n( "%1 Templates for %2", partStr, doctype) );
  mGroupBox->setTitle( t );
  mDocType = doctype;

  DocTextList dtList = DefaultProvider::self()->documentTexts( doctype, mPart );

  QStringList templNames;
  if( dtList.count() == 0 ) {
    showHelp( i18n("There is no %1 template text available for document type %2.<br/>"
                   "Click the add-button below to create one.", partStr, doctype ) );
  } else {
    foreach( DocText dt, dtList ) {
      templNames << dt.name();
    }
    showHelp();
  }
  mTemplNamesModel->setStringList( templNames );

  mTextDisplay->clear();

}

void TextSelection::addNewDocText( const DocText& dt )
{
  slotSelectDocType( mDocType ); // update the list of available texts

  QModelIndexList newItems = mTemplNamesModel->match( mTemplNamesModel->index(0), Qt::DisplayRole, dt.name() );
  if( newItems.size() > 0 ) {
    QModelIndex selected = newItems[0];
    mTextNameView->selectionModel()->setCurrentIndex( selected, QItemSelectionModel::Select);
  } else {
    // qDebug () << "Unable to find the new item named " << dt.name();
  }
  emit validTemplateSelected();
}

/* requires the QListViewItem set as a member in the doctext */
void TextSelection::updateDocText( const DocText& )
{
  QModelIndex selected = mTextNameView->selectionModel()->currentIndex();
  if( selected.isValid() ) {
    slotSelectDocType( mDocType );
    mTextNameView->selectionModel()->setCurrentIndex( selected, QItemSelectionModel::Select );
  }
}

bool TextSelection::validSelection() const
{
  return mTextNameView->selectionModel()->currentIndex().isValid();
}

void TextSelection::deleteCurrentText()
{
  slotSelectDocType( mDocType );
}


TextSelection::~TextSelection()
{
}

void TextSelection::initActions()
{
  mAcMoveToDoc = new QAction(QIcon::fromTheme( "go-previous" ), i18n("&Use in Document"), this);
  connect(mAcMoveToDoc, SIGNAL(triggered()), this, SIGNAL(actionCurrentTextToDoc()));

  mMenu->addAction( mAcMoveToDoc );

}

/* if the help string is empty, the help widget disappears. */
void TextSelection::showHelp( const QString& help )
{
  mHelpDisplay->setText( help );
  if( help.isEmpty() ) {
    mHelpDisplay->hide();
  } else {
    mHelpDisplay->show();
#if 0
    // qDebug () << "Displaying help text: " << help;

    QPropertyAnimation *ani = new QPropertyAnimation( mHelpDisplay, "geometry" );
    QRect r2 = r1;
    r2.setHeight( 200 );
    ani->setDuration( 2000 );
    ani->setStartValue( r1 );
    ani->setEndValue( r2 );
    ani->start();
#endif
  }
}

DocText TextSelection::currentDocText() const
{
  DocTextList dtList = DefaultProvider::self()->documentTexts( mDocType, mPart );
  foreach( DocText dt, dtList ) {
    if( dt.name() == mCurrTemplateName ) {
      return dt;
    }
  }
  DocText dt;
  return dt;
}

QString TextSelection::currentText() const
{
  return currentDocText().text();
}


void TextSelection::slotRMB(QPoint )
{
  // mMenu->popup( mTextsView->mapToGlobal(point) );
}



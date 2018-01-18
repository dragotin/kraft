/***************************************************************************
                          docassistant.cpp  - Assistant widget
                             -------------------
    begin                : April 2007
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
#include <QtGui>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include <klocalizedstring.h>

#include "docassistant.h"
#include "docpostcard.h"
#include "catalogselection.h"
#include "textselection.h"
#include "kraftsettings.h"
#include "kataloglistview.h"
#include "doctext.h"
#include "defaultprovider.h"
#include "headertemplateprovider.h"
#include "footertemplateprovider.h"
#include "catalogtemplateprovider.h"
#include "addresstemplateprovider.h"

DocAssistant::DocAssistant( QWidget *parent ):
  QSplitter( parent ), mFullPreview( true ),
  mActivePage( KraftDoc::Header )
{
  setOrientation( Qt::Vertical );

  QWidget *topWidget = new QWidget;
  QVBoxLayout *topVBox = new QVBoxLayout;
  topVBox->setMargin(0);
  topWidget->setLayout( topVBox );

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  topVBox->addLayout( buttonLayout );

  QPushButton *pb = new QPushButton( i18n( "Show &Templates" ) );
  buttonLayout->addWidget( pb );
  connect( pb, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleShowTemplates( bool ) ) );
  pb->setCheckable( true );
  pb->setToolTip( i18n( "Show mask to create or select templates to be used in the document" ) );

  buttonLayout->addStretch();
  topVBox->addLayout(buttonLayout);
  mPostCard = new DocPostCard;
  mPostCard->slotSetMode( DocPostCard::Full, KraftDoc::Header );
  // setResizeMode( vb /* mPostCard->view() */, KeepSize );

  topVBox->addWidget(mPostCard);

  addWidget(topWidget);

  mTemplatePane = new QWidget;
  QVBoxLayout *bottomVBox = new QVBoxLayout;
  bottomVBox->setMargin(0);

  mTemplatePane->setLayout( bottomVBox );
  addWidget( mTemplatePane );

  setStretchFactor(0, 0);
  setStretchFactor(1, 0);

  mWidgetStack = new QStackedWidget;

  bottomVBox->addWidget( mWidgetStack );
  mWidgetStack->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  /* Selections are the gui reprenentations of the template providing catalogs
   * like header- and footer texts and catalogs.
   */
  mCatalogSelection = new CatalogSelection;
  mWidgetStack->addWidget( mCatalogSelection );
  connect( mCatalogSelection, SIGNAL( selectionChanged(QTreeWidgetItem*,QTreeWidgetItem*) ),
           this,  SLOT( slotCatalogSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*) ) );

  mHeaderSelector = new TextSelection( 0, KraftDoc::Header );
  mWidgetStack->addWidget( mHeaderSelector );

  connect( mHeaderSelector, SIGNAL(validTemplateSelected() ),
           this, SLOT( slotTemplateSelectionChanged() ) );
  connect( mHeaderSelector, SIGNAL(editCurrentTemplate()),
           this, SLOT(slotEditTemplate()));

  mFooterSelection = new TextSelection( 0, KraftDoc::Footer );
  mWidgetStack->addWidget( mFooterSelection );

  connect( mFooterSelection, SIGNAL(validTemplateSelected()),
           this, SLOT(slotTemplateSelectionChanged()));
  connect( mFooterSelection, SIGNAL(editCurrentTemplate()),
           this, SLOT(slotEditTemplate()));
  connect( mFooterSelection, SIGNAL( actionCurrentTextToDoc() ),
           this,  SLOT( slotAddToDocument() ) );

  connect( mPostCard, SIGNAL( selectPage( int ) ),
           this,  SLOT( slotSelectDocPart( int ) ) );

  QHBoxLayout *butHBox2 = new QHBoxLayout;
  bottomVBox->addLayout( butHBox2 );

  QIcon icons = QIcon::fromTheme( "go-previous" ); // KDE 4 icon name: go-previous
  mPbAdd  = new QPushButton( icons, QString() );
  mPbAdd->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbAdd, SIGNAL( clicked() ), this, SLOT( slotAddToDocument() ) );
  butHBox2->addWidget( mPbAdd );
  mPbAdd->setToolTip( i18n( "Add a template to the document" ) );

  icons = QIcon::fromTheme( "document-new" );
  mPbNew  = new QPushButton( icons, QString() ); // KDE 4 icon name: document-new
  mPbNew->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbNew, SIGNAL( clicked() ), this, SLOT( slotNewTemplate() ) );
  mPbNew->setToolTip( i18n( "Create a new template" ) );
  butHBox2->addWidget( mPbNew );

  icons = QIcon::fromTheme( "document-properties" );
  mPbEdit  = new QPushButton( icons, QString() ); // KDE 4 icon name: document-properties
  mPbEdit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbEdit, SIGNAL( clicked() ), this, SLOT( slotEditTemplate() ) );
  mPbEdit->setToolTip( i18n( "Edit the current template" ) );
  butHBox2->addWidget( mPbEdit );

  icons = QIcon::fromTheme( "edit-delete" );
  mPbDel  = new QPushButton( icons, QString() ); // KDE 4 icon name: edit-delete
  mPbDel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbDel, SIGNAL( clicked() ), this, SLOT( slotDeleteTemplate() ) );
  mPbDel->setToolTip( i18n( "Delete the current template" ) );
  butHBox2->addWidget( mPbDel );

  butHBox2->addStretch();

  mPbAdd->setEnabled( false );
  mPbNew->setEnabled( false );
  mPbEdit->setEnabled( false );
  mPbDel->setEnabled( false );

  /* Template Provider initialisations */
  mHeaderTemplateProvider = new HeaderTemplateProvider( parent );

  /* get a new header text from the default provider */
  connect( mHeaderTemplateProvider, SIGNAL( newHeaderText( const DocText& ) ),
           this,  SLOT( slotNewHeaderDocText( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( updateHeaderText( const DocText& ) ),
           this,  SLOT( slotUpdateHeaderDocText( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( headerTextToDocument( const DocText& ) ),
           this,  SLOT( slotHeaderTextToDocument( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( deleteHeaderText( const DocText& ) ),
           this,  SLOT( slotHeaderTextDeleted( const DocText& ) ) );
  mHeaderTemplateProvider->setSelection( mHeaderSelector );

  mFooterTemplateProvider = new FooterTemplateProvider( parent );

  /* get a new Footer text from the default provider */
  connect( mFooterTemplateProvider, SIGNAL( newFooterText( const DocText& ) ),
           this,  SLOT( slotNewFooterDocText( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( updateFooterText( const DocText& ) ),
           this,  SLOT( slotUpdateFooterDocText( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( footerTextToDocument( const DocText& ) ),
           this,  SLOT( slotFooterTextToDocument( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( deleteFooterText( const DocText& ) ),
           this,  SLOT( slotFooterTextDeleted( const DocText& ) ) );
  mFooterTemplateProvider->setSelection( mFooterSelection );

  /* Catalog Template Provider */
  mCatalogTemplateProvider = new CatalogTemplateProvider( parent );
  mCatalogTemplateProvider->setCatalogSelection( mCatalogSelection );
  connect( mCatalogTemplateProvider,  SIGNAL( templatesToDocument(Katalog*,CatalogTemplateList) ),
           this, SIGNAL( templatesToDocument(Katalog*,CatalogTemplateList) ) );

  mCurrTemplateProvider = mHeaderTemplateProvider;

  const QList<int> sizes = KraftSettings::self()->assistantSplitterSetting();
  if( sizes.count() > 0 ) {
      setSizes( sizes );
  }
  mTemplatePane->hide();
}

void DocAssistant::slotAddToDocument()
{
  // qDebug () << "SlotAddToDocument called!" << endl;
  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotTemplateToDocument();
  }
}

void DocAssistant::slotTemplateSelectionChanged( )
{
  if( mActivePage == KraftDoc::Positions ) { // no editing on the catalogs
    mPbNew->setEnabled( false );
    mPbEdit->setEnabled( false );
    mPbDel->setEnabled( false );
  } else {
    bool mv = false;
    if( mActivePage == KraftDoc::Header ) {
      mv = mHeaderSelector->validSelection();
    } else if( mActivePage == KraftDoc::Footer ) {
      mv = mFooterSelection->validSelection();
    }
    mPbAdd->setEnabled( mv );
    mPbNew->setEnabled( true );
    mPbEdit->setEnabled( mv );
    mPbDel->setEnabled( mv );
  }
}

void DocAssistant::slotCatalogSelectionChanged(QTreeWidgetItem *current ,QTreeWidgetItem *)
{
  // enable the move-to-document button.
  // qDebug () << "catalog position selection changed!" << endl;
  if ( current ) {
    mPbAdd->setEnabled( true );
  } else {
    mPbAdd->setEnabled( false );
  }
}

void DocAssistant::slotNewTemplate()
{
  /* always set the doc type in case the provider benefits from that */
  mCurrTemplateProvider->slotSetDocType( mDocType );
  mCurrTemplateProvider->slotNewTemplate();
}

/* a new header doc text was created and should go to the document */
void DocAssistant::slotNewHeaderDocText( const DocText& dt )
{
  /* show in list of texts in the GUI */
  mHeaderSelector->addNewDocText( dt );
}

/* called with a changed text that needs to be updated in the view */
void DocAssistant::slotUpdateHeaderDocText( const DocText& dt )
{
  mHeaderSelector->updateDocText( dt );
}

/* the user hit "add to document" to use a header text template */
void DocAssistant::slotHeaderTextToDocument( const DocText& dt )
{
  emit headerTextTemplate( dt.text() );
}

/* a new header doc text was created and should go to the document */
void DocAssistant::slotNewFooterDocText( const DocText& dt )
{
  /* show in list of texts in the GUI */
  mFooterSelection->addNewDocText( dt );
}

/* called with a changed text that needs to be updated in the view */
void DocAssistant::slotUpdateFooterDocText( const DocText& dt )
{
  mFooterSelection->updateDocText( dt );
}

/* the user hit "add to document" to use a header text template */
void DocAssistant::slotFooterTextToDocument( const DocText& dt )
{
  emit footerTextTemplate( dt.text() );
}

/* Slot that initiates an edit */
void DocAssistant::slotEditTemplate()
{
  // qDebug () << "Editing a template using the currentTemplProvider" << endl;
  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotSetDocType( mDocType );
    mCurrTemplateProvider->slotEditTemplate();
  }
}

/* slot that initialises a delete, called from the delete button */
void DocAssistant::slotDeleteTemplate()
{

    QMessageBox msgBox;
    msgBox.setText(i18n( "Do you really want to delete the template permanently?\n"
                         "It can not be recovered."));
    msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();

    if ( ret == QMessageBox::No ) {
        return;
    }

    if ( mCurrTemplateProvider ) {
        mCurrTemplateProvider->slotDeleteTemplate();
    }
}

void DocAssistant::slotHeaderTextDeleted( const DocText& /* dt */)
{
  mHeaderSelector->deleteCurrentText();
  slotTemplateSelectionChanged( ); // disable the edit buttons etc.
}

void DocAssistant::slotFooterTextDeleted( const DocText& /* dt */)
{
  mFooterSelection->deleteCurrentText();
  slotTemplateSelectionChanged( ); // disable the edit buttons etc.
}

/* slot that opens the template details in case on == true */
void DocAssistant::slotToggleShowTemplates( bool on )
{
  if ( on ) {
    // setFullPreview is set in the subslots called from here, that
    // makes mFullPreview truly reflecting the state of the toggle button
    if ( mActivePage == KraftDoc::Header ) {
      slotShowHeaderTemplates();
    } else if ( mActivePage == KraftDoc::Positions ) {
      slotShowCatalog();
    } else if ( mActivePage == KraftDoc::Footer ) {
      slotShowFooterTemplates();
    }
  } else {
    // hide the details
    setFullPreview( true, mActivePage );
  }
  emit toggleShowTemplates( on );
}

DocPostCard *DocAssistant::postCard()
{
    return mPostCard;
}

CatalogSelection* DocAssistant::catalogSelection()
{
    return mCatalogSelection;
}

/* sets the Part of the doc, eg. Header, Footer */
void DocAssistant::slotSelectDocPart( int p )
{
  mActivePage = p;
  if( mActivePage == KraftDoc::Header ) {
    mCurrTemplateProvider = mHeaderTemplateProvider;
  } else if( mActivePage == KraftDoc::Positions ) {
    mCurrTemplateProvider = mCatalogTemplateProvider;
  } else if( mActivePage == KraftDoc::Footer ) {
    mCurrTemplateProvider = mFooterTemplateProvider;
  } else {
    // qDebug () << "Alert: Unknown document part id: " << p;
  }
  emit selectPage( p );
  slotToggleShowTemplates( !mFullPreview );
  slotTemplateSelectionChanged( ); // hide the add, edit- and del buttons
}

/* Doc Type like offer, invoice etc. */
void DocAssistant::slotSetDocType( const QString& type )
{
  mDocType = type;
  mHeaderSelector->slotSelectDocType( type );
  mFooterSelection->slotSelectDocType( type );
}

void DocAssistant::slotShowCatalog( )
{
  setFullPreview( false, KraftDoc::Positions );
  mWidgetStack->setCurrentWidget( mCatalogSelection );
}

void DocAssistant::slotShowHeaderTemplates()
{
  setFullPreview( false, KraftDoc::Header );
  mWidgetStack->setCurrentWidget( mHeaderSelector );
}

void DocAssistant::slotShowFooterTemplates()
{
  setFullPreview( false, KraftDoc::Footer );
  mWidgetStack->setCurrentWidget( mFooterSelection );
}

void DocAssistant::setFullPreview( bool setFull, int id )
{
    if ( setFull ) {
        /* remember the sizes used before */
        saveSplitterSizes();

        mTemplatePane->hide();
        mPostCard->slotSetMode( DocPostCard::Full, id );
        mFullPreview = true;
    } else {
        mTemplatePane->show();
        mPostCard->slotSetMode( DocPostCard::Mini, id );

        if ( KraftSettings::self()->assistantSplitterSetting().size() == 2 ) {
            QList<int> sizes = KraftSettings::self()->assistantSplitterSetting();
            if( sizes.contains(0)) {
                sizes[0] = 50;
                sizes[1] = 50;
            }
            setSizes( sizes );
        }
        mFullPreview = false;
    }
}

void DocAssistant::saveSplitterSizes()
{
    if( mTemplatePane->isVisible() ) {
        const QList<int> s = sizes();
        KraftSettings::self()->setAssistantSplitterSetting( s );
    }
}

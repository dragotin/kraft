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
  mActivePage( KraftDoc::Part::Header )
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
  mPostCard->slotSetMode( DocPostCard::Full, KraftDoc::Part::Header );
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
  connect( mCatalogSelection, &CatalogSelection::selectionChanged,
           this,  &DocAssistant::slotCatalogSelectionChanged);

  mHeaderSelector = new TextSelection( 0, KraftDoc::Part::Header );
  mWidgetStack->addWidget( mHeaderSelector );

  connect( mHeaderSelector, &TextSelection::validTemplateSelected,
           this, &DocAssistant::slotTemplateSelectionChanged );
  connect( mHeaderSelector, &TextSelection::editCurrentTemplate,
           this, &DocAssistant::slotEditTemplate);

  mFooterSelection = new TextSelection( 0, KraftDoc::Part::Footer );
  mWidgetStack->addWidget( mFooterSelection );

  connect( mFooterSelection, &TextSelection::validTemplateSelected,
           this, &DocAssistant::slotTemplateSelectionChanged);
  connect( mFooterSelection, &TextSelection::editCurrentTemplate,
           this, &DocAssistant::slotEditTemplate);
  connect( mFooterSelection, &TextSelection::actionCurrentTextToDoc,
           this,  &DocAssistant::slotAddToDocument );

  connect( mPostCard, &DocPostCard::selectPage, this, &DocAssistant::slotSelectDocPart);

  QHBoxLayout *butHBox2 = new QHBoxLayout;
  bottomVBox->addLayout( butHBox2 );

  QIcon icons = DefaultProvider::self()->icon( "arrow-narrow-left" );
  mPbAdd  = new QPushButton( icons, QString() );
  mPbAdd->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbAdd, SIGNAL( clicked() ), this, SLOT( slotAddToDocument() ) );
  butHBox2->addWidget( mPbAdd );
  mPbAdd->setToolTip( i18n( "Add a template to the document" ) );

  icons = DefaultProvider::self()->icon( "arrow-bar-to-left" );
  mPbInsert  = new QPushButton( icons, QString() );
  mPbInsert->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbInsert, SIGNAL( clicked() ), this, SLOT( slotInsertIntoDocument() ) );
  butHBox2->addWidget( mPbInsert);
  mPbInsert->setToolTip( i18n( "Insert the template to the document" ) );

  butHBox2->insertSpacing(2, 40);

  icons = DefaultProvider::self()->icon( "plus" );
  mPbNew  = new QPushButton( icons, QString() ); // KDE 4 icon name: document-new
  mPbNew->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbNew, SIGNAL( clicked() ), this, SLOT( slotNewTemplate() ) );
  mPbNew->setToolTip( i18n( "Create a new template" ) );
  butHBox2->addWidget( mPbNew );

  icons = DefaultProvider::self()->icon( "edit" );
  mPbEdit  = new QPushButton( icons, QString() ); // KDE 4 icon name: document-properties
  mPbEdit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbEdit, SIGNAL( clicked() ), this, SLOT( slotEditTemplate() ) );
  mPbEdit->setToolTip( i18n( "Edit the current template" ) );
  butHBox2->addWidget( mPbEdit );

  icons = DefaultProvider::self()->icon( "x" );
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
  mPbInsert->setEnabled(false);

  /* Template Provider initialisations */
  mHeaderTemplateProvider = new HeaderTemplateProvider( parent );

  /* get a new header text from the default provider */
  connect( mHeaderTemplateProvider, SIGNAL( newHeaderText( const DocText& ) ),
           this,  SLOT( slotNewHeaderDocText( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( updateHeaderText( const DocText& ) ),
           this,  SLOT( slotUpdateHeaderDocText( const DocText& ) ) );
  connect( mHeaderTemplateProvider, &HeaderTemplateProvider::headerTextToDocument,
           this, &DocAssistant::headerTextTemplate);

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
  connect( mFooterTemplateProvider, &FooterTemplateProvider::footerTextToDocument,
           this, &DocAssistant::footerTextTemplate);
  connect( mFooterTemplateProvider, SIGNAL( deleteFooterText( const DocText& ) ),
           this,  SLOT( slotFooterTextDeleted( const DocText& ) ) );
  mFooterTemplateProvider->setSelection( mFooterSelection );

  /* Catalog Template Provider */
  mCatalogTemplateProvider = new CatalogTemplateProvider( parent );
  mCatalogTemplateProvider->setCatalogSelection( mCatalogSelection );
  connect(mCatalogTemplateProvider, &CatalogTemplateProvider::templatesToDocument,
          this, &DocAssistant::templatesToDocument);
  mCurrTemplateProvider = mHeaderTemplateProvider;

  const QList<int> sizes = KraftSettings::self()->assistantSplitterSetting();
  if( sizes.count() > 0 ) {
      setSizes( sizes );
  }
  mTemplatePane->hide();
}

void DocAssistant::slotInsertIntoDocument()
{
  // qDebug () << "SlotInsertIntoDocument called!";
  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotInsertTemplateToDocument();
  }
}

void DocAssistant::slotAddToDocument()
{
  // qDebug () << "SlotAddToDocument called!";
  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotTemplateToDocument();
  }
}

void DocAssistant::slotTemplateSelectionChanged( )
{
    if (!mCurrTemplateProvider) {
        mPbNew->setEnabled(false);
        mPbEdit->setEnabled(false);
        mPbDel->setEnabled(false);
        mPbInsert->setEnabled(false);
        return;
    }

  if( mActivePage == KraftDoc::Part::Positions ) { // no editing on the catalogs
      bool enableNew {false};

      auto kat = static_cast<CatalogTemplateProvider*>(mCurrTemplateProvider)->currentCatalog();
      if (kat->type() == KatalogType::TemplateCatalog) {
          enableNew = true;
      }
    mPbNew->setEnabled(enableNew);
    mPbEdit->setEnabled( false );
    mPbDel->setEnabled( false );
    mPbInsert->setEnabled(false);
  } else {
    bool mv {false};
    if( mActivePage == KraftDoc::Part::Header ) {
      mv = mHeaderSelector->validSelection();
    } else if( mActivePage == KraftDoc::Part::Footer ) {
      mv = mFooterSelection->validSelection();
    }
    mPbAdd->setEnabled( mv );
    mPbNew->setEnabled( true );
    mPbEdit->setEnabled( mv );
    mPbDel->setEnabled( mv );
    mPbInsert->setEnabled(mv);
  }
}

void DocAssistant::slotCatalogSelectionChanged(QTreeWidgetItem *current ,QTreeWidgetItem *)
{
    // enable the move-to-document button.
    // qDebug () << "catalog position selection changed!";
    if ( current ) {
        mPbAdd->setEnabled( true );
    } else {
        mPbAdd->setEnabled( false );
    }
    mPbInsert->setEnabled(false);

    slotTemplateSelectionChanged();
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

/* Slot that initiates an edit */
void DocAssistant::slotEditTemplate()
{
  // qDebug () << "Editing a template using the currentTemplProvider";
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
    if ( mActivePage == KraftDoc::Part::Header ) {
      slotShowHeaderTemplates();
    } else if ( mActivePage == KraftDoc::Part::Positions ) {
      slotShowCatalog();
    } else if ( mActivePage == KraftDoc::Part::Footer ) {
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
void DocAssistant::slotSelectDocPart( KraftDoc::Part p )
{
  mActivePage = p;
  if( mActivePage == KraftDoc::Part::Header ) {
    mCurrTemplateProvider = mHeaderTemplateProvider;
  } else if( mActivePage == KraftDoc::Part::Positions ) {
    mCurrTemplateProvider = mCatalogTemplateProvider;
  } else if( mActivePage == KraftDoc::Part::Footer ) {
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
  setFullPreview( false, KraftDoc::Part::Positions );
  mWidgetStack->setCurrentWidget( mCatalogSelection );
}

void DocAssistant::slotShowHeaderTemplates()
{
  setFullPreview( false, KraftDoc::Part::Header );
  mWidgetStack->setCurrentWidget( mHeaderSelector );
}

void DocAssistant::slotShowFooterTemplates()
{
  setFullPreview( false, KraftDoc::Part::Footer );
  mWidgetStack->setCurrentWidget( mFooterSelection );
}

void DocAssistant::setFullPreview( bool setFull, KraftDoc::Part p )
{
    if ( setFull ) {
        /* remember the sizes used before */
        saveSplitterSizes();

        mTemplatePane->hide();
        mPostCard->slotSetMode(DocPostCard::Full, p);
        mFullPreview = true;
    } else {
        mTemplatePane->show();
        mPostCard->slotSetMode(DocPostCard::Mini, p);

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

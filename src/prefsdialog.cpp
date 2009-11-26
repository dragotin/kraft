/***************************************************************************
                   prefsdialog.cpp  - the preferences Dialog
                             -------------------
    begin                : Sun Jul 3 2004
    copyright            : (C) 2004 by Klaas Freitag
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

#include <QLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLayout>
#include <QCheckBox>
#include <QSqlQuery>
#include <QSpinBox>
#include <QToolTip>
#include <q3listview.h>
#include <QPalette>
#include <QTreeView>
#include <QSqlTableModel>

#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kglobal.h>

#include "prefsdialog.h"
#include "katalogsettings.h"
#include "kraftsettings.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include "doctypeedit.h"
#include "taxeditdialog.h"
#include "documentman.h"


// ################################################################################

PrefsDialog::PrefsDialog( QWidget *parent)
    : KPageDialog( parent )
{
  setFaceType( KPageDialog::List );
  setModal( true );
  setCaption( i18n( "Configure Kraft" ) );
  setButtons( Ok|Cancel);
  setDefaultButton( Ok );

  databaseTab();
  docTab();
  doctypeTab();
  taxTab();

  readConfig();
  slotCheckConnect();
}


void PrefsDialog::databaseTab()
{
  QWidget *topWidget = new QWidget;

  QLabel *label;

  QVBoxLayout *vboxLay = new QVBoxLayout;
  KPageWidgetItem *topFrame = addPage( topWidget, i18n( "Database" ) );
                              // i18n( "Database Connection Settings" ) );

  // topFrame->setIcon( DesktopIcon( "connect_no" ) ); // KDE 4 name: (probably) network-server-database


  QGridLayout *topLayout = new QGridLayout;
  vboxLay->addLayout( topLayout );
  topLayout->setSpacing( spacingHint() );
  topLayout->addItem(new QSpacerItem(spacingHint(), 0), 0, 0);

  label = new QLabel(i18n("Database Host:") );
  topLayout->addWidget(label, 0,0);

  label = new QLabel(i18n("Database Name:") );
  topLayout->addWidget(label, 1,0);

  label = new QLabel(i18n("Database User:") );
  topLayout->addWidget(label, 2,0);

  label = new QLabel(i18n("Database Password:") );
  topLayout->addWidget(label, 3,0);

  label = new QLabel(i18n("Connection Status:") );
  topLayout->addWidget(label, 4,0);

  m_pbCheck = new QPushButton( i18n( "Check Connection" ) );
  m_pbCheck->setEnabled( false );
  topLayout->addWidget( m_pbCheck, 5, 1 );

  QLabel *l1 = new QLabel(  i18n( "Please restart Kraft after "
                                  "changes in the database connection "
                                  "parameters to make the changes "
                                  "effective!" ) );
  l1->setTextFormat( Qt::RichText );
  QPalette palette;
  palette.setColor(l1->backgroundRole(), QColor( "#ffcbcb"));
  l1->setPalette(palette);
  l1->setMargin( 5 );
  l1->setFrameStyle( QFrame::Box + QFrame::Raised );
  l1->setLineWidth( 1 );
  //l1->setAlignment( Qt::AlignCenter | Qt::TextExpandTabs | Qt::TextWordWrap );
  l1->setAlignment(Qt::AlignCenter);
  topLayout->addWidget( l1, 0, 1, 6, 5 ); //Not sure!

  m_leHost = new QLineEdit;
  connect( m_leHost, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  topLayout->addWidget(m_leHost, 0,1);

  m_leName = new QLineEdit;
  connect( m_leName, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  topLayout->addWidget(m_leName, 1,1);

  m_leUser = new QLineEdit;
  connect( m_leUser, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  topLayout->addWidget(m_leUser, 2,1);

  m_lePasswd = new QLineEdit;
  m_lePasswd->setEchoMode(QLineEdit::Password);
  connect( m_lePasswd, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  topLayout->addWidget(m_lePasswd, 3,1);

  m_statusLabel = new QLabel;
  topLayout->addWidget( m_statusLabel,  4, 1 );

  connect( m_pbCheck, SIGNAL( clicked() ),
           this, SLOT( slotCheckConnect() ) );

  vboxLay->addItem( new QSpacerItem( 1, 1 ) );

  topWidget->setLayout( vboxLay );
}

void PrefsDialog::taxTab()
{
  QWidget *topWidget = new QWidget;

  KPageWidgetItem *topFrame = addPage( topWidget, i18n( "Taxes" )
                              /* i18n( "Tax Settings." )
                                 DesktopIcon( "queue" ) */ );

  QVBoxLayout *vboxLay = new QVBoxLayout;
  vboxLay->setSpacing( spacingHint() );

  QLabel *label;
  label = new QLabel(i18n("Tax rates beginning at date:"));
  vboxLay->addWidget( label );

  mTaxModel = new QSqlTableModel;
  mTaxModel->setTable("taxes");
  mTaxModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mTaxModel->select();
  mTaxModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
  mTaxModel->setHeaderData(1, Qt::Horizontal, tr("Full Tax [%]"));
  mTaxModel->setHeaderData(2, Qt::Horizontal, tr("Reduced Tax [%]"));
  mTaxModel->setHeaderData(3, Qt::Horizontal, tr("Start Date"));

  mTaxTreeView = new QTreeView;
  vboxLay->addWidget( mTaxTreeView );
  mTaxTreeView->setModel(mTaxModel);
  mTaxTreeView->hideColumn(0);

  connect( mTaxTreeView, SIGNAL(clicked(QModelIndex)),
           SLOT( slotTaxSelected(QModelIndex) ) );

  QHBoxLayout *butLay = new QHBoxLayout;
  // butLay->setSpacing( KDialogBase::spacingHint() );
  butLay->addStretch( 1 );
  KPushButton *but = new KPushButton( i18n( "add" ));
  connect( but, SIGNAL( clicked() ), SLOT( slotAddTax() ) );
  butLay->addWidget( but );

  mDelTax = new KPushButton( i18n( "delete" ) );
  connect( mDelTax, SIGNAL( clicked() ), SLOT( slotDeleteTax() ) );
  butLay->addWidget( mDelTax );
  mDelTax->setEnabled( false );

  vboxLay->addLayout( butLay );
  topWidget->setLayout( vboxLay );
}

void PrefsDialog::slotAddTax()
{
  TaxEditDialog *dialog = new TaxEditDialog(mTaxModel, this);
  dialog->show();
}

void PrefsDialog::slotDeleteTax()
{
  if ( mTaxTreeView->currentIndex().isValid() )
  {
    kDebug() << mTaxTreeView->currentIndex().row();
    mTaxModel->removeRow(mTaxTreeView->currentIndex().row());
    mTaxTreeView->update();
    //Will submit changes directly into the database
    //mTaxModel->submitAll();
  }
}

void PrefsDialog::slotTaxSelected(QModelIndex)
{
  bool state = false;
  if ( mTaxTreeView->currentIndex().isValid() ) {
    state = true;
  }

  mDelTax->setEnabled( state );
}

void PrefsDialog::docTab()
{
  QLabel *label;
  QWidget *topWidget = new QWidget;

  KPageWidgetItem *topFrame = addPage( topWidget, i18n( "Document Defaults" ) );
                              // i18n( "Defaults for new Documents." ),
                              // DesktopIcon( "queue" ) );

  QVBoxLayout *vboxLay = new QVBoxLayout;
  topWidget->setLayout( vboxLay );
  QGridLayout *topLayout = new QGridLayout;
  vboxLay->addLayout( topLayout );

  topLayout->setSpacing( spacingHint() );
  topLayout->addItem(new QSpacerItem(spacingHint(), 0), 0, 0);

  label = new QLabel(i18n("&Default document type on creation:") );
  topLayout->addWidget(label, 0,0);

  mCbDocTypes = new QComboBox;
  label->setBuddy( mCbDocTypes );
  mCbDocTypes->setToolTip( i18n( "New documents are from the selected type by default." ) );
  topLayout->addWidget( mCbDocTypes, 0, 1 );
  mCbDocTypes->insertItems(-1, DocType::allLocalised() );

  QLabel *f = new QLabel;
  f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  vboxLay->addWidget( f );

  // Localisation on document level
  mCbDocLocale = new QCheckBox( i18n( "Enable &Localisation on Document Level" ) );
  mCbDocLocale->setToolTip( i18n( "Checking this enables language settings for each document."
                                     "<br>Leave it unchecked to use the KDE default settings for "
                                     "the document localisation." ) );
  vboxLay->addWidget( mCbDocLocale );

  vboxLay->addWidget( new QWidget );

  f = new QLabel;
  f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  vboxLay->addWidget( f );

  QHBoxLayout *butLay = new QHBoxLayout;
  QLabel *l = new QLabel( i18n( "Default &Tax for Documents:" ) );
  butLay->addWidget( l );
  mCbDefaultTaxType = new QComboBox;
  butLay->addWidget( mCbDefaultTaxType );
  l->setBuddy( mCbDefaultTaxType );

  mCbDefaultTaxType->setToolTip( i18n( "The default tax setting for all documents." ) );
  mCbDefaultTaxType->insertItem( 0, i18n("Display no tax at all" , 0));
  mCbDefaultTaxType->insertItem( 1, i18n("Calculate reduced tax for all items" ));
  mCbDefaultTaxType->insertItem( 2, i18n("Calculate full tax for all items" ) );
  // mCbDefaultTaxType->insertItem( 3, i18n("Calculate on individual item tax rate" ));
  vboxLay->addLayout( butLay );

  // space eater
  QWidget *spaceEater = new QWidget;
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
  vboxLay->addWidget( spaceEater );
}

void PrefsDialog::doctypeTab()
{
  QWidget *topWidget = new QWidget;
  KPageWidgetItem *topFrame = addPage( topWidget, i18n( "Document Types" ) );
                              // i18n( "Edit Details of Document Types." ),
                              // DesktopIcon( "folder_man" ) );

  QVBoxLayout *vboxLay = new QVBoxLayout;
  topWidget->setLayout(vboxLay);
  vboxLay->setSpacing( 0 ); // spacingHint() );

  mDocTypeEdit = new DocTypeEdit;
  // FIXME mDocTypeEdit->mCentralSplit->setMargin( 0 );
  vboxLay->addWidget( mDocTypeEdit );

  connect( mDocTypeEdit, SIGNAL( removedType( const QString& ) ),
           SLOT( slotDocTypeRemoved( const QString& ) ) );

}

void PrefsDialog::slotDocTypeRemoved( const QString& type )
{
  // check if the default document type is still there
  QString currDefault = mCbDocTypes->currentText();

  if ( currDefault == type ) {
    KMessageBox::information ( this,  i18n( "The old default doc type for new documents was just deleted."
                                            "Please check the setting in the Document Defaults in the "
                                            "Kraft preferences Dialog." ),
                               i18n( "Document Default Change" ),
                               QString::fromLatin1( "DefaultDocTypeDeleted" ) );
  }

  for ( int i=0; i < mCbDocTypes->count(); i++ ) {
    if ( mCbDocTypes->itemText( i ) == type ) {
      mCbDocTypes->removeItem( i );
      continue;
    }
  }
}

void PrefsDialog::slotDbCredentialsChanged( const QString& )
{
  bool en = false;
  if ( !m_leName->text().isEmpty() ) {
    en = true;
  }

  m_pbCheck->setEnabled( en );
}

void PrefsDialog::readConfig()
{
    m_leHost->setText( KatalogSettings::self()->dbServerName() );
    m_leName->setText( KatalogSettings::self()->dbFile() );
    m_leUser->setText( KatalogSettings::self()->dbUser() );
    m_lePasswd->setText( KatalogSettings::self()->dbPassword() );

    mCbDocLocale->setChecked( KraftSettings::self()->showDocumentLocale() );

    QString t = KraftSettings::self()->doctype();
    if ( t.isEmpty() ) t = DefaultProvider::self()->docType();

    mCbDocTypes->setCurrentIndex( mCbDocTypes->findText( t ));

    mCbDefaultTaxType->setCurrentIndex( KraftSettings::self()->defaultTaxType()-1 );
}

void PrefsDialog::writeConfig()
{
    KatalogSettings::self()->setDbServerName(m_leHost->text());
    KatalogSettings::self()->setDbFile(m_leName->text());
    KatalogSettings::self()->setDbUser(m_leUser->text());
    KatalogSettings::self()->setDbPassword( m_lePasswd->text());
    KatalogSettings::self()->writeConfig();

    KraftSettings::self()->setShowDocumentLocale( mCbDocLocale->isChecked() );
    KraftSettings::self()->setDoctype( mCbDocTypes->currentText() );
    KraftSettings::self()->setDefaultTaxType( 1+mCbDefaultTaxType->currentIndex() );

    KraftSettings::self()->writeConfig();
}

void PrefsDialog::writeTaxes()
{
    mTaxModel->submitAll();
}

PrefsDialog::~PrefsDialog()
{
}

void PrefsDialog::slotCheckConnect()
{
  kDebug() << "Trying database connect to db " << m_leName->text() << endl;

  int x = KraftDB::self()->checkConnect( m_leHost->text(), m_leName->text(),
                                         m_leUser->text(), m_lePasswd->text() );
  kDebug() << "Connection result: " << x << endl;
  if ( x == 0 ) {
    m_statusLabel->setText( i18n( "Good!" ) );
  } else {
    m_statusLabel->setText( i18n( "Failed" ) );
  }
}

void PrefsDialog::accept()
{
  mDocTypeEdit->saveDocTypes();
  writeTaxes();
  writeConfig();
  QDialog::accept();
}

#include "prefsdialog.moc"

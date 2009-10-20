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

#include <qlayout.h>
#include <qlineedit.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <q3frame.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <qpushbutton.h>
#include <q3textedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <q3listbox.h>
#include <qsqlquery.h>
#include <qspinbox.h>
#include <q3sqlcursor.h>
#include <q3datatable.h>
#include <qtooltip.h>
#include <q3listview.h>

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
  topLayout->setColSpacing( 0, spacingHint() );

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
  l1->setBackgroundColor( QColor( "#ffcbcb" ) );
  l1->setMargin( 5 );
  l1->setFrameStyle( Q3Frame::Box + Q3Frame::Raised );
  l1->setLineWidth( 1 );
  l1->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextExpandTabs | Qt::TextWordWrap );
  topLayout->addMultiCellWidget( l1, 6,  6, 0, 1 );



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
  QLabel *label;
  QWidget *topWidget = new QWidget;

  KPageWidgetItem *topFrame = addPage( topWidget, i18n( "Taxes" )
                              /* i18n( "Tax Settings." )
                                 DesktopIcon( "queue" ) */ );

  QVBoxLayout *vboxLay = new QVBoxLayout;
  vboxLay->setSpacing( spacingHint() );

  label = new QLabel(i18n("Tax rates beginning at date:"));
  vboxLay->addWidget( label );

  mTaxListView = new Q3ListView( topWidget );
  vboxLay->addWidget( mTaxListView );
  mTaxListView->addColumn( i18n( "Start Date" ) );
  mTaxListView->addColumn( i18n( "Reduced Tax [%]" ) );
  mTaxListView->setColumnAlignment( 1, Qt::AlignRight );
  mTaxListView->addColumn( i18n( "Full Tax [%]" ) );
  mTaxListView->setColumnAlignment( 2, Qt::AlignRight );

  connect( mTaxListView, SIGNAL( selectionChanged() ),
           SLOT( slotTaxSelected() ) );

  QHBoxLayout *butLay = new QHBoxLayout;
  // butLay->setSpacing( KDialogBase::spacingHint() );
  butLay->addStretch( 1 );
  KPushButton *but = new KPushButton( i18n( "add" ));
  connect( but, SIGNAL( clicked() ), SLOT( slotAddTax() ) );
  butLay->addWidget( but );

#if 0
  but = new KPushButton( BarIconSet( "edit" ), i18n( "edit" ), topFrame );
  connect( but, SIGNAL( clicked() ), SLOT( slotEditTax() ) );
  butLay->addWidget( but );
#endif

  mDelTax = new KPushButton( i18n( "delete" ) );
  connect( mDelTax, SIGNAL( clicked() ), SLOT( slotDeleteTax() ) );
  butLay->addWidget( mDelTax );
  mDelTax->setEnabled( false );

  vboxLay->addLayout( butLay );
  topWidget->setLayout( vboxLay );
  buildTaxList();
}

void PrefsDialog::buildTaxList()
{
  QSqlQuery q( "SELECT * FROM taxes ORDER BY startDate" );
  mTaxListView->clear();

  TaxRecord::List taxes;

  while ( q.next() ) {
    QDate d = q.value( 3 ).toDate();
    double fullTax = q.value( 1 ).toDouble();
    double redTax = q.value( 2 ).toDouble();

    Q3ListViewItem *newItem = new Q3ListViewItem( mTaxListView,
                                                DefaultProvider::self()->locale()->formatDate( d ),
                                                DefaultProvider::self()->locale()->formatNumber(
                                                  QString::number( redTax ), true, 1 ),
                                                DefaultProvider::self()->locale()->formatNumber(
                                                  QString::number( fullTax ), true, 1 ) );
    ( void )newItem;
  }
}

void PrefsDialog::slotAddTax()
{
  TaxEditDialog ted( this );

  if ( ted.exec() == QDialog::Accepted ) {
    TaxRecord newTax = ted.newTaxRecord();

    Q3ListViewItem *item = mTaxListView->firstChild();
    bool found = false;
    while ( item && !found ) {
      bool ok;
      QDate date = DefaultProvider::self()->locale()->readDate( item->text( 0 ), &ok );
      if ( date == newTax.date ) {
        item->setText( 1, DefaultProvider::self()->locale()->formatNumber( newTax.reducedTax, 1 ) );
        item->setText( 2, DefaultProvider::self()->locale()->formatNumber( newTax.fullTax, 1 ) );
        found = true;
      }
      item = item->nextSibling();
    }

    if ( !found ) {
      Q3ListViewItem *newItem = new Q3ListViewItem( mTaxListView,
                                                 DefaultProvider::self()->locale()->formatDate( newTax.date, KLocale::ShortDate ),
                                                  DefaultProvider::self()->locale()->formatNumber(
                                                    QString::number( newTax.reducedTax ), true, 1 ),
                                                  DefaultProvider::self()->locale()->formatNumber(
                                                    QString::number( newTax.fullTax ), true, 1 ) );
      ( void )newItem;

    }
  }
}

void PrefsDialog::slotEditTax()
{

}

void PrefsDialog::slotDeleteTax()
{
  if ( mTaxListView->currentItem() )
    delete mTaxListView->currentItem();
}

void PrefsDialog::slotTaxSelected()
{
  bool state = false;
  if ( mTaxListView->currentItem() ) {
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
  topLayout->setColSpacing( 0, spacingHint() );

  label = new QLabel(i18n("&Default document type on creation:") );
  topLayout->addWidget(label, 0,0);

  mCbDocTypes = new QComboBox;
  label->setBuddy( mCbDocTypes );
  QToolTip::add( mCbDocTypes, i18n( "New documents are from the selected type by default." ) );
  topLayout->addWidget( mCbDocTypes, 0, 1 );
  mCbDocTypes->insertStringList( DocType::allLocalised() );

  QLabel *f = new QLabel;
  f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  vboxLay->addWidget( f );

  // Localisation on document level
  mCbDocLocale = new QCheckBox( i18n( "Enable &Localisation on Document Level" ) );
  QToolTip::add( mCbDocLocale, i18n( "Checking this enables language settings for each document."
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

  QToolTip::add( mCbDefaultTaxType, i18n( "The default tax setting for all documents." ) );
  mCbDefaultTaxType->insertItem( i18n( "Display no tax at all" ), 0 );
  mCbDefaultTaxType->insertItem( i18n( "Calculate reduced tax for all items" ), 1);
  mCbDefaultTaxType->insertItem( i18n( "Calculate full tax for all items" ), 2 );
  // mCbDefaultTaxType->insertItem( i18n( "Calculate on individual item tax rate" ), 3 );
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
    if ( mCbDocTypes->text( i ) == type ) {
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

    mCbDocTypes->setCurrentText( t );

    mCbDefaultTaxType->setCurrentItem( KraftSettings::self()->defaultTaxType()-1 );
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
    KraftSettings::self()->setDefaultTaxType( 1+mCbDefaultTaxType->currentItem() );

    KraftSettings::self()->writeConfig();
}

void PrefsDialog::writeTaxes()
{
  // First, multiply all fullTaxes with -1
  QSqlQuery q;
  q.prepare( "UPDATE taxes SET fullTax=(-1.0*fullTax)" );
  q.exec();

  // Go through all entries in the listview and update or insert
  QSqlQuery qUpdate;
  qUpdate.prepare( "UPDATE taxes SET fullTax=:fullTax, reducedTax=:redTax WHERE startDate=:date" );
  QSqlQuery qInsert;
  qInsert.prepare( "INSERT INTO taxes (fullTax, reducedTax, startDate) VALUES (:fullTax, :redTax, :date)" );

  Q3ListViewItem *item = mTaxListView->firstChild();
  while ( item ) {
    bool ok;
    QDate date     = DefaultProvider::self()->locale()->readDate( item->text( 0 ), &ok );
    double redTax  = DefaultProvider::self()->locale()->readNumber( item->text( 1 ), &ok );
    double fullTax = DefaultProvider::self()->locale()->readNumber( item->text( 2 ), &ok );

    qUpdate.bindValue( ":fullTax", fullTax );
    qUpdate.bindValue( ":redTax", redTax );
    qUpdate.bindValue( ":date", date );

    qUpdate.exec();
    if ( ! qUpdate.numRowsAffected() ) {
      qInsert.bindValue( ":fullTax", fullTax );
      qInsert.bindValue( ":redTax", redTax );
      qInsert.bindValue( ":date", date );
      qInsert.exec();
      if ( !qInsert.numRowsAffected() ) {
        kError() << "Could not insert tax records!";
      }
    }
    item = item->nextSibling();
  }

  QSqlQuery qDel;
  qDel.prepare( "DELETE FROM taxes WHERE fullTax < 0" );
  qDel.exec();

  DocumentMan::self()->clearTaxCache();
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

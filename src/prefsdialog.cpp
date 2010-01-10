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
#include <QSpinBox>
#include <QToolTip>
#include <QPalette>
#include <QSqlTableModel>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QStackedWidget>

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
#include "impviewwidgets.h"

// ################################################################################

PrefsDialog::PrefsDialog( QWidget *parent)
    : KPageDialog( parent )
{
  setFaceType( KPageDialog::List );
  setModal( true );
  setCaption( i18n( "Configure Kraft" ) );
  setButtons( Ok|Cancel);
  setDefaultButton( Ok );
  setMinimumWidth(700);

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

  //Setup the different widgets for the different database drivers first
  //Mysql first
  m_mysqlpart = new QWidget;
  QGridLayout *mysqlLayout = new QGridLayout;
  m_mysqlpart->setLayout(mysqlLayout);
  mysqlLayout->setMargin(0);

  label = new QLabel(i18n("Database Host:") );
  mysqlLayout->addWidget(label, 0,0);

  label = new QLabel(i18n("Database Name:") );
  mysqlLayout->addWidget(label, 1,0);

  label = new QLabel(i18n("Database User:") );
  mysqlLayout->addWidget(label, 2,0);

  label = new QLabel(i18n("Database Password:") );
  mysqlLayout->addWidget(label, 3,0);

  label = new QLabel(i18n("Connection Status:") );
  mysqlLayout->addWidget(label, 4,0);

  m_pbCheck = new QPushButton( i18n( "Check Connection" ) );
  mysqlLayout->addWidget( m_pbCheck, 5, 1 );

  m_leHost = new QLineEdit;
  connect( m_leHost, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  mysqlLayout->addWidget(m_leHost, 0,1);

  m_leName = new QLineEdit;
  connect( m_leName, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  mysqlLayout->addWidget(m_leName, 1,1);

  m_leUser = new QLineEdit;
  connect( m_leUser, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  mysqlLayout->addWidget(m_leUser, 2,1);

  m_lePasswd = new QLineEdit;
  m_lePasswd->setEchoMode(QLineEdit::Password);
  connect( m_lePasswd, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  mysqlLayout->addWidget(m_lePasswd, 3,1);

  m_statusLabel = new QLabel;
  m_statusLabel->setWordWrap(true);
  mysqlLayout->addWidget( m_statusLabel,  4, 1 );

  connect( m_pbCheck, SIGNAL( clicked() ),
           this, SLOT( slotCheckConnect() ) );

  //Sqlite next
  m_sqlitepart = new QWidget;
  QVBoxLayout *wrapper = new QVBoxLayout;
  QHBoxLayout *sqlitelayout = new QHBoxLayout;
  wrapper->addLayout(sqlitelayout);
  wrapper->setMargin(0);
  m_sqlitepart->setLayout(wrapper);

  label = new QLabel(i18n("Database File:") );
  sqlitelayout->addWidget(label);
  sqlitelayout->setMargin(0);

  m_leFile = new KUrlRequester;
  connect( m_leFile, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotDbCredentialsChanged( const QString& ) ) );
  sqlitelayout->addWidget(m_leFile);
  wrapper->addStretch(1);

  QVBoxLayout *vboxLay = new QVBoxLayout;
  KPageWidgetItem *topFrame = addPage( topWidget, i18n( "Database" ) );
  topFrame->setIcon( KIcon( "network-server-database" ) );

  QHBoxLayout *databasedriver = new QHBoxLayout;
  vboxLay->addLayout( databasedriver );

  label = new QLabel(i18n("Database Driver:") );
  databasedriver->addWidget(label);

  m_databaseDriver = new QComboBox;
  m_databaseDriver->addItem("SQLite");
  m_databaseDriver->addItem("MySQL");

  databasedriver->addWidget(m_databaseDriver);

  m_databaseconfigparts = new QStackedWidget;
  m_databaseconfigparts->addWidget(m_sqlitepart);
  m_databaseconfigparts->addWidget(m_mysqlpart);

  vboxLay->addWidget(m_databaseconfigparts);

  connect( m_databaseDriver, SIGNAL(currentIndexChanged(int)),
           m_databaseconfigparts, SLOT(setCurrentIndex(int)));

  QLabel *l1 = new QLabel(  i18n( "Please restart Kraft after "
                                  "changes in the database connection "
                                  "parameters to make the changes "
                                  "effective!" ) );
  l1->setTextFormat( Qt::RichText );
  l1->setAutoFillBackground(true);
  QPalette palette;
  palette.setColor(l1->backgroundRole(), QColor( "#ff6666"));
  l1->setPalette(palette);
  l1->setFrameStyle( QFrame::Box + QFrame::Raised );
  l1->setLineWidth( 1 );
  l1->setMargin( 5 );
  l1->setAlignment(Qt::AlignCenter);
  l1->setWordWrap(true);
  vboxLay->addWidget(l1);

  vboxLay->addStretch(2);

  topWidget->setLayout( vboxLay );
}

void PrefsDialog::taxTab()
{
  QWidget *topWidget = new QWidget;

  KPageWidgetItem *topFrame = addPage( topWidget, i18n( "Taxes" ));

  topFrame->setIcon(KIcon( "accessories-text-editor" ) );

  QVBoxLayout *vboxLay = new QVBoxLayout;
  vboxLay->setSpacing( spacingHint() );

  QLabel *label;
  label = new QLabel(i18n("Tax rates beginning at date:"));
  vboxLay->addWidget( label );

  mTaxModel = new QSqlTableModel(this);
  mTaxModel->setTable("taxes");
  mTaxModel->setSort(3, Qt::DescendingOrder);
  mTaxModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mTaxModel->select();
  mTaxModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
  mTaxModel->setHeaderData(1, Qt::Horizontal, tr("Full Tax [%]"));
  mTaxModel->setHeaderData(2, Qt::Horizontal, tr("Reduced Tax [%]"));
  mTaxModel->setHeaderData(3, Qt::Horizontal, tr("Start Date"));

  mTaxTreeView = new ImpTreeView;
  vboxLay->addWidget( mTaxTreeView );
  mTaxTreeView->setModel(mTaxModel);
  mTaxTreeView->setItemDelegate(new TaxItemDelegate());
  mTaxTreeView->hideColumn(0);
  mTaxTreeView->header()->moveSection(3, 1);
  mTaxTreeView->header()->stretchLastSection();
  mTaxTreeView->setColumnWidth(3, 200);
  mTaxTreeView->resizeColumnToContents(2);
  mTaxTreeView->resizeColumnToContents(1);

  connect( mTaxTreeView, SIGNAL(clicked(QModelIndex)),
           SLOT( slotTaxSelected(QModelIndex) ) );

  QHBoxLayout *butLay = new QHBoxLayout;
  butLay->addStretch( 1 );

  KPushButton *but = new KPushButton( KIcon("list-add"), i18n( "Add" ));
  connect( but, SIGNAL( clicked() ), SLOT( slotAddTax() ) );
  butLay->addWidget( but );

  mDelTax = new KPushButton( KIcon("list-remove"), i18n( "Remove" ) );
  connect( mDelTax, SIGNAL( clicked() ), SLOT( slotDeleteTax() ) );
  butLay->addWidget( mDelTax );
  mDelTax->setEnabled( false );

  vboxLay->addLayout( butLay );
  topWidget->setLayout( vboxLay );
}

void PrefsDialog::slotBrowse()
{

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
    int row = mTaxTreeView->currentIndex().row();
    //mTaxTreeView->setRowHidden( row, mTaxTreeView->rootIndex(), true );
    mTaxModel->removeRows(row, 1);
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
  topFrame->setIcon(KIcon( "edit-copy" ) );

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
  topFrame->setIcon(KIcon( "folder-documents" ) );

  QVBoxLayout *vboxLay = new QVBoxLayout;
  topWidget->setLayout(vboxLay);
  vboxLay->setSpacing( 0 ); // spacingHint() );

  mDocTypeEdit = new DocTypeEdit;
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
    if(KatalogSettings::self()->dbDriver() == "QSQLITE")
        m_databaseDriver->setCurrentIndex(0);
    else if(KatalogSettings::self()->dbDriver() == "QMYSQL")
        m_databaseDriver->setCurrentIndex(1);

    m_leHost->setText( KatalogSettings::self()->dbServerName() );
    m_leName->setText( KatalogSettings::self()->dbDatabaseName() );
    m_leUser->setText( KatalogSettings::self()->dbUser() );
    m_lePasswd->setText( KatalogSettings::self()->dbPassword() );
    m_leFile->setText( KatalogSettings::self()->dbFile() );

    mCbDocLocale->setChecked( KraftSettings::self()->showDocumentLocale() );

    QString t = KraftSettings::self()->doctype();
    if ( t.isEmpty() ) t = DefaultProvider::self()->docType();

    mCbDocTypes->setCurrentIndex( mCbDocTypes->findText( t ));

    mCbDefaultTaxType->setCurrentIndex( KraftSettings::self()->defaultTaxType()-1 );
}

void PrefsDialog::writeConfig()
{
    if(m_databaseDriver->currentIndex() == 0)
        KatalogSettings::self()->setDbDriver("QSQLITE");
    else if(m_databaseDriver->currentIndex() == 1)
        KatalogSettings::self()->setDbDriver("QMYSQL");

    KatalogSettings::self()->setDbServerName(m_leHost->text());
    KatalogSettings::self()->setDbDatabaseName(m_leName->text());
    KatalogSettings::self()->setDbUser(m_leUser->text());
    KatalogSettings::self()->setDbPassword( m_lePasswd->text());
    KatalogSettings::self()->setDbFile( m_leFile->text());
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

  QSqlDatabase check;
  check = QSqlDatabase::addDatabase( "QMYSQL" );
  check.setHostName( m_leHost->text() );
  check.setDatabaseName( m_leName->text() );
  check.setUserName( m_leUser->text() );
  check.setPassword( m_lePasswd->text() );

  check.open();

  bool x = check.isOpen();

  kDebug() << "Connection result: " << x << endl;
  if ( x == true ) {
    m_statusLabel->setText( i18n( "<font color='green'>Good!</font>" ) );
  } else {
    m_statusLabel->setText( i18n( "<font color='red'>Failed</font><br>") + check.lastError().text()  );
  }
}

void PrefsDialog::accept()
{
  mDocTypeEdit->saveDocTypes();
  writeTaxes();
  writeConfig();
  QDialog::accept();
}

TaxItemDelegate::TaxItemDelegate(QObject * parent) : QItemDelegate(parent) {}

void TaxItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  if(index.column() == 1 || index.column() == 2)
  {
    double percentage = index.data(Qt::DisplayRole).toDouble();
    QString string = DefaultProvider::self()->locale()->formatNumber(QString::number(percentage), true, 1);
    drawDisplay(painter, option, option.rect, string);
  }
  else if(index.column() == 3)
  {
    QDate date = index.data(Qt::DisplayRole).toDate();
    QString string = DefaultProvider::self()->locale()->formatDate(date);
    drawDisplay(painter, option, option.rect, string);
  }
  else
  {
    QItemDelegate::paint(painter, option, index);
  }
}



#include "prefsdialog.moc"

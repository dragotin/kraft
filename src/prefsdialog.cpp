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
#include <QCheckBox>
#include <QSpinBox>
#include <QToolTip>
#include <QPalette>
#include <QSqlTableModel>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QHeaderView>
#include <QDialog>
#include <QMessageBox>

#include <QDebug>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "prefsdialog.h"
#include "prefswages.h"
#include "prefsunits.h"
#include "databasesettings.h"
#include "kraftsettings.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include "taxeditdialog.h"
#include "documentman.h"
#include "impviewwidgets.h"
#include "texttemplate.h"
#include "htmlview.h"
#include "addressselectordialog.h"
#include "addressprovider.h"
#include "format.h"

#include "kcontacts/vcardconverter.h"

// ################################################################################

PrefsDialog::PrefsDialog( QWidget *parent)
    :QDialog( parent )
{
  setModal( true );
  setWindowTitle( i18n( "Configure Kraft" ) );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QHBoxLayout *mainLayout = new QHBoxLayout;

  _navigationBar = new QListWidget(this);
  _navigationBar->setViewMode(QListView::IconMode);
  _navigationBar->setIconSize(QSize(96, 64));
  _navigationBar->setMovement(QListView::Static);
  //_navigationBar->setSpacing(6);
  _navigationBar->setCurrentRow(0);
  _navigationBar->setFixedWidth(195);

  setLayout(mainLayout);

  QVBoxLayout *vbox = new QVBoxLayout;
  _pagesWidget = new QStackedWidget(this);
  vbox->addWidget( _pagesWidget );

  vbox->addWidget( buttonBox );

  mainLayout->addWidget(_navigationBar);

  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addLayout(vbox);
  okButton->setDefault(true);
  setMinimumWidth(700);

  _maxNavBarTextWidth = 0;

  addDialogPage( docTab(), QIcon::fromTheme( "edit-copy"), i18n( "Document Defaults" ));
  addDialogPage( taxTab(), QIcon::fromTheme( "accessories-text-editor" ), i18n("Taxes"));
  addDialogPage( doctypeTab(), QIcon::fromTheme( "folder-documents"), i18n( "Document Types" ));
  mPrefsWages = new PrefsWages(this);
  addDialogPage(mPrefsWages, QIcon::fromTheme( "help-donate" ), i18n( "Wages" ));
  mPrefsUnits = new PrefsUnits(this);
  addDialogPage(mPrefsUnits, QIcon::fromTheme( "chronometer" ), i18n("Units"));
  addDialogPage( whoIsMeTab(), QIcon::fromTheme( "user-identity" ), i18n( "Own Identity" ));

  readConfig();


  connect( _navigationBar, &QListWidget::itemClicked,
           this, &PrefsDialog::changePage);
}

void PrefsDialog::changePage(QListWidgetItem *current)
{
  if (!current)
      return;

  int indx = _navigationBar->row(current);
  _pagesWidget->setCurrentIndex(indx);
}

int PrefsDialog::addDialogPage( QWidget *w, const QIcon& icon, const QString& title)
{
    QListWidgetItem *listWidgetItem = new QListWidgetItem(_navigationBar);
    listWidgetItem->setIcon(icon);
    listWidgetItem->setText(title);
    listWidgetItem->setTextAlignment(Qt::AlignCenter);
    listWidgetItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    listWidgetItem->setSizeHint( QSize(170, 100));

    _navigationBar->addItem(listWidgetItem);

    QWidget *w_with_title = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(QStringLiteral("<h1>")+title+QStringLiteral("</h1>")));
    w_with_title->setLayout(layout);
    layout->addWidget(w);
    int indx = _pagesWidget->addWidget(w_with_title);

    return indx;
}

QWidget* PrefsDialog::taxTab()
{
  QWidget *topWidget = new QWidget;

  QVBoxLayout *vboxLay = new QVBoxLayout;
  // vboxLay->setSpacing( spacingHint() );

  QLabel *label;
  label = new QLabel(i18n("Tax rates beginning at date:"));
  vboxLay->addWidget( label );

  mTaxModel = new QSqlTableModel(this);
  mTaxModel->setTable("taxes");
  mTaxModel->setSort(3, Qt::DescendingOrder);
  mTaxModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mTaxModel->select();
  mTaxModel->setHeaderData(0, Qt::Horizontal, i18n("ID"));
  mTaxModel->setHeaderData(1, Qt::Horizontal, i18n("Full Tax [%]"));
  mTaxModel->setHeaderData(2, Qt::Horizontal, i18n("Reduced Tax [%]"));
  mTaxModel->setHeaderData(3, Qt::Horizontal, i18n("Start Date"));

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

  QPushButton *but = new QPushButton( QIcon::fromTheme("list-add"), i18n( "Add" ));
  connect( but, SIGNAL( clicked() ), SLOT( slotAddTax() ) );
  butLay->addWidget( but );

  mDelTax = new QPushButton( QIcon::fromTheme("list-remove"), i18n( "Remove" ) );
  connect( mDelTax, SIGNAL( clicked() ), SLOT( slotDeleteTax() ) );
  butLay->addWidget( mDelTax );
  mDelTax->setEnabled( false );

  vboxLay->addLayout( butLay );
  topWidget->setLayout( vboxLay );

  return topWidget;
}

QWidget* PrefsDialog::whoIsMeTab()
{
  QWidget *topWidget = new QWidget;

  QVBoxLayout *vboxLay = new QVBoxLayout;

  QLabel *label = new QLabel(i18n("Select the identity of the sending entity of documents. That's <b>your companies</b> address."));
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  vboxLay->addWidget( label );

  _tabWidget = new QTabWidget;
  vboxLay->addWidget(_tabWidget);

  // == Tab that displays the Addressbook widget
  QWidget *w = new QWidget;
  QVBoxLayout *t1Lay = new QVBoxLayout;
  mIdentityView = new HtmlView;
  mIdentityView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  t1Lay->addWidget(mIdentityView);
  QHBoxLayout *butLay = new QHBoxLayout;
  butLay->addStretch( 1 );
  _pbChangeIdentity = new QPushButton(i18n("Select Identity..."));
  connect( _pbChangeIdentity, SIGNAL(clicked()), SLOT(slotChangeIdentity()) );
  butLay->addWidget(_pbChangeIdentity);
  t1Lay->addLayout( butLay );

  w->setLayout(t1Lay);
  _tabWidget->insertTab(0, w, i18n("From AddressBook"));

  // == Tab that displays the manual widget
  QWidget *w1 = new QWidget;
  ui.setupUi(w1);
  _tabWidget->insertTab(1, w1, QIcon(), i18n("Manual Entry"));
  ui.nameLabel->setText( KContacts::Addressee::formattedNameLabel() );
  ui.orgLabel->setText( KContacts::Addressee::organizationLabel());
  ui.streetLabel->setText(KContacts::Addressee::businessAddressStreetLabel());
  ui.postCodeLabel->setText(KContacts::Addressee::businessAddressPostalCodeLabel());
  ui.cityLabel->setText(KContacts::Addressee::businessAddressLocalityLabel());
  ui.phoneLabel->setText(KContacts::Addressee::businessPhoneLabel());
  ui.faxLabel->setText(KContacts::Addressee::businessFaxLabel());
  ui.mobileLabel->setText(KContacts::Addressee::mobilePhoneLabel());
  ui.emailLabel->setText(KContacts::Addressee::emailLabel());
  ui.websiteLabel->setText(KContacts::Addressee::urlLabel());

  _tabWidget->insertTab(1, w1, i18n("Manual Address"));

  topWidget->setLayout( vboxLay );

  return topWidget;
}

void PrefsDialog::slotChangeIdentity()
{
  AddressSelectorDialog dialog(this);

  if( dialog.exec() ) {
    _newIdentity = dialog.addressee();
    if( ! _newIdentity.isEmpty() ) {
      setMyIdentity(_newIdentity, true);
    }
  }
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

QWidget* PrefsDialog::docTab()
{
  QLabel *label;
  QWidget *topWidget = new QWidget;

  QVBoxLayout *vboxLay = new QVBoxLayout;
  topWidget->setLayout( vboxLay );
  QGridLayout *topLayout = new QGridLayout;
  vboxLay->addLayout( topLayout );

  label = new QLabel(i18n("&Default document type on creation:"), this );
  topLayout->addWidget(label, 0,0);

  mCbDocTypes = new QComboBox;
  label->setBuddy( mCbDocTypes );
  mCbDocTypes->setToolTip( i18n( "New documents default to the selected type." ) );
  topLayout->addWidget( mCbDocTypes, 0, 1 );
  mCbDocTypes->insertItems(-1, DocType::allLocalised() );

  QLabel *f = new QLabel(this);
  f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  vboxLay->addWidget( f );

  QHBoxLayout *butLay = new QHBoxLayout;
  QLabel *l = new QLabel( i18n( "Default &Tax for Documents:" ), this );
  butLay->addWidget( l );
  mCbDefaultTaxType = new QComboBox(this);
  butLay->addWidget( mCbDefaultTaxType );
  l->setBuddy( mCbDefaultTaxType );
  mCbDefaultTaxType->setToolTip( i18n( "The default tax setting for all documents." ) );
  mCbDefaultTaxType->insertItem( 0, i18n("Display no tax at all"));
  mCbDefaultTaxType->insertItem( 1, i18n("Calculate reduced tax for all items" ));
  mCbDefaultTaxType->insertItem( 2, i18n("Calculate full tax for all items" ) );
  // mCbDefaultTaxType->insertItem( 3, i18n("Calculate on individual item tax rate" ));
  vboxLay->addLayout( butLay );

  f = new QLabel(this);
  f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  vboxLay->addWidget( f );

  butLay = new QHBoxLayout;
  l = new QLabel( i18n( "Document Date Format:" ), this );
  butLay->addWidget( l );
  mCbDateFormats = new QComboBox(this);
  butLay->addWidget( mCbDateFormats );
  l->setBuddy( mCbDateFormats);
  const QDate d = QDate::currentDate();
  mCbDateFormats->setToolTip( i18n( "The default date format for documents." ) );
  QString formattedDate = d.toString(Qt::ISODate);
  mCbDateFormats->insertItem( 0, i18n("ISO-Format: %1").arg(formattedDate));
  formattedDate = d.toString(Qt::DefaultLocaleShortDate);
  mCbDateFormats->insertItem( 1, i18n("Short-Date: %1").arg(formattedDate));
  formattedDate = d.toString(Qt::DefaultLocaleLongDate);
  mCbDateFormats->insertItem( 2, i18n("Long-Date: %1").arg(formattedDate));
  formattedDate = d.toString(Qt::RFC2822Date);
  mCbDateFormats->insertItem( 3, i18n("RFC 2822-Format: %1").arg(formattedDate));
  formattedDate = d.toString("dd.MM.yyyy");
  mCbDateFormats->insertItem( 4, i18n("\"German Format\": %1").arg(formattedDate));
  mCbDateFormats->insertItem( 5, i18n("Custom Setting in Settingsfile"));
  vboxLay->addLayout( butLay );

  // space eater
  QWidget *spaceEater = new QWidget;
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
  vboxLay->addWidget( spaceEater );

    return topWidget;
}

QWidget* PrefsDialog::doctypeTab()
{
  QWidget *topWidget = new QWidget;

  QVBoxLayout *vboxLay = new QVBoxLayout;
  topWidget->setLayout(vboxLay);
  vboxLay->setSpacing( 0 ); // spacingHint() );

  mDocTypeEdit = new DocTypeEdit;
  vboxLay->addWidget( mDocTypeEdit );

  connect( mDocTypeEdit, SIGNAL( removedType( const QString& ) ),
           SLOT( slotDocTypeRemoved( const QString& ) ) );

  return topWidget;
}

void PrefsDialog::slotDocTypeRemoved( const QString& type )
{
  // check if the default document type is still there
  QString currDefault = mCbDocTypes->currentText();

  if ( currDefault == type ) {
      QMessageBox msgBox;
      msgBox.setText(i18n( "The old default doc type for new documents was just deleted."
                           "Please check the setting in the Document Defaults in the "
                           "Kraft preferences Dialog." ));
      msgBox.setInformativeText(i18n("Document Default Change"));
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setDefaultButton(QMessageBox::Ok);
      msgBox.exec();
  }


  for ( int i=0; i < mCbDocTypes->count(); i++ ) {
    if ( mCbDocTypes->itemText( i ) == type ) {
      mCbDocTypes->removeItem( i );
      continue;
    }
  }
}

void PrefsDialog::readConfig()
{
    QString t = KraftSettings::self()->doctype();
    if ( t.isEmpty() ) t = DefaultProvider::self()->docType();

    mCbDocTypes->setCurrentIndex( mCbDocTypes->findText( t ));

    mCbDefaultTaxType->setCurrentIndex( KraftSettings::self()->defaultTaxType()-1 );

    int index {5};
    const QString dFormat = KraftSettings::self()->dateFormat();
    if (dFormat == Format::DateFormatIso) {
        // iso
        index = 0;
    } else if (dFormat == Format::DateFormatShort) {
        // short
        index = 1;
    } else if (dFormat == Format::DateFormatLong) {
        // long
        index = 2;
    } else if (dFormat == Format::DateFormatRFC) {
        // RFC
        index = 3;
    } else if (dFormat == Format::DateFormatGerman) {
        // German
        index = 4;
    }

    if (index == 5 && dFormat.isEmpty()) { // default case - no entry
        // HACK: If it is german, choose the "german" format
        const QString ln = DefaultProvider::self()->locale()->name();
        if( ln == QStringLiteral("de_DE")) {
            index = 4;
        } else {
            index = 1; // Short locale aware.
        }
    }

    mCbDateFormats->setCurrentIndex(index);
}

void PrefsDialog::writeIdentity()
{
    /*
     * Save either the manually added address, or the Addressbook-ID
     * If the user fills in the manual form, the addressbook ID is removed.
     * FIXME: The handling of the ownIdentity should be refactored to its
     * own class.
     */

    if(_tabWidget->currentIndex() == 1 /* manually entered */ ) {
        KContacts::Addressee add;
        add.setFormattedName(ui.leName->text());
        add.setOrganization(ui.leOrganization->text());
        KContacts::Address workAddress;

        workAddress.setStreet(ui.leStreet->text());
        workAddress.setPostalCode(ui.lePostcode->text());
        workAddress.setLocality(ui.leCity->text());
        workAddress.setType(KContacts::Address::Work);
        add.insertAddress(workAddress);

        add.insertPhoneNumber(PhoneNumber(ui.lePhone->text(), KContacts::PhoneNumber::Work));
        add.insertPhoneNumber(PhoneNumber(ui.leFax->text(), KContacts::PhoneNumber::Fax));
        add.insertPhoneNumber(PhoneNumber(ui.leMobile->text(), KContacts::PhoneNumber::Cell));
        ResourceLocatorUrl resUrl;
        resUrl.setUrl(QUrl(ui.leWebsite->text()));
        add.setUrl(resUrl);
        add.insertEmail(ui.leEmail->text(), true /* prefered */ );

        VCardConverter vcc;
        QByteArray vcard = vcc.createVCard(add);

        QString file = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
        file += "/myidentity.vcd";
        QFile f ( file );
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            f.write(vcard);
            f.close();
            qDebug() << "Saved own identity to " << file;

            KraftSettings::self()->setUserName( QString() );
            KraftSettings::self()->setUserUid( QString() );
            KraftSettings::self()->save();
        }
    } else { /* AddressBook */
        KraftSettings::self()->setUserName( _newIdentity.name() );
        KraftSettings::self()->setUserUid( _newIdentity.uid() );
        KraftSettings::self()->save();
    }
    emit newOwnIdentity(_newIdentity.uid(), _newIdentity);
}

void PrefsDialog::writeConfig()
{
    KraftSettings::self()->setDoctype( mCbDocTypes->currentText() );
    KraftSettings::self()->setDefaultTaxType( 1+mCbDefaultTaxType->currentIndex() );

    int dateFormat = mCbDateFormats->currentIndex();

    QString dateFormatString;
    if (dateFormat == 0) {
        // iso
        dateFormatString = Format::DateFormatIso;
    } else if (dateFormat == 1) {
        // short
        dateFormatString = Format::DateFormatShort;
    } else if (dateFormat == 2) {
        // long
        dateFormatString = Format::DateFormatLong;
    } else if (dateFormat == 3) {
        // RFC
        dateFormatString = Format::DateFormatRFC;
    } else if (dateFormat == 4) {
        // German
        dateFormatString = Format::DateFormatGerman;
    }

    if (dateFormatString.isEmpty()) {
        // do not touch!
    } else {
        KraftSettings::self()->setDateFormat(dateFormatString);
    }

    KraftSettings::self()->save();
}

void PrefsDialog::writeTaxes()
{
    mTaxModel->submitAll();
}

PrefsDialog::~PrefsDialog()
{
}

void PrefsDialog::accept()
{
  mDocTypeEdit->saveDocTypes();
  mPrefsWages->save();
  mPrefsUnits->save();
  writeTaxes();
  writeConfig();
  writeIdentity();
  QDialog::accept();
}

#define IDENTITY_TAG(X) QLatin1String(X)
#define QL1(X) QLatin1String(X)

void PrefsDialog::fillManualIdentityForm(const KContacts::Addressee& addressee)
{
    ui.leName->setText(addressee.formattedName());
    ui.leStreet->setText(addressee.address(Address::Work).street());
    ui.leCity->setText(addressee.address(Address::Work).locality());
    ui.lePostcode->setText(addressee.address(Address::Work).postalCode());

    ui.leEmail->setText(addressee.preferredEmail());
    ui.leFax->setText(addressee.phoneNumber(PhoneNumber::Fax).number());
    ui.leOrganization->setText(addressee.organization());
    ui.lePhone->setText(addressee.phoneNumber(PhoneNumber::Work).number());
    ui.leMobile->setText(addressee.phoneNumber(PhoneNumber::Cell).number());
    ui.leWebsite->setText(addressee.url().url().toDisplayString());
}

void PrefsDialog::setMyIdentity( const KContacts::Addressee& addressee, bool backendUp )
{
  // Note: This code is stolen from DocDigestDetailView::slotShowDocDetails
  // It should be refactored.

    const QString tmplFile = DefaultProvider::self()->locateFile( "views/identity.thtml" );
  TextTemplate tmpl;
  tmpl.setTemplateFileName(tmplFile);
  if( !tmpl.isOk() ) {
      return;
  }
  if( ! tmpl.errorString().isEmpty() ) {
    mIdentityView->displayContent( QString("<h1>Unable to find template <i>identity.trml</i></h1><p>%1</p>")
                                   .arg(tmpl.errorString()));
    return;
  }

  QString addressBookInfo;
  _pbChangeIdentity->setEnabled(backendUp);
  if( !backendUp ) {
      addressBookInfo = i18n("The identity can not be found.");
      tmpl.createDictionary(QL1("NO_IDENTITY"));
      tmpl.setValue(QL1("NO_IDENTITY_WRN"), i18n("<p><b>Kraft Addressbook Integration down.</b></p>"
                                             "<p>The address book backend is not up and running.</p>"
                                             "<p>Please check your addressbook integration setup.</p>"));
  }

  if( addressee.isEmpty() ) {
      addressBookInfo = i18n("The identity is not listed in an address book.");
      tmpl.createDictionary(QL1("NO_IDENTITY"));
      tmpl.setValue(QL1("NO_IDENTITY_WRN"), i18n("<p><b>Kraft does not know your identity.</b></p>"
                                                 "<p>Please pick one from the address books by clicking on the Button below.</p>"
                                                 "<p>Not having an identity selected can make your documents look incomplete.</p>"));
  } else {
      const QString origin = addressee.custom( CUSTOM_ADDRESS_MARKER );
      if( origin.isEmpty() || origin == "manual") {
          // it is an manually added address.
          fillManualIdentityForm(addressee);
          _tabWidget->setTabIcon(1, QIcon::fromTheme("checkmark"));
          _tabWidget->setTabIcon(0, QIcon());
          _tabWidget->setCurrentIndex(1);
      } else {
          _tabWidget->setTabIcon(0, QIcon::fromTheme("checkmark"));
          _tabWidget->setTabIcon(1, QIcon());
          _tabWidget->setCurrentIndex(0);

          // it is an address from the address book
          addressBookInfo  = i18n("Your identity can be found in the address books.");
          tmpl.createDictionary(QL1("IDENTITY"));
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("IDENTITY_NAME"), addressee.realName() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("IDENTITY_ORGANISATION"), addressee.organization() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("IDENTITY_URL"), addressee.url().toString() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("IDENTITY_EMAIL"), addressee.preferredEmail() );

          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("IDENTITY_WORK_PHONE"), addressee.phoneNumber(PhoneNumber::Work).number());
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("IDENTITY_MOBILE_PHONE"), addressee.phoneNumber(PhoneNumber::Cell).number());
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("IDENTITY_FAX"), addressee.phoneNumber(PhoneNumber::Fax).number());

          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("WORK_PHONE_LABEL"), i18n("Work Phone") );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("FAX_LABEL"), i18n("Fax") );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("MOBILE_PHONE_LABEL"), i18n("Cell Phone") );
          KContacts::Address myAddress;
          myAddress = addressee.address( KContacts::Address::Pref );
          QString addressType = i18n("preferred address");

          if( myAddress.isEmpty() ) {
              myAddress = addressee.address( KContacts::Address::Home );
              addressType = i18n("home address");
          }
          if( myAddress.isEmpty() ) {
              myAddress = addressee.address( KContacts::Address::Work );
              addressType = i18n("work address");
          }
          if( myAddress.isEmpty() ) {
              myAddress = addressee.address( KContacts::Address::Postal );
              addressType = i18n("postal address");
          }
          if( myAddress.isEmpty() ) {
              myAddress = addressee.address( KContacts::Address::Intl );
              addressType = i18n("international address");
          }
          if( myAddress.isEmpty() ) {
              myAddress = addressee.address( KContacts::Address::Dom );
              addressType = i18n("domestic address");
          }

          if( myAddress.isEmpty() ) {
              addressType = i18n("unknown");
              // qDebug () << "WRN: Address is still empty!";
          }

          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_POSTBOX" ),  myAddress.postOfficeBox() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_EXTENDED" ), myAddress.extended() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_STREET" ),   myAddress.street() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_LOCALITY" ), myAddress.locality() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_REGION" ),   myAddress.region() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_POSTCODE" ), myAddress.postalCode() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_COUNTRY" ),  myAddress.country() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_REGION" ),   myAddress.region() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_LABEL" ),    myAddress.label() );
          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG( "IDENTITY_ADDRESS_TYPE" ), QL1("(")+addressType+QL1(")") );

          tmpl.setValue( QL1("IDENTITY"), IDENTITY_TAG("ADDRESSBOOK_INFO"), addressBookInfo );
      }
  }

  mIdentityView->displayContent(tmpl.expand());
}

TaxItemDelegate::TaxItemDelegate(QObject * parent) : QItemDelegate(parent) {}

void TaxItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  if(index.column() == 1 || index.column() == 2)
  {
    double percentage = index.data(Qt::DisplayRole).toDouble();
    // QString string = DefaultProvider::self()->locale()->formatNumber(QString::number(percentage), true, 1);
    QString string = DefaultProvider::self()->locale()->toString(percentage);
    drawDisplay(painter, option, option.rect, string);
  }
  else if(index.column() == 3)
  {
    QDate date = index.data(Qt::DisplayRole).toDate();
    // QString string = DefaultProvider::self()->locale()->formatDate(date);
    QString string = DefaultProvider::self()->locale()->toString(date);
    drawDisplay(painter, option, option.rect, string);
  }
  else
  {
    QItemDelegate::paint(painter, option, index);
  }
}

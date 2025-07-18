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
#include <QFileDialog>
#include <QVBoxLayout>

#include "prefsdialog.h"
#include "prefswages.h"
#include "prefsunits.h"
#include "kraftsettings.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include "taxeditdialog.h"
#include "impviewwidgets.h"
#include "htmlview.h"
#include "addressselectordialog.h"
#include "addressprovider.h"
#include "format.h"
#include "positionviewwidget.h"
#include "myidentity.h"
#include "grantleetemplate.h"

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

  addDialogPage( docTab(), DefaultProvider::self()->icon( "copy"), i18n( "Document Defaults" ));
  addDialogPage( taxTab(), DefaultProvider::self()->icon( "receipt-tax" ), i18n("Taxes"));
  addDialogPage( doctypeTab(), DefaultProvider::self()->icon( "files"), i18n( "Document Types" ));
  mPrefsWages = new PrefsWages(this);
  addDialogPage(mPrefsWages, DefaultProvider::self()->icon( "cash-banknote" ), i18n( "Wages" ));
  mPrefsUnits = new PrefsUnits(this);
  addDialogPage(mPrefsUnits, DefaultProvider::self()->icon( "atom" ), i18n("Units"));
  _whoIndx = addDialogPage( whoIsMeTab(), DefaultProvider::self()->icon( "id-badge-2" ), i18n( "Own Identity" ));

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

  QPushButton *but = new QPushButton( DefaultProvider::self()->icon("plus"), i18n( "Add" ));
  connect( but, SIGNAL( clicked() ), SLOT( slotAddTax() ) );
  butLay->addWidget( but );

  mDelTax = new QPushButton( DefaultProvider::self()->icon("minus"), i18n( "Remove" ) );
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
  _pbChangeIdentity = new QPushButton(i18n("Select Identity…"));

  connect( _pbChangeIdentity, &QPushButton::clicked, this, &PrefsDialog::slotChangeIdentity);

  butLay->addWidget(_pbChangeIdentity);
  t1Lay->addLayout( butLay );

  w->setLayout(t1Lay);
  _tabWidget->insertTab(0, w, i18n("From Address Book"));

  // == Tab that displays the manual widget
  QWidget *w1 = new QWidget;
  _ownIdentUi.setupUi(w1);
  _tabWidget->insertTab(1, w1, QIcon(), i18n("Manual Entry"));
  _ownIdentUi.nameLabel->setText( KContacts::Addressee::formattedNameLabel() );
  _ownIdentUi.orgLabel->setText( KContacts::Addressee::organizationLabel());
  _ownIdentUi.streetLabel->setText(KContacts::Addressee::businessAddressStreetLabel());
  _ownIdentUi.postCodeLabel->setText(KContacts::Addressee::businessAddressPostalCodeLabel());
  _ownIdentUi.cityLabel->setText(KContacts::Addressee::businessAddressLocalityLabel());
  _ownIdentUi.phoneLabel->setText(KContacts::Addressee::businessPhoneLabel());
  _ownIdentUi.faxLabel->setText(KContacts::Addressee::businessFaxLabel());
  _ownIdentUi.mobileLabel->setText(KContacts::Addressee::mobilePhoneLabel());
  _ownIdentUi.emailLabel->setText(KContacts::Addressee::emailLabel());
  _ownIdentUi.websiteLabel->setText(KContacts::Addressee::urlLabel());

  _tabWidget->insertTab(1, w1, i18n("Manual Address"));

  // == Bank Account information
  QGroupBox *gbox = new QGroupBox(i18n("Bank Account Information"), this);
  QFormLayout *formLayout = new QFormLayout;
  _bacName = new QLineEdit(this);
  formLayout->addRow(tr("&Bank Account Holder:"), _bacName);
  _bacIBAN = new QLineEdit(this);
  formLayout->addRow(tr("&IBAN:"), _bacIBAN);
  _bacBIC = new QLineEdit(this);
  formLayout->addRow(tr("&BIC:"), _bacBIC);
  gbox->setLayout(formLayout);
  vboxLay->addWidget(gbox);

  topWidget->setLayout( vboxLay );

  return topWidget;
}

void PrefsDialog::slotChangeIdentity()
{
  AddressSelectorDialog dialog(this);

  if( dialog.exec() ) {
    _newOwnAddress = dialog.addressee();
    if( ! _newOwnAddress.isEmpty() ) {
      displayOwnAddress(_newOwnAddress, true);
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
  QLocale locale;
  mCbDateFormats->insertItem( 0, i18n("ISO-Format: %1", formattedDate));
  formattedDate = d.toString(locale.dateFormat(QLocale::ShortFormat));
  mCbDateFormats->insertItem( 1, i18n("Short-Date: %1", formattedDate));
  formattedDate = d.toString(locale.dateFormat(QLocale::LongFormat));
  mCbDateFormats->insertItem( 2, i18n("Long-Date: %1", formattedDate));
  formattedDate = d.toString(Qt::RFC2822Date);
  mCbDateFormats->insertItem( 3, i18n("RFC 2822-Format: %1", formattedDate));
  formattedDate = d.toString("dd.MM.yyyy");
  mCbDateFormats->insertItem( 4, i18n("\"German Format\": %1", formattedDate));
  mCbDateFormats->insertItem( 5, i18n("Custom Setting in Settingsfile"));
  vboxLay->addLayout( butLay );

  // ---- Alternative- and Demand Text
  f = new QLabel(this);
  f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  vboxLay->addWidget( f );

  auto gridLay = new QGridLayout;
  l = new QLabel(i18n("Prefix text for Demand items:"), this );
  gridLay->addWidget(l, 0, 0);

  _lineEditDemandText = new QLineEdit(this);
  _lineEditDemandText->setText(PositionViewWidget::kindLabel(DocPosition::Type::Demand));
  _lineEditDemandText->setToolTip(i18n("This text is automatically prepended to new 'on demand' items."));

  gridLay->addWidget(_lineEditDemandText, 0, 1);

  l = new QLabel(i18n("Prefix text for Alternative items:"), this );
  gridLay->addWidget(l, 1, 0);

  _lineEditAlternativeText = new QLineEdit(this);
  _lineEditAlternativeText->setText(PositionViewWidget::kindLabel(DocPosition::Type::Alternative));
  _lineEditAlternativeText->setToolTip(i18n("This text is automatically prepended to new 'alternative' items."));

  gridLay->addWidget(_lineEditAlternativeText, 1, 1);
  vboxLay->addLayout( gridLay );

  // ---- XRechnung Template
  f = new QLabel(this);
  f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  vboxLay->addWidget( f );

  butLay = new QHBoxLayout;
  l = new QLabel(i18n("XRechnung template file:"), this );
  butLay->addWidget(l);

  _lineEditXRechnung = new QLineEdit(this);

  butLay->addWidget(_lineEditXRechnung);
  QPushButton *pbXRechTmpl = new QPushButton(i18n("Select…"), this);
  butLay->addWidget(pbXRechTmpl);

  const QIcon& icon = DefaultProvider::self()->icon("device-floppy");
  if (!icon.isNull() ) {
      pbXRechTmpl->setIcon(icon);
      pbXRechTmpl->setText("");
  }
  pbXRechTmpl->setToolTip(i18n("Select template file for XRechnung"));

  connect(pbXRechTmpl, &QPushButton::clicked, this, [this]() {
      const QString file = QFileDialog::getOpenFileName(this,
                                                        i18n("Find Template File"), QDir::homePath(),
                                                        i18n("XRechnung Templates (*.xrtmpl)"));

      if (!file.isEmpty()) {
          _lineEditXRechnung->setText(file);
      }
  });
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

    DocType dt(QStringLiteral("Rechnung")); // FIXME
    const auto tmpl = dt.xRechnungTemplate();
    _lineEditXRechnung->setText(tmpl);

    // == Date format
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

    // == Bank Account Information
    QString h = KraftSettings::self()->bankAccountName();
    _bacName->setText(h);
    h = KraftSettings::self()->bankAccountBIC();
    _bacBIC->setText(h);
    h = KraftSettings::self()->bankAccountIBAN();
    _bacIBAN->setText(h);


}

void PrefsDialog::writeIdentity(int currIndx)
{
    /*
     * Save either the manually added address, or the Addressbook-ID
     * If the user fills in the manual form, the addressbook ID is removed.
     */

    if (currIndx ==_whoIndx) { // own Identity page
        bool isManualPage = (_tabWidget->currentIndex() == 1);
        if (isManualPage) {  // if it is the address book page, newOwnAddress is already set
            KContacts::Addressee add = MyIdentity::UIToAddressee(_ownIdentUi);

            if (!add.isEmpty()) {
                _newOwnAddress = add;
                _newOwnAddress.insertCustom(CUSTOM_ADDRESS_MARKER, "manual");
            }

        }
    }

    if (_newOwnAddress.isEmpty()) {
        // no need to save
        return;
    }

    const QString origin = _newOwnAddress.custom( CUSTOM_ADDRESS_MARKER );
    if( origin.isEmpty() || origin == "manual") {
        _myIdentity->save(QString(), _newOwnAddress);
    } else { /* AddressBook */        
        _myIdentity->save(_newOwnAddress.uid());
    }
}

void PrefsDialog::writeConfig()
{
    KraftSettings::self()->setDoctype( mCbDocTypes->currentText() );
    KraftSettings::self()->setDefaultTaxType( 1+mCbDefaultTaxType->currentIndex() );

    DocType dt(QStringLiteral("Rechnung")); // FIXME
    const auto newTmpl = _lineEditXRechnung->text();
    if (newTmpl != dt.xRechnungTemplate()) {
        dt.setXRechnungTemplate(newTmpl);
        dt.save();
    }

    const QString demandText = _lineEditDemandText->text();
    KraftSettings::self()->setDemandLabel(demandText);
    const QString alterText = _lineEditAlternativeText->text();
    KraftSettings::self()->setAlternativeLabel(alterText);

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

    QString h = _bacName->text();
    if (h != KraftSettings::self()->bankAccountName()) {
        KraftSettings::self()->setBankAccountName(h);
    }
    h = _bacBIC->text();
    if (h != KraftSettings::self()->bankAccountBIC()) {
        KraftSettings::self()->setBankAccountBIC(h);
    }
    h = _bacIBAN->text();
    if (h != KraftSettings::self()->bankAccountIBAN()) {
        KraftSettings::self()->setBankAccountIBAN(h);
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
    int currIndx = _pagesWidget->currentIndex();

    mDocTypeEdit->saveDocTypes();
    mPrefsWages->save();
    mPrefsUnits->save();
    writeTaxes();
    writeConfig();
    writeIdentity(currIndx);
    QDialog::accept();
}

#define IDENTITY_TAG(X) (X)
#define QL1(X) QStringLiteral(X)

void PrefsDialog::fillManualIdentityForm(const KContacts::Addressee& addressee)
{
    _ownIdentUi.leName->setText(addressee.formattedName());
    _ownIdentUi.leStreet->setText(addressee.address(Address::Work).street());
    _ownIdentUi.leCity->setText(addressee.address(Address::Work).locality());
    _ownIdentUi.lePostcode->setText(addressee.address(Address::Work).postalCode());

    _ownIdentUi.leEmail->setText(addressee.preferredEmail());
    _ownIdentUi.leFax->setText(addressee.phoneNumber(PhoneNumber::Fax).number());
    _ownIdentUi.leOrganization->setText(addressee.organization());
    _ownIdentUi.lePhone->setText(addressee.phoneNumber(PhoneNumber::Work).number());
    _ownIdentUi.leMobile->setText(addressee.phoneNumber(PhoneNumber::Cell).number());
    _ownIdentUi.leWebsite->setText(addressee.url().url().toDisplayString());
}

void PrefsDialog::setMyIdentity(MyIdentity *identity)
{
    _myIdentity = identity;
    displayOwnAddress(identity->contact(), identity->hasBackend());
}

void PrefsDialog::displayOwnAddress(const KContacts::Addressee& addressee, bool backendUp)
{
    // Note: This code is stolen from DocDigestDetailView::slotShowDocDetails
    // It should be refactored.

    const QString tmplFile = DefaultProvider::self()->locateFile( "views/identity.gtmpl" );
    GrantleeFileTemplate tmpl(tmplFile);
    if( !tmpl.isOk() ) {
        return;
    }
    bool ok;
    if( ! tmpl.isOk()) {
        mIdentityView->displayContent(QStringLiteral("<h1>Unable to find template <i>identity.gtmpl</i></h1>"));
        return;
    }

    QString addressBookInfo;
    _pbChangeIdentity->setEnabled(backendUp);

    QObject errorObj;
    QObject obj;

    obj.setProperty(IDENTITY_TAG("CSS_WARN_BACKGROUND_COLOR"), "#ea4e1d");

    if( !backendUp ) {
        addressBookInfo = i18n("The identity cannot be found.");
        errorObj.setProperty("NO_IDENTITY_WRN", i18n("<p><b>Kraft Addressbook Integration down.</b></p>"
                                                   "<p>The address book backend is not up and running.</p>"
                                                   "<p>Please check your addressbook integration setup.</p>"));
    }

    if( addressee.isEmpty() ) {
        addressBookInfo = i18n("The identity is not listed in an address book.");
        errorObj.setProperty("NO_IDENTITY_WRN", i18n("<p><b>Kraft does not know your identity.</b></p>"
                                                   "<p>Please pick one from the address books by clicking on the Button below.</p>"
                                                   "<p>Not having an identity selected can make your documents look incomplete.</p>"));
    } else {
        const QString origin = addressee.custom( CUSTOM_ADDRESS_MARKER );
        if( origin.isEmpty() || origin == "manual") {
            // it is an manually added address.
            fillManualIdentityForm(addressee);
            _tabWidget->setTabIcon(1, DefaultProvider::self()->icon("check"));
            _tabWidget->setTabIcon(0, QIcon());
            _tabWidget->setCurrentIndex(1);
        } else {
            _tabWidget->setTabIcon(0, DefaultProvider::self()->icon("check"));
            _tabWidget->setTabIcon(1, QIcon());
            _tabWidget->setCurrentIndex(0);

            // it is an address from the address book
            addressBookInfo  = i18n("Your identity can be found in the address books.");
            obj.setProperty( IDENTITY_TAG("IDENTITY_NAME"), addressee.realName() );
            obj.setProperty( IDENTITY_TAG("IDENTITY_ORGANISATION"), addressee.organization() );
            obj.setProperty( IDENTITY_TAG("IDENTITY_URL"), addressee.url().toString() );
            obj.setProperty( IDENTITY_TAG("IDENTITY_EMAIL"), addressee.preferredEmail() );

            obj.setProperty( IDENTITY_TAG("IDENTITY_WORK_PHONE"), addressee.phoneNumber(PhoneNumber::Work).number());
            obj.setProperty( IDENTITY_TAG("IDENTITY_MOBILE_PHONE"), addressee.phoneNumber(PhoneNumber::Cell).number());
            obj.setProperty( IDENTITY_TAG("IDENTITY_FAX"), addressee.phoneNumber(PhoneNumber::Fax).number());

            obj.setProperty( IDENTITY_TAG("WORK_PHONE_LABEL"), i18n("Work Phone") );
            obj.setProperty( IDENTITY_TAG("FAX_LABEL"), i18n("Fax") );
            obj.setProperty( IDENTITY_TAG("MOBILE_PHONE_LABEL"), i18n("Cell Phone") );
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

            obj.setProperty( IDENTITY_TAG( "IDENTITY_POSTBOX" ),  myAddress.postOfficeBox() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_EXTENDED" ), myAddress.extended() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_STREET" ),   myAddress.street() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_LOCALITY" ), myAddress.locality() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_REGION" ),   myAddress.region() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_POSTCODE" ), myAddress.postalCode() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_COUNTRY" ),  myAddress.country() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_REGION" ),   myAddress.region() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_LABEL" ),    myAddress.label() );
            obj.setProperty( IDENTITY_TAG( "IDENTITY_ADDRESS_TYPE" ), addressType);

            obj.setProperty( IDENTITY_TAG("ADDRESSBOOK_INFO"), addressBookInfo );
        }
    }

    tmpl.addToObjMapping("kraft", &obj);
    tmpl.addToObjMapping("error", &errorObj);

    const QString ex = tmpl.render(ok);
    qDebug() << ex;
    mIdentityView->displayContent(ex);
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

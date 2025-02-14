/***************************************************************************
    AddressSelectorWidget - Address Selection Widget based on Akonadi
                             -------------------
    begin                : Jul 2011
    copyright            : (C) 2011 by Klaas Freitag
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

#include "kraftsettings.h"
#include "addressselectorwidget.h"
#include "addressprovider.h"

#include <QtGui>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QLineEdit>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QDebug>

#include <klocalizedstring.h>

#include <kcontacts/addressee.h>
#include <kcontacts/contactgroup.h>

#include "defaultprovider.h"

#ifdef HAVE_AKONADI
#include <Akonadi/EntityTreeModel>
#include <Akonadi/EntityTreeView>
#else
#include "htmlview.h"
#include "grantleetemplate.h"
#include "documenttemplate.h"
#endif

/* ==================================================================== */
AddressSortProxyModel::AddressSortProxyModel(AddressProvider *provider, QObject *parent)
    : QSortFilterProxyModel(parent),
      _provider(provider)
{
    // setFilterRole(ActivityItemDelegate::ActionTextRole);
    setFilterKeyColumn(0);
}

static bool addressMatchesFilter(const KContacts::Address &address, const QString &filterString)
{
    if (address.street().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.locality().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.region().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.postalCode().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.country().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.label().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.postOfficeBox().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

static bool contactMatchesFilter(const KContacts::Addressee &contact, const QString &filterString)
{
    if (contact.assembledName().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.formattedName().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.nickName().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.birthday().toString().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    const KContacts::Address::List addresses = contact.addresses();
    int count = addresses.count();
    for (int i = 0; i < count; ++i) {
        if (addressMatchesFilter(addresses.at(i), filterString)) {
            return true;
        }
    }

    const KContacts::PhoneNumber::List phoneNumbers = contact.phoneNumbers();
    count = phoneNumbers.count();
    for (int i = 0; i < count; ++i) {
        if (phoneNumbers.at(i).number().contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    const QStringList emails = contact.emails();
    count = emails.count();
    for (int i = 0; i < count; ++i) {
        if (emails.at(i).contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    const QStringList categories = contact.categories();
    count = categories.count();
    for (int i = 0; i < count; ++i) {
        if (categories.at(i).contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    if (contact.mailer().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.title().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.role().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.organization().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.department().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.note().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.url().url().url().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    const QStringList customs = contact.customs();
    count = customs.count();
    for (int i = 0; i < count; ++i) {
        if (customs.at(i).contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

bool contactGroupMatchesFilter(const KContacts::ContactGroup &group, const QString &filterString)
{
    if (group.name().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    const uint count = group.dataCount();
    for (uint i = 0; i < count; ++i) {
        if (group.data(i).name().contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
        if (group.data(i).email().contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

bool AddressSortProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    KContacts::Addressee contact = _provider->getAddressee(row, parent);

    if( contact.isEmpty() ) {
        return true;
    } else {
        const QString p = filterRegularExpression().pattern();
        return contactMatchesFilter(contact, p );
    }

    return true;
}

QVariant AddressSortProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal &&
         role == Qt::DisplayRole) {
        if( section == 0 ) {
            return i18n("Name");
        } else if ( section == 1 ) {
            return i18n("Address");
        }
    }
    return QVariant();
}
/* ------------------------------------------------------------------------------ */

KraftContactViewer::KraftContactViewer(QWidget *parent)
    :QWidget(parent)
#ifdef HAVE_AKONADI
    , _contactViewer(0)
#endif
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setContentsMargins(0,0,0,0);
    setLayout(lay);
#ifdef HAVE_AKONADI
    _contactViewer = new Akonadi::ContactViewer;
    _contactViewer->setShowQRCode(false);
    lay->addWidget(_contactViewer);
#else
    _htmlView = new HtmlView(this);
    lay->addWidget(_htmlView);
#endif
}

void KraftContactViewer::setContact( const KContacts::Addressee& contact)
{
#ifdef HAVE_AKONADI
    _contactViewer->setRawContact(contact);
#else
    const QString templateFile = DefaultProvider::self()->locateFile( "views/contact.gtmpl" );

    GrantleeFileTemplate tmpl(templateFile);

    const auto contactHash = Template::contactToVariantHash(contact);
    tmpl.addToMappingHash("contact", contactHash);
    const auto labelHash = Template::labelVariantHash();
    tmpl.addToMappingHash("label", labelHash);

    bool ok;
    const QString rendered = tmpl.render(ok);
    _htmlView->setHtml(rendered);
#endif

}

/* ------------------------------------------------------------------------------ */

AddressSelectorWidget::AddressSelectorWidget(QWidget *parent, bool /* showText */)
    : QSplitter(parent),
      mButEditContact(nullptr),
      _provider(nullptr)
{
    setupUi();
    restoreState();
}


AddressSelectorWidget::~AddressSelectorWidget()
{
}

void AddressSelectorWidget::setupUi()
{
    _provider = new AddressProvider(this);

    setChildrenCollapsible(false);
    // Left page of the splitter
    QWidget *leftW = new QWidget;
    QVBoxLayout *leftLay = new QVBoxLayout;
    leftW->setLayout(leftLay);
    this->addWidget(leftW);

    QHBoxLayout *searchLay = new QHBoxLayout;
    leftLay->addLayout(searchLay);

    QLabel *searchLabel = new QLabel(i18n("&Search:"));
    searchLay->addWidget( searchLabel );
    QLineEdit *edit = new QLineEdit;
    searchLabel->setBuddy(edit);

    edit->setClearButtonEnabled(true);
    searchLay->addWidget( edit );
    connect(edit, SIGNAL(textChanged(QString)), SLOT(slotFilterTextChanged(QString)));

#ifdef HAVE_AKONADI
    _addressTreeView = new Akonadi::EntityTreeView( this );
#else
    _addressTreeView = new QTreeView;
#endif
    _addressTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftLay->addWidget(_addressTreeView);
    mProxyModel = new AddressSortProxyModel(_provider, this);
    mProxyModel->setSourceModel(_provider->model());
    _addressTreeView->setModel(mProxyModel);
    connect(_addressTreeView, &QTreeView::clicked, this, &AddressSelectorWidget::slotAddresseeSelected);

    mProxyModel->sort(0);

    // the right side
    QWidget *rightW = new QWidget;
    QVBoxLayout *rightLay = new QVBoxLayout;
    rightW->setLayout(rightLay);
    this->addWidget(rightW);
    _contactViewer = new KraftContactViewer(this);
    _contactViewer->setMinimumWidth(200);
    rightLay->addWidget(_contactViewer);

    // Buttons to create and edit
    QHBoxLayout *hboxBot = new QHBoxLayout;
    hboxBot->addStretch(4);
    rightLay->addLayout( hboxBot );
#ifdef HAVE_AKONADI
    mButEditContact = new QPushButton(i18n("Edit Contact…"));
    mButEditContact->setToolTip( i18n("Edit the currently selected contact" ));
    mButEditContact->setEnabled( false );
    hboxBot->addWidget( mButEditContact );
    QPushButton *butCreateContact = new QPushButton(i18n("New Contact…"));
    butCreateContact->setToolTip( i18n("Create a new Contact" ) );
    hboxBot->addWidget( butCreateContact );

    connect(butCreateContact,SIGNAL(clicked()),SLOT(slotCreateNewContact()));
    connect(mButEditContact, SIGNAL(clicked()),SLOT(slotEditContact()));
#else
    QLabel *lab = new QLabel(i18n("Addresses read from directory (read only)"), rightW);
    hboxBot->addWidget(lab);
#endif

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

}

void AddressSelectorWidget::slotFilterTextChanged( const QString& filter)
{
    // qDebug() << "Filter: " << filter;
    mProxyModel->setFilterRegularExpression(filter);
}

void AddressSelectorWidget::restoreState()
{
    const QList<int> sizes = KraftSettings::self()->addressPickerSplitterSize();
    setSizes(sizes);

    const QByteArray state = QByteArray::fromBase64( KraftSettings::self()->addressPickerTreeviewState().toLatin1() );
    _addressTreeView->header()->restoreState(state);

}

void AddressSelectorWidget::saveState()
{
    const QList<int> s = sizes();
    KraftSettings::self()->setAddressPickerSplitterSize(s);

    const QByteArray state = _addressTreeView->header()->saveState().toBase64();
    KraftSettings::self()->setAddressPickerTreeviewState(state);

}

bool AddressSelectorWidget::backendUp() const
{
    bool re = false;
    if( _provider ) {
        re = _provider->backendUp();
    }
    return re;
}

void AddressSelectorWidget::slotCreateNewContact()
{
#ifdef HAVE_AKONADI
    _addressEditor = new Akonadi::ContactEditorDialog(Akonadi::ContactEditorDialog::CreateMode, this );
    _addressEditor->show();
#endif
}

void AddressSelectorWidget::slotAddresseeSelected(QModelIndex index)
{
    if ( index.isValid() ) {
        QModelIndex sourceIdx = mProxyModel->mapToSource(index);
        KContacts::Addressee contact = _provider->getAddressee( sourceIdx );
        qDebug() << "----------- " << contact.formattedName() << contact.uid();
        _contactViewer->setContact(contact);

        Q_EMIT addressSelected(contact);

        if(mButEditContact)
            mButEditContact->setEnabled( true );
    } else {
        // qDebug () << "No address was selected!";
        if(mButEditContact)
            mButEditContact->setEnabled( false );
    }
}

void AddressSelectorWidget::slotEditContact()
{
#ifdef HAVE_AKONADI
  if( _addressTreeView->selectionModel()->hasSelection() ) {
      QModelIndex index = _addressTreeView->selectionModel()->currentIndex();
    if ( index.isValid() ) {
      const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      if ( item.isValid() && item.hasPayload<KContacts::Addressee>() ) {
        _addressEditor = new Akonadi::ContactEditorDialog(Akonadi::ContactEditorDialog::CreateMode, this );
        _addressEditor->setContact( item );
        _addressEditor->show();
      }
    }
  }
#endif
}


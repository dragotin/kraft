/***************************************************************************
              texteditdialog.cpp  - Edit document text templates
                             -------------------
    begin                : Apr 2007
    copyright            : (C) 2007 by Klaas Freitag
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

#include <QComboBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QSizePolicy>

#include <QDialog>
#include <QDebug>
#include <QLocale>
#include <klanguagebutton.h>
#include <QIcon>
#include <QDialogButtonBox>
#include <QPushButton>

#include <klocalizedstring.h>

#include "doclocaledialog.h"
#include "defaultprovider.h"


DocLocaleDialog::DocLocaleDialog( QWidget *parent )
  : QDialog( parent ),
    mLocale( 0 )
{
  setObjectName( "DOCLOCALE_DIALOG" );
  setModal( true );
  setWindowTitle( i18n( "Document Locale Settings" ) );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QWidget *mainWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(mainWidget);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
  mainLayout->addWidget(buttonBox);

  QWidget *w = new QWidget;
  mainLayout->addWidget(w);
  QVBoxLayout *layout = new QVBoxLayout;
  w->setLayout(layout);

//TODO PORT QT5   layout->setSpacing( QDialog::spacingHint() );
  QLabel *l = new QLabel( i18n( "<h2>Document Localization</h2>" ));
  layout->addWidget(l);

  l = new QLabel( i18n( "Select country and language for the document.\n"
                                "That influences the formatting of numbers, dates etc." ));
  layout->addWidget(l);
  // l->setFrameStyle( QFrame::Box | QFrame::Sunken );

  QGridLayout *g = new QGridLayout;
  layout->addLayout(g);
//TODO PORT QT5   g->setSpacing( QDialog::spacingHint() );
  l = new QLabel( i18n( "Country: " ));
  mCountryButton = new QComboBox;

  g->addWidget(l, 0, 0);
  g->addWidget(mCountryButton, 0, 1);

  connect( mCountryButton, SIGNAL(activated(const QString &)),
           this, SLOT(changedCountry(const QString &)) );

  l = new QLabel( i18n( "Language:" ) );
  mLanguageButton = new KLanguageButton;

  g->addWidget(l, 1, 0);
  g->addWidget(mLanguageButton, 1, 1);

  mLabSample = new QLabel;
  layout->addWidget(mLabSample);
//TODO PORT QT5   mLabSample->setMargin( QDialog::marginHint() );
  mLabSample->setFrameStyle( QFrame::Box | QFrame::Sunken );

#if 0
  QWidget *dummy = new QWidget( w );
  mainLayout->addWidget(dummy);
  dummy->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ) );
#endif
}

DocLocaleDialog::~DocLocaleDialog()
{
  if ( mLocale ) delete mLocale;
}

void DocLocaleDialog::setLocale( const QString& c, const QString& lang )
{
  // qDebug () << "Setting country " << c << " and lang " << lang << endl;
  if ( !mLocale ) mLocale = new QLocale;
#if 0 // FIXME Porting QT5
  KConfig *cfg = KGlobal::config().data();
  mLocale->setCountry( c, cfg );
  mLocale->setLanguage( lang, cfg );
#endif
  loadCountryList();
  loadLanguageList();

  mCountryButton->setCurrentIndex( mCountryButton->findText(c) );
  mLanguageButton->setCurrentItem( lang );

  slotUpdateSample();
}

void DocLocaleDialog::slotUpdateSample()
{
  Geld g( 12204.23 );

  mLabSample->setText( i18n( "Money: %1\nDate: %2\nDate (short): %3" ).arg( g.toString( mLocale ) )
                       .arg( QDate::currentDate().toString())
                       .arg( QDate::currentDate().toString() ) );
}

QLocale DocLocaleDialog::locale() const
{
  return *mLocale;
}

void DocLocaleDialog::loadLanguageList()
{
  mLanguageButton->loadAllLanguages();
}

QStringList DocLocaleDialog::languageList() const
{
  QStringList list2 = mLocale->uiLanguages();

  return list2;
}

void DocLocaleDialog::loadCountryList()
{
  //Fixme: Don't just give countrycodes, but this needs some other adjustements aswell
    QStringList countrylist;
#if 0 // FIXME Porting
  QStringList countrylist = mLocale->allCountriesList();
  QStringList list2;

  for (int i = 0; i < countrylist.size(); ++i)
            list2 << mLocale->countryCodeToName(countrylist.at(i));
#endif
  mCountryButton->addItems(countrylist);
}


void DocLocaleDialog::changedCountry(const QString & code)
{
  // KConfig *cfg = KGlobal::config().data();
  // mLocale->setCountry(code, cfg );

  // qDebug () << "Country selection changed to " << code << endl;

  // change to the preferred languages in that country, installed only
  QStringList languages = languageList();
  QStringList newLanguageList;
  for ( QStringList::Iterator it = languages.begin();
        it != languages.end();
        ++it )
  {
    QString name;
    readLocale(*it, name, QString::null);

    if (!name.isEmpty())
      newLanguageList += *it;
  }
  // mLocale->setLanguage( newLanguageList );
  slotUpdateSample();
}

  void DocLocaleDialog::readLocale(const QString &path, QString &name,
                                 const QString &sub) const
{
  // FIXME !!
#if 0
  // temperary use of our locale as the global locale
  QLocale *lsave = KGlobal::locale();
  KGlobal::setLocale( mLocale, KGlobal::DontCopyCatalogs );

  // read the name
  QString filepath = QString::fromLatin1("%1%2/entry.desktop")
    .arg(sub)
    .arg(path);

  KSimpleConfig entry(locate("locale", filepath));
  entry.setGroup("KCM Locale");
  name = entry.readEntry("Name");

  // restore the old global locale
  KGlobal::setLocale( lsave, KGlobal::DontCopyCatalogs );
#endif
}


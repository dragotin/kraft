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

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <klanguagebutton.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kvbox.h>

#include "doclocaledialog.h"
#include "defaultprovider.h"


DocLocaleDialog::DocLocaleDialog( QWidget *parent )
  : KDialog( parent ),
    mLocale( 0 )
{
  setObjectName( "DOCLOCALE_DIALOG" );
  setModal( true );
  setCaption( i18n( "Document Locale Settings" ) );
  setButtons( KDialog::Ok | KDialog::Cancel );

  QWidget *w = new QWidget;
  this->setMainWidget(w);
  QVBoxLayout *layout = new QVBoxLayout;
  w->setLayout(layout);

  layout->setSpacing( KDialog::spacingHint() );
  QLabel *l = new QLabel( i18n( "<h2>Document Localization</h2>" ));
  layout->addWidget(l);

  l = new QLabel( i18n( "Select country and language for the document.\n"
                                "That influences the formatting of numbers, dates etc." ));
  layout->addWidget(l);
  // l->setFrameStyle( QFrame::Box | QFrame::Sunken );

  QGridLayout *g = new QGridLayout;
  layout->addLayout(g);
  g->setSpacing( KDialog::spacingHint() );
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
  mLabSample->setMargin( KDialog::marginHint() );
  mLabSample->setFrameStyle( QFrame::Box | QFrame::Sunken );

#if 0
  QWidget *dummy = new QWidget( w );
  dummy->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ) );
#endif
  showButtonSeparator( false );
}

DocLocaleDialog::~DocLocaleDialog()
{
  if ( mLocale ) delete mLocale;
}

void DocLocaleDialog::setLocale( const QString& c, const QString& lang )
{
  kDebug() << "Setting country " << c << " and lang " << lang << endl;
  if ( !mLocale ) mLocale = new KLocale( QString::fromLatin1( "kraft" ) );
  KConfig *cfg = KGlobal::config().data();
  mLocale->setCountry( c, cfg );
  mLocale->setLanguage( lang, cfg );

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
                       .arg( mLocale->formatDate( QDate::currentDate() ) )
                       .arg( mLocale->formatDate( QDate::currentDate(), KLocale::ShortDate ) ) );
}

KLocale DocLocaleDialog::locale() const
{
  return *mLocale;
}

void DocLocaleDialog::loadLanguageList()
{
  mLanguageButton->loadAllLanguages();
}

QStringList DocLocaleDialog::languageList() const
{
  QStringList langlist = mLocale->allLanguagesList();
  QStringList list2;

  for (int i = 0; i < langlist.size(); ++i)
            list2 << mLocale->languageCodeToName(langlist.at(i));

  return list2;
}

void DocLocaleDialog::loadCountryList()
{
  //Fixme: Don't just give countrycodes, but this needs some other adjustements aswell
  QStringList countrylist = mLocale->allCountriesList();
  QStringList list2;

  for (int i = 0; i < countrylist.size(); ++i)
            list2 << mLocale->countryCodeToName(countrylist.at(i));

  mCountryButton->addItems(countrylist);
}


void DocLocaleDialog::changedCountry(const QString & code)
{
  KConfig *cfg = KGlobal::config().data();
  mLocale->setCountry(code, cfg );

  kDebug() << "Country selection changed to " << code << endl;

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
  mLocale->setLanguage( newLanguageList );
  slotUpdateSample();
}

  void DocLocaleDialog::readLocale(const QString &path, QString &name,
                                 const QString &sub) const
{
  // FIXME !!
#if 0
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::locale();
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


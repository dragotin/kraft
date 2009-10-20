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

#include <qcombobox.h>
#include <qwidget.h>
#include <q3vbox.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3Frame>

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
#include <q3grid.h>
#include <qsizepolicy.h>


DocLocaleDialog::DocLocaleDialog( QWidget *parent )
  : KDialog( parent ),
    mLocale( 0 )
{
  setObjectName( "DOCLOCALE_DIALOG" );
  setModal( true );
  setCaption( i18n( "Document Locale Settings" ) );
  setButtons( KDialog::Ok | KDialog::Cancel );

  KVBox *w = new KVBox( this );
  setMainWidget( w );

  w->setSpacing( KDialog::spacingHint() );
  ( void ) new QLabel( i18n( "<h2>Document Localisation</h2>" ), w );
  QLabel *l = new QLabel( i18n( "Select country and language for the document.\n"
                                "That influences the formatting of numbers, dates etc." ), w );
  ( void ) l;
  // l->setFrameStyle( QFrame::Box | QFrame::Sunken );

  Q3Grid *g = new Q3Grid( 2, Qt::Horizontal, w );
  g->setSpacing( KDialog::spacingHint() );
  new QLabel( i18n( "Country: " ),  g );

  mCountryButton = new KLanguageButton( g );
  connect( mCountryButton, SIGNAL(activated(const QString &)),
           this, SLOT(changedCountry(const QString &)) );

  new QLabel( i18n( "Language: : " ),  g );
  mLanguageButton = new KLanguageButton( g );

  mLabSample = new QLabel( w );
  mLabSample->setMargin( KDialog::marginHint() );
  mLabSample->setFrameStyle( Q3Frame::Box | Q3Frame::Sunken );

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

  mCountryButton->setCurrentItem( c );
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
#if 0
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::locale();
  KGlobal::setLocale( mLocale, KGlobal::DontCopyCatalogs );

  // clear the list
  mLanguageButton->clear();

  QStringList first = languageList();

  QStringList prilang;
  // add the primary languages for the country to the list
  for ( QStringList::ConstIterator it = first.begin();
        it != first.end();
        ++it )
  {
    QString str = KStandardDirs::locate("locale", QString::fromLatin1("%1/entry.desktop")
                         .arg(*it));
    if (!str.isNull())
      prilang << str;
  }

  // add all languages to the list
  QStringList alllang = KGlobal::dirs()->findAllResources("locale",
                               QString::fromLatin1("*/entry.desktop") );
  QStringList langlist = prilang;
  if (langlist.count() > 0)
    langlist << QString::null; // separator
  langlist += alllang;

  int menu_index = -2;
  QString submenu; // we are working on this menu
  for ( QStringList::ConstIterator it = langlist.begin();
        it != langlist.end(); ++it )
  {
    if ((*it).isNull())
    {
      mLanguageButton->insertSeparator();
      submenu = QString::fromLatin1("other");
      mLanguageButton->insertSubmenu( mLocale->translate("Other"),
                                      submenu, QString::null, -1 );
      menu_index = -2; // first entries should _not_ be sorted
      continue;
    }
    KSimpleConfig entry(*it);
    entry.setGroup("KCM Locale");
    QString name = entry.readEntry("Name",
                                   mLocale->translate("without name"));
   QString tag = *it;
    int index = tag.findRev('/');
    tag = tag.left(index);
    index = tag.findRev('/');
    tag = tag.mid(index + 1);
    mLanguageButton->insertItem(name, tag, submenu, menu_index);
  }

  // restore the old global locale
  KGlobal::_locale = lsave;
#endif
}

QStringList DocLocaleDialog::languageList() const
{
  return QStringList(); // FIXME !!
#if 0
  QString fileName = KStandardDirs::locate("locale",
                            QString::fromLatin1("l10n/%1/entry.desktop")
                            .arg(mLocale->country()));

  KSimpleConfig entry(fileName);
  entry.setGroup("KCM Locale");
  return entry.readListEntry("Languages");
#endif

}

void DocLocaleDialog::loadCountryList()
{
  // FIXME !!
#if 0
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = mLocale;

  QString sub = QString::fromLatin1("l10n/");

  // clear the list
  mCountryButton->clear();

  QStringList regionlist = KGlobal::dirs()->findAllResources("locale",
                                 sub + QString::fromLatin1("*.desktop"),
                                 false, true );

  for ( QStringList::ConstIterator it = regionlist.begin();
    it != regionlist.end();
    ++it )
  {
    QString tag = *it;
    int index;

    index = tag.findRev('/');
    if (index != -1)
      tag = tag.mid(index + 1);

    index = tag.findRev('.');
    if (index != -1)
      tag.truncate(index);

    KSimpleConfig entry(*it);
    entry.setGroup("KCM Locale");
    QString name = entry.readEntry("Name",
                                   mLocale->translate("without name"));
    QString map( locate( "locale",
                          QString::fromLatin1( "l10n/%1.png" )
                          .arg(tag) ) );
    QIcon icon;
    if ( !map.isNull() )
      icon = KIconLoader::global()->loadIconSet(map, KIcon::Small);
    mCountryButton->insertSubmenu( icon, name, tag, sub, -2 );
  }

  // add all languages to the list
  QStringList countrylist = KGlobal::dirs()->findAllResources
    ("locale", sub + QString::fromLatin1("*/entry.desktop"), false, true);

  for ( QStringList::ConstIterator it = countrylist.begin();
        it != countrylist.end(); ++it )
  {
    KSimpleConfig entry(*it);
    entry.setGroup("KCM Locale");
    QString name = entry.readEntry("Name",
                                   mLocale->translate("without name"));
    QString submenu = entry.readEntry("Region");

    QString tag = *it;
    int index = tag.findRev('/');
    tag.truncate(index);
    index = tag.findRev('/');
    tag = tag.mid(index + 1);
    int menu_index = submenu.isEmpty() ? -1 : -2;

    QString flag( locate( "locale",
                          QString::fromLatin1( "l10n/%1/flag.png" )
                          .arg(tag) ) );
    QIcon icon( KIconLoader::global()->loadIconSet(flag, KIcon::Small) );
    mCountryButton->insertItem( icon, name, tag, submenu, menu_index );
  }

  // restore the old global locale
  KGlobal::_locale = lsave;
#endif
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

#include "doclocaledialog.moc"

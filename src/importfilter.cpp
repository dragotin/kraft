/***************************************************************************
         importfilter.cpp  - Import positions into Kraft documents
                             -------------------
    begin                : Oct 2008
    copyright            : (C) 2008 by Klaas Freitag
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

// include files for QT
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QStandardPaths>
#include <QLocale>
#include <QDebug>
#include <QProcess>

#include <KLocalizedString>

#include "importfilter.h"
#include "unitmanager.h"
#include "defaultprovider.h"


ImportFilter::ImportFilter()
  : mStrict( true )
{

}

bool ImportFilter::readDefinition( const QString& name )
{
  QString defFile = name;
  if ( ! name.startsWith( "/" ) ) {
    QString defFileName = QString( name ).toLower();
    QString findFile = kdeStdDirPath() + defFileName;

    // qDebug () << "KDE StdDir Path: " << findFile;
    defFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, findFile);
    if ( defFile.isEmpty() ) {
      mError = i18n( "Unable to find filter called %1", name );
      return false;
    }
  }

  // qDebug () << "Reading definition file " << defFile;
  QFile f( defFile );
  if ( !f.open( QIODevice::ReadOnly ) ) {
    mError = i18n( "Could not open the definition file!" );
    return false;
  }

  QTextStream t( &f );
  t.setCodec("UTF-8");

  while ( !t.atEnd() ) {
    mDefinition << t.readLine();
  }
  f.close();
  return true;
}

bool ImportFilter::parse()
{
  return true;
}

bool ImportFilter::recode( const QString& file, const QString& outfile )
{
  if ( mEncoding.isEmpty() ) return true;

  QString cmd = DefaultProvider::self()->iconvTool();

  if ( QFile::exists( cmd ) ) {
    QStringList args = QStringList()
      << "-f" << mEncoding << "-t" << "utf-8" << "-o" << outfile << file;
    int result = QProcess::execute( cmd, args );
    Q_UNUSED(result);
    // qDebug () << "Recode finished with exit code " << result;
    return true;
  } else {
    // qDebug () << "Recode-tool does not exist!";
  }
  return false;
}


// ###########################################################################

DocPositionImportFilter::DocPositionImportFilter()
 :ImportFilter( )
{

}

QString DocPositionImportFilter::kdeStdDirPath() const
{
  QString re = QString::fromLatin1( "kraft/importfilter/positions/" );
  return re;
}

#define FILTER_TAG( x, y ) x

bool DocPositionImportFilter::parseDefinition()
{
  bool ret = true;
  for ( QStringList::Iterator it = mDefinition.begin(); it != mDefinition.end(); ++it ) {
    QString l = ( *it ).trimmed();

    if ( l.isEmpty() || l.startsWith( "#" ) ) {
      // continue - whitespace....
    } else if ( l.startsWith( FILTER_TAG( "amount:", "amount of the item" ),  Qt::CaseInsensitive ) ) {
      mAmount = ( l.right( l.length()-7 ) ).trimmed();
    } else if ( l.startsWith( FILTER_TAG( "text:",  "The item text" ),  Qt::CaseInsensitive ) ) {
      mText = ( l.right( l.length()-5 ) ).trimmed();
      mText.replace( "<br>", QChar( 0x0A ) );
    } else if ( l.startsWith( FILTER_TAG( "unit:",  "The item unit" ),  Qt::CaseInsensitive ) ) {
      mUnit = ( l.right( l.length()-5 ) ).trimmed();
    } else if ( l.startsWith( FILTER_TAG( "unit_price:", "unit price" ),  Qt::CaseInsensitive ) ) {
      mUnitPrice = ( l.right( l.length()-11 ) ).trimmed();
    } else if ( l.startsWith( FILTER_TAG( "name:",  "The name of the filter" ),  Qt::CaseInsensitive ) ) {
      mName = ( l.right( l.length()-5 ) ).trimmed();
    } else if ( l.startsWith( FILTER_TAG( "description:", "The filter description" ), Qt::CaseInsensitive ) ) {
      mDescription = ( l.right( l.length()-12 ) ).trimmed();
    } else if ( l.startsWith( FILTER_TAG("encoding:", "The encoding of the source file" ),  Qt::CaseInsensitive ) ) {
      mEncoding = ( l.right( l.length()-9 ) ).trimmed();
    } else if ( l.startsWith( FILTER_TAG( "separator:", "The separator used in the source file" ),  Qt::CaseInsensitive ) ) {
      // qDebug () << "Separator found: " << l.right( l.length()-10 );
      mSeparator = ( l.right( l.length()-10 ) ).trimmed();
    } else if ( l.startsWith( FILTER_TAG( "tags:", "Comma separated list of tags for one item" ),  Qt::CaseInsensitive ) ) {
      mTags = ( l.right( l.length()-5 ) ).trimmed();
    } else {
      // qDebug () << "WRN: Unknown filter tag found: " << l;
      if ( mError.isEmpty() ) mError = i18n( "Unknown tags: " );
      mError.append( l );
      ret = false;
    }
  }

  if ( mSeparator.isEmpty() ) {
    mSeparator = QString::fromLatin1( ";" );
  }

  return ret;
}

void DocPositionImportFilter::debugDefinition()
{
  // qDebug () << "Amount: " << mAmount;
  // qDebug () << "Unit: " << mUnit;
  // qDebug () << "UnitPrice: " << mUnitPrice;
  // qDebug () << "Text: " << mText;
  // qDebug () << "Separator: <" << mSeparator << ">";
}

DocPositionList DocPositionImportFilter::import( const QUrl &inFile )
{
  DocPositionList list;
  bool copied = false;

  if( !inFile.isLocalFile() ) return list;

  QString file( inFile.toLocalFile() );

  // in case we have to recode, the source file needs to be copied.
  bool ok = true;
  if ( !mEncoding.isEmpty() ) {
    file += ".tmp";
    copied = true;
    // qDebug () << "Encoding file to " << file;

    ok = recode( inFile.toLocalFile(), file );
    if ( !ok ) {
      // qDebug () << "Recoding failed!";
      mError = i18n( "Could not recode input file!" );
    }
  }

  QFile f( file );

  if ( ! f.exists() ) {
    // qDebug () << "File " << file << " could not be found!";
    mError = i18n( "Unable to open temp file " ) + file;
    ok = false;
  }

  if ( ok ) {
    if ( !f.open( QIODevice::ReadOnly ) ) {
      mError = i18n( "Could not open the import source file!" );
      ok = false;
    }
  }
  if ( ok ) {
    QTextStream t( &f );
    t.setCodec("UTF-8");

    int cnt = 0;
    while ( !t.atEnd() ) {
      cnt++;
      QString l = t.readLine().trimmed();
      // qDebug () << "Importing line " << l;
      if ( !( l.isEmpty() || l.startsWith( "#" ) ) ) {
        bool ok;
        DocPosition dp = importDocPosition( l, ok );
        if ( ok )
          list.append( new DocPosition( dp ) );
      }
    }
    f.close();
  }
  if ( copied ) {
    QFile::remove( file );
  }
  return list;
}

// creates a DocPosition from one line of the imported file
DocPosition DocPositionImportFilter::importDocPosition( const QString& l, bool& ok )
{
  QStringList parts = l.split( mSeparator, QString::KeepEmptyParts );
  // qDebug () << "Importing raw line " << l;

  QString h;
  ok = true;

  DocPosition pos;

  // the text (mandatory)
  QString t = replaceCOL( parts, mText );

  pos.setText( t );
  QString unit = replaceCOL( parts, mUnit );

  int unitId = UnitManager::self()->getUnitIDSingular( unit );
  if ( unitId > -1 ) {
    pos.setUnit( UnitManager::self()->getUnit( unitId ) );
  } else {
    pos.setUnit(Einheit( unit ));
  }

  // Amount.
  h = replaceCOL( parts, mAmount );
  bool convOk = true;
  double a = h.toDouble( &convOk );
  if ( convOk ) {
    pos.setAmount( a );
  } else {
    // qDebug () << "WRN: Unable to convert amount to double: " << h;
    if ( mStrict ) ok = false;
  }

  // Unit Price
  h = replaceCOL( parts, mUnitPrice );
  a = h.toDouble( &convOk );
 if ( convOk ) {
   pos.setUnitPrice( Geld( a ) );
 } else {
    // qDebug () << "WRN: Unable to convert unit price to double: " << h;
    if ( mStrict ) ok = false;
 }

 if ( !mTags.isEmpty() ) {
   QStringList tags =  mTags.split(QRegExp( "\\s*,\\s*" ));

   for ( QStringList::Iterator it = tags.begin(); it != tags.end(); ++it ) {
     QString t = ( *it ).trimmed();
     pos.setTag( t );
   }
 }

 return pos;
}

QString DocPositionImportFilter::replaceCOL( const QStringList& cols, const QString& in )
{
  QString re( in );
  for ( int i = 0; i < cols.size(); i++ ) {
    QString replacer = QString( "COL(%1)" ).arg( i+1 );
    QString col = cols[i].trimmed();

    re.replace( replacer, col, Qt::CaseInsensitive );
  }
  // qDebug() << "replaced line: " << re;
  return re;
}

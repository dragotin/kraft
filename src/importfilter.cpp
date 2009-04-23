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
#include <qfile.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include "importfilter.h"
#include "unitmanager.h"
#include "defaultprovider.h"

#include <qregexp.h>
#include <kio/netaccess.h>
#include <ktempfile.h>

ImportFilter::ImportFilter()
  : mStrict( true )
{

}

bool ImportFilter::readDefinition( const QString& name )
{
  QString defFile = name;
  if ( ! name.startsWith( "/" ) ) {
    KStandardDirs stdDirs;
    QString defFileName = QString( name ).lower();
    QString findFile = kdeStdDirPath() + defFileName;

    kdDebug() << "KDE StdDir Path: " << findFile << endl;
    defFile = stdDirs.findResource( "data", findFile );
    if ( defFile.isEmpty() ) {
      mError = i18n( "Unable to find filter called %1" ).arg( name );
      return false;
    }
  }

  kdDebug() << "Reading definition file " << defFile << endl;
  QFile f( defFile );
  if ( !f.open( IO_ReadOnly ) ) {
    mError = i18n( "Could not open the definition file!" );
    return false;
  }

  QTextStream t( &f );
  t.setEncoding(QTextStream::UnicodeUTF8);

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
    QString command = QString( "%1 -f %2 -t utf-8 -o %3 %4" ).arg( cmd )
                      .arg( mEncoding ).arg( outfile ).arg( file );
    int result = system( command.latin1() );
    kdDebug() << "Recode finished with exit code " << result << endl;
    return true;
  } else {
    kdDebug() << "Recode-tool does not exist!" << endl;
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
    QString l = ( *it ).stripWhiteSpace();

    if ( l.isEmpty() || l.startsWith( "#" ) ) {
      // continue - whitespace....
    } else if ( l.startsWith( FILTER_TAG( "amount:", "amount of the item" ),  false ) ) {
      mAmount = ( l.right( l.length()-7 ) ).stripWhiteSpace();
    } else if ( l.startsWith( FILTER_TAG( "text:",  "The item text" ),  false ) ) {
      mText = ( l.right( l.length()-5 ) ).stripWhiteSpace();
      mText.replace( "<br>", QChar( 0x0A ) );
    } else if ( l.startsWith( FILTER_TAG( "unit:",  "The item unit" ),  false ) ) {
      mUnit = ( l.right( l.length()-5 ) ).stripWhiteSpace();
    } else if ( l.startsWith( FILTER_TAG( "unit_price:", "unit price" ),  false ) ) {
      mUnitPrice = ( l.right( l.length()-11 ) ).stripWhiteSpace();
    } else if ( l.startsWith( FILTER_TAG( "name:",  "The name of the filter" ),  false ) ) {
      mName = ( l.right( l.length()-5 ) ).stripWhiteSpace();
    } else if ( l.startsWith( FILTER_TAG( "description:", "The filter description" ), false ) ) {
      mDescription = ( l.right( l.length()-12 ) ).stripWhiteSpace();
    } else if ( l.startsWith( FILTER_TAG("encoding:", "The encoding of the source file" ),  false ) ) {
      mEncoding = ( l.right( l.length()-9 ) ).stripWhiteSpace();
    } else if ( l.startsWith( FILTER_TAG( "separator:", "The separator used in the source file" ),  false ) ) {
      kdDebug() << "Separator found: " << l.right( l.length()-10 ) << endl;
      mSeparator = ( l.right( l.length()-10 ) ).stripWhiteSpace();
    } else if ( l.startsWith( FILTER_TAG( "tags:", "Comma separated list of tags for one item" ),  false ) ) {
      mTags = ( l.right( l.length()-5 ) ).stripWhiteSpace();
    } else {
      kdDebug() << "WRN: Unknown filter tag found: " << l << endl;
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
  kdDebug() << "Amount: " << mAmount << endl;
  kdDebug() << "Unit: " << mUnit << endl;
  kdDebug() << "UnitPrice: " << mUnitPrice << endl;
  kdDebug() << "Text: " << mText << endl;
  kdDebug() << "Separator: <" << mSeparator << ">" << endl;
}

QValueList<DocPosition> DocPositionImportFilter::import( const QString& inFile )
{
  QValueList<DocPosition> list;
  bool copied = false;
  QString file( inFile );

  // in case we have to recode, the source file needs to be copied.
  bool ok = true;
  if ( !mEncoding.isEmpty() ) {
    file = inFile+".tmp";
    copied = true;
    kdDebug() << "Encoding file to " << file << endl;

    ok = recode( inFile, file );
    if ( !ok ) {
      kdDebug() << "Recoding failed!" << endl;
      mError = i18n( "Could not recode input file!" );
    }
  }

  QFile f( file );

  if ( ! f.exists() ) {
    kdDebug() << "File " << file << " could not be found!" << endl;
    mError = i18n( "Unable to open temp file " ) + file;
    ok = false;
  }

  if ( ok ) {
    if ( !f.open( IO_ReadOnly ) ) {
      mError = i18n( "Could not open the import source file!" );
      ok = false;
    } else {
      kdDebug() << "Unable to open file in read only mode" << endl;
    }
  }
  if ( ok ) {
    QTextStream t( &f );
    t.setEncoding(QTextStream::UnicodeUTF8);

    int cnt = 0;
    while ( !t.atEnd() ) {
      cnt++;
      QString l = t.readLine().stripWhiteSpace();
      kdDebug() << "Importing line " << l << endl;
      if ( !( l.isEmpty() || l.startsWith( "#" ) ) ) {
        bool ok;
        DocPosition dp = importDocPosition( l, ok );
        if ( ok )
          list.append( dp );
      }
    }
    f.close();
  }
  if ( copied ) {
    KIO::NetAccess::del( file, 0 );
  }
  return list;
}

// creates a DocPosition from one line of the imported file
DocPosition DocPositionImportFilter::importDocPosition( const QString& l, bool& ok )
{
  QStringList parts = QStringList::split( mSeparator, l, true );
  kdDebug() << "Importing raw line " << l << endl;

  QString h;
  ok = true;

  DocPosition pos;

  // the text (mandatory)
  QString t = replaceCOL( parts, mText );

  pos.setText( t );
  QString unit = replaceCOL( parts, mUnit );

  int unitId = UnitManager::getUnitIDSingular( unit );
  if ( unitId > -1 ) {
    pos.setUnit( UnitManager::getUnit( unitId ) );
  } else {
    kdDebug() << "WRN: Unable to get a valid unit" << endl;
    if ( mStrict ) ok = false;
  }

  // Amount.
  h = replaceCOL( parts, mAmount );
  bool convOk = true;
  double a = h.toDouble( &convOk );
  if ( convOk ) {
    pos.setAmount( a );
  } else {
    kdDebug() << "WRN: Unable to convert amount to double: " << h << endl;
    if ( mStrict ) ok = false;
  }

  // Unit Price
  h = replaceCOL( parts, mUnitPrice );
  a = h.toDouble( &convOk );
 if ( convOk ) {
   pos.setUnitPrice( Geld( a ) );
 } else {
    kdDebug() << "WRN: Unable to convert unit price to double: " << h << endl;
    if ( mStrict ) ok = false;
 }

 if ( !mTags.isEmpty() ) {
   QStringList tags = QStringList::split( QRegExp( "\\s*,\\s*" ), mTags );

   for ( QStringList::Iterator it = tags.begin(); it != tags.end(); ++it ) {
     QString t = ( *it ).stripWhiteSpace();
     pos.setTag( t );
   }
 }

 return pos;
}

QString DocPositionImportFilter::replaceCOL( const QStringList& cols, const QString& in )
{
  QString re( in );
  for ( uint i = 0; i < cols.size(); i++ ) {
    QString replacer = QString( "COL(%1)" ).arg( i+1 );
    re.replace( replacer, cols[i], false );
  }
  // kdDebug() << "replaced line: " << re << endl;
  return re;
}

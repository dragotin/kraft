/***************************************************************************
                 doctype.cpp - doc type class
                             -------------------
    begin                : Oct. 2007
    copyright            : (C) 2007 by Klaas Freitag
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

// include files for Qt
#include <QString>
#include <QSqlQuery>
#include <QFile>
#include <qfile.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// application specific includes
#include "doctype.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "kraftdb.h"
#include "numbercycle.h"

/**
@author Klaas Freitag
*/

idMap DocType::mNameMap = idMap();


DocType::DocType()
  : mAttributes( QString::fromLatin1( "DocType" ) ),
    mDirty( false )
{
  init();
}

DocType::DocType( const QString& name, bool dirty )
  : mAttributes( QString::fromLatin1( "DocType" ) ),
    mName( name ),
    mDirty( dirty )
{
  init();
  if ( mNameMap.contains( name ) ) {
    dbID id = mNameMap[ name ];

    mAttributes.load( id );
  }

  readFollowerList();
  readIdentTemplate();
}

DocType& DocType::operator=( const DocType& dt )
{
  if( this != &dt ) {
    mAttributes = dt.mAttributes;
    mFollowerList = dt.mFollowerList;
    mName = dt.mName;
    mIdentTemplate = dt.mIdentTemplate;
    mDirty = dt.mDirty;
    mMergeIdent = dt.mMergeIdent;

    mNameMap = dt.mNameMap;
  }
  return *this;
}


void DocType::init()
{
  // === Start to fill static content
  if ( ! mNameMap.empty() ) return;

  QSqlQuery q;
  q.prepare( "SELECT docTypeID, name FROM DocTypes ORDER BY name" );
  q.exec();

  while ( q.next() ) {
    dbID id( q.value(0).toInt() );
    QString name = q.value(1).toString();

    mNameMap[ name ] = id;
    // QString h = DefaultProvider::self()->locale()->translate( cur.value( "name" ).toString() );
  }
}

void DocType::clearMap()
{
  mNameMap.clear();
}

QStringList DocType::all()
{
  init();

  QStringList re;

  QSqlQuery q;
  q.prepare( "SELECT docTypeID, name FROM DocTypes ORDER BY name" );
  q.exec();

  while ( q.next() ) {
    re << q.value(1).toString();
  }

  return re;
}

QStringList DocType::allLocalised()
{
  return all();
}

// static function to retrieve id of a certain doctype
dbID DocType::docTypeId( const QString& docType )
{
  dbID id;
  init();
  if ( mNameMap.contains( docType ) ) {
    id = mNameMap[ docType ];

    return id;
  } else {
    kError()<< "Can not find id for doctype named " << docType;
  }
  return id;
}

bool DocType::allowDemand()
{
  bool re = false;

  if ( mAttributes.contains( "AllowDemand" ) ) {
    re = true;
  }
  return re;
}

bool DocType::allowAlternative()
{
  bool re = false;

  if ( mAttributes.contains( "AllowAlternative" ) ) {
    re = true;
  }
  return re;
}

bool DocType::pricesVisible()
{
    bool re = true;
    if( mAttributes.contains("HidePrices")) {
        re = false;
    }
    return re;
}


QStringList DocType::follower()
{
  return mFollowerList;
}

void DocType::readFollowerList()
{
  QSqlQuery q;
  q.prepare( "SELECT typeId, followerId, sequence FROM DocTypeRelations WHERE typeId=:type ORDER BY sequence");
  q.bindValue( ":type", mNameMap[mName].toInt() );
  q.exec();

  while ( q.next() ) {
    dbID followerId( q.value(1).toInt() );

    idMap::Iterator it;
    for ( it = mNameMap.begin(); it != mNameMap.end(); ++it ) {
      if ( it.value() == followerId ) {
        mFollowerList << it.key();
      }
    }
  }
}

QString DocType::numberCycleName()
{
  QString re = NumberCycle::defaultName();
  if ( mAttributes.hasAttribute( "identNumberCycle" ) ) {
    re = mAttributes["identNumberCycle"].value().toString();
  }
  return re;
}

void DocType::setNumberCycleName( const QString& name )
{
  if ( name.isEmpty() ) return;

  if ( name != NumberCycle::defaultName() ) {
    Attribute att( "identNumberCycle" );
    att.setPersistant( true );
    att.setValue( name );
    mAttributes["identNumberCycle"] = att;
  } else {
    // remove default value from map
    mAttributes.markDelete( "identNumberCycle" );
    kDebug() << "Removing identNumberCycle Attribute";
  }
  mDirty = true;
  readIdentTemplate();
}

QString DocType::templateFile( const QString& lang )
{
  KStandardDirs stdDirs;
  QString tmplFile;

  QString reportFileName = QString( "%1.trml").arg( name().toLower() );
  reportFileName.replace(QChar(' '), QChar('_'));

  if ( mAttributes.hasAttribute( "docTemplateFile" ) ) {
    tmplFile = mAttributes["docTemplateFile"].value().toString();
    if( !tmplFile.isEmpty() ) {
        return tmplFile;
    }
  }

  // Try to find it from the installation
  QStringList searchList;
  if( !lang.isEmpty() ) {
      searchList << QString("kraft/reports/%1/%2").arg(lang).arg(reportFileName);
  }
  searchList << QString("kraft/reports/%1").arg(reportFileName);
  if( !lang.isEmpty() ) {
      searchList << QString("kraft/reports/%1/invoice.trml").arg(lang);
  }
  searchList << QLatin1String("kraft/reports/invoice.trml");

  foreach( QString searchPath, searchList ) {
     const QString tFile = KStandardDirs::locate( "data", searchPath );

      if( !tFile.isEmpty() && tFile != searchPath && QFile::exists( tFile )) {
          tmplFile = tFile;
          kDebug() << "Found template file " << tmplFile;
          break;
      }
  }

  if( tmplFile.isEmpty() ) {
      const QString prjPath = QString::fromUtf8(qgetenv( "KRAFT_HOME" ));

      if( tmplFile.isEmpty() && !prjPath.isEmpty() ) {
          foreach( QString searchPath, searchList ) {
              if( searchPath.startsWith(QLatin1String("kraft"))) {
                  // remove the kraft-String here.
                  searchPath.remove(0, 5); // remove "kraft"
              }
              const QString tFile = prjPath + searchPath;
              if( !tFile.isEmpty() && QFile::exists(tFile) ) {
                  kDebug() << "Found template file " << tFile;
                  tmplFile = tFile;
                  break;
              }
          }
      }
  }

  if( tmplFile.isEmpty() ) {
      kDebug() << "unable to find a template file for " << name();
  }

  return tmplFile;
}

QString DocType::defaultTemplateFile() const
{
  KStandardDirs stdDirs;

  QString findFile = "kraft/reports/invoice.trml";
  return stdDirs.findResource( "data", findFile );
}

void DocType::setTemplateFile( const QString& name )
{
  if ( name.isEmpty() || name == defaultTemplateFile() ) { // the default is returned anyway.
    // remove default value from map
    mAttributes.markDelete( "docTemplateFile" );
    kDebug() << "Removing docTemplateFile Attribute";
  } else {
    Attribute att( "docTemplateFile" );
    att.setPersistant( true );
    att.setValue( name );
    mAttributes["docTemplateFile"] = att;
  }
  mDirty = true;
}

QString DocType::mergeIdent()
{
  QString re = "0";
  if ( mAttributes.hasAttribute( "docMergeIdent" ) ) {
    re = mAttributes["docMergeIdent"].value().toString();
  }

  return re;
}

void DocType::setMergeIdent( const QString& ident )
{
  if ( !ident.isEmpty() ) {
    Attribute att( "docMergeIdent" );
    att.setPersistant( true );
    att.setValue( ident );
    mAttributes["docMergeIdent"] = att;
  } else {
    // remove default value from map
    mAttributes.markDelete( "docMergeIdent" );
    kDebug() << "Removing docMergeIdent Attribute";
  }
  mDirty = true;

}

QString DocType::watermarkFile()
{
  QString re;
  if ( mAttributes.hasAttribute( "watermarkFile" ) ) {
    re = mAttributes["watermarkFile"].value().toString();
  }

  return re;
}


void DocType::setWatermarkFile( const QString& file )
{
  if ( !file.isEmpty() ) {
    Attribute att( "watermarkFile" );
    att.setPersistant( true );
    att.setValue( file );
    mAttributes["watermarkFile"] = att;
  } else {
    // remove default value from map
    mAttributes.markDelete( "watermarkFile" );
    kDebug() << "Removing docMergeFile Attribute";
  }
  mDirty = true;
}

QString DocType::generateDocumentIdent( KraftDoc *doc, int id )
{
  /*
   * The pattern may contain the following tags:
   * %y - the year of the documents date.
   * %w - the week number of the documents date
   * %d - the day number of the documents date
   * %m - the month number of the documents date
   * %c - the customer id from kaddressbook
   * %i - the uniq identifier from db.
   * %type - the localised doc type (offer, invoice etc.)
   * %uid  - the customer uid
   */

  QString pattern = identTemplate();
  if ( pattern.indexOf( "%i" ) == -1 ) {
    kWarning() << "No %i found in identTemplate, appending it to meet law needs!";
    pattern += "-%i";
  }
  QDate d = QDate::currentDate();
  if ( doc ) d = doc->date();

  KraftDB::StringMap m;

  m[ "%yyyy" ] = d.toString( "yyyy" );
  m[ "%yy" ] = d.toString( "yy" );
  m[ "%y" ] = d.toString( "yyyy" );

  QString h;
  h.sprintf( "%02d", d.weekNumber( ) );
  m[ "%ww" ] = h;
  m[ "%w" ] = QString::number( d.weekNumber( ) );

  m[ "%dd" ] = d.toString( "dd" );
  m[ "%d" ] = d.toString( "d" );

  m[ "%m" ] = QString::number( d.month() );

  m[ "%MM" ] = d.toString( "MM" );
  m[ "%M" ] = d.toString( "M" );

  int i = id;
  if ( id == -1 ) { // hot mode: The database id is incremented by nextIdentId()
    i = nextIdentId();
  }

  h.sprintf( "%06d", i );
  m[ "%iiiiii" ] = h;

  h.sprintf( "%05d", i );
  m[ "%iiiii" ] = h;

  h.sprintf( "%04d", i );
  m[ "%iiii" ] = h;

  h.sprintf( "%03d", i );
  m[ "%iii" ] = h;

  h.sprintf( "%02d", i );
  m[ "%ii" ] = h;

  m[ "%i" ] = QString::number( i );

  if ( doc ) {
    m[ "%c" ] = doc->addressUid();
    m[ "%type" ] = doc->docType();
    m[ "%uid" ] = doc->addressUid();
  } else {
    m[ "%c"] = QLatin1String(" <addressUid>" );
    m[ "%type" ] = mName;
    m[ "%uid" ] = QLatin1String("<uid>");
  }

  QString re = KraftDB::self()->replaceTagsInWord( pattern, m );
  kDebug() << "Generated document ident: " << re;

  return re;
}

// if hot, the id is updated in the database, otherwise not.
int DocType::nextIdentId( bool hot )
{
  QString numberCycle = numberCycleName();

  if ( numberCycle.isEmpty() ) {
    kError() << "NumberCycle name is empty";
    return -1;
  }

  QSqlQuery qLock;
  if ( hot ) {
    qLock.exec( "LOCK TABLES numberCycles WRITE" );
  }

  QSqlQuery q;
  q.prepare( "SELECT lastIdentNumber FROM numberCycles WHERE name=:name" );

  int num = -1;
  q.bindValue( ":name", numberCycle );
  q.exec();
  if ( q.next() ) {
    num = 1+( q.value( 0 ).toInt() );
    kDebug() << "Got current number: " << num;

    if ( hot ) {
      QSqlQuery setQuery;
      setQuery.prepare( "UPDATE numberCycles SET lastIdentNumber=:newNumber WHERE name=:name" );
      setQuery.bindValue( ":name", numberCycle );
      setQuery.bindValue( ":newNumber", num );
      setQuery.exec();
      if ( setQuery.isActive() ) {
        kDebug() << "Successfully created new id number for numbercycle " << numberCycle << ": "
                  << num << endl;
      }
    }
  }
  if ( hot ) {
    qLock.exec( "UNLOCK TABLES" );
  }

  return num;
}

QString DocType::identTemplate()
{
  return mIdentTemplate;
}

void DocType::setIdentTemplate( const QString& t )
{
  mIdentTemplate = t;
}

void DocType::readIdentTemplate()
{
  QSqlQuery q;
  QString tmpl;

  const QString defaultTempl = QString::fromLatin1( "%y%ww-%i" );

  QString numberCycle = numberCycleName();
  if ( numberCycle.isEmpty() ) {
    kError() << "Numbercycle for doctype is empty, returning default";
    mIdentTemplate = defaultTempl;
  }
  kDebug() << "Picking ident Template for numberCycle " << numberCycle;

  q.prepare( "SELECT identTemplate FROM numberCycles WHERE name=:name" );

  q.bindValue( ":name", numberCycle );
  q.exec();
  if ( q.next() ) {
    tmpl = q.value( 0 ).toString();
    kDebug() << "Read ident template from database: " << tmpl;
  }

  // FIXME: Check again.
  if ( tmpl.isEmpty() ) {
    // migration: If there is nothing yet in the database, check the local config and
    // transfer the setting to the db
    QString pattern = KraftSettings::self()->docIdent();
    if ( pattern.isEmpty() ) {
      // There is nothing in KConfig File, so we use our default from here.
      pattern = defaultTempl;
    }
    kDebug() << "Writing ident template to database: " << pattern;
    QSqlQuery insQuery;
    insQuery.prepare( "UPDATE numberCycles SET identTemplate=:pattern WHERE name=:name" );
    insQuery.bindValue( ":name", numberCycle );
    insQuery.bindValue( ":pattern", pattern );
    insQuery.exec();
    tmpl = pattern;
  }
  mIdentTemplate = tmpl;
}

QString DocType::name() const
{
  return mName;
}

void DocType::setName( const QString& name )
{
  QString oldName = mName;
  dbID id = mNameMap[ oldName ]; // The old id.
  mNameMap[ name ] = id;
  mNameMap.remove( oldName );
  mName = name;
  mDirty = true;
}


/*
 * Saves the name and the attriutes (numbercycle, demand, etc.)
 */
void DocType::save()
{
  if ( !mDirty ) {
    kDebug() << "Saving: not DIRTY!";
    return;
  }

  if ( !mNameMap.contains( mName ) ) {
    kError() << "nameMap does not contain id for " << mName;
    return;
  }
  dbID id = mNameMap[ mName ];

  QSqlQuery q;

  bool doInsert = false;
  if ( id.isOk() ) {
    q.prepare( "UPDATE DocTypes SET name=:name WHERE docTypeId=:id" );
    q.bindValue( ":id", id.toInt() );
  } else {
    q.prepare( "INSERT INTO DocTypes (name) VALUES (:name)" );
    doInsert = true;
  }

  q.bindValue( ":name", mName );
  q.exec();

  if ( doInsert ) {
    mNameMap[mName] = KraftDB::self()->getLastInsertID();
  }

  mAttributes.save( mNameMap[mName] );
}

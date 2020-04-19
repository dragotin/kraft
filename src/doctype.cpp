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
#include <QtCore>
#include <QSqlQuery>

// application specific includes
#include "doctype.h"
#include "kraftdb.h"
#include "numbercycle.h"
#include "attribute.h"

/**
@author Klaas Freitag
*/

idMap DocType::mNameMap = idMap();

DocType::DocType()
  : mAttributes( QStringLiteral( "DocType" ) ),
    mDirty( false )
{
  init();
}

DocType::DocType( const QString& name, bool dirty )
  : mAttributes( QStringLiteral( "DocType" ) ),
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
    qCritical()<< "Can not find id for doctype named " << docType;
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

bool DocType::substractPartialInvoice()
{
    bool re = false;
    if( mAttributes.contains("SubstractPartialInvoice")) {
        re = true;
    }
    return re;
}

bool DocType::partialInvoice()
{
    bool re = false;
    if( mAttributes.contains("PartialInvoice")) {
        re = true;
    }
    return re;
}

// returns the amount of followers added
int DocType::setAllFollowers( const QStringList& followers)
{
    QSqlQuery q;
    q.prepare("INSERT INTO DocTypeRelations (typeId, followerId, sequence) VALUES (:typeId, :followerId, 0)");
    QSqlQuery qu;
    qu.prepare("UPDATE DocTypeRelations SET sequence=:seq WHERE typeId=:typeId AND followerId=:followerId");

    // get "my" doc type Id
    int typeId = mNameMap[mName].toInt();
    q.bindValue(":typeId", typeId);
    qu.bindValue(":typeId", typeId);

    // get the max sequence for me
    int seq = 0;
    {
        QSqlQuery cq;
        cq.prepare("SELECT MAX(sequence) FROM DocTypeRelations WHERE typeId=:tdId");
        cq.bindValue(":tdId", typeId);
        cq.exec();
        if( cq.next() ) {
            seq = cq.value(0).toInt();
        }
    }

    const QStringList existingFollowers = follower();
    int cnt = 0; // simple counter to return.
    for( const QString& f : followers ) {
        if( mNameMap.contains(f) ) {
            int followerId = mNameMap[f].toInt();
            if( !existingFollowers.contains(f) ) {
                q.bindValue(":followerId", followerId );
                q.exec();
                cnt++;
            }
            // use the updater
            qu.bindValue(":seq", ++seq);
            qu.bindValue(":followerId", followerId);
            qu.exec();
        }
    }
    return cnt;
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
    // qDebug () << "Removing identNumberCycle Attribute";
  }
  mDirty = true;
  readIdentTemplate();
}

QString DocType::templateFile( )
{
  QString tmplFile;

  QString reportFileName = QString( "%1.trml").arg( name().toLower() );
  reportFileName.replace(QChar(' '), QChar('_'));

  if ( mAttributes.hasAttribute( "docTemplateFile" ) ) {
    tmplFile = mAttributes["docTemplateFile"].value().toString();
    if( !tmplFile.isEmpty() ) {
        QFileInfo fi(tmplFile);
        if( fi.isAbsolute() ) {
            return tmplFile;
        } else {
            // it is not an absolute file name, try to find it
            reportFileName = tmplFile;
        }
        tmplFile.clear();
    }
  }

  // Try to find it from the installation
  QStringList searchList;
  searchList << QString("kraft/reports/%1").arg(reportFileName);
  searchList << QLatin1String("kraft/reports/invoice.trml");

  const QString prjPath = QString::fromUtf8(qgetenv( "KRAFT_HOME" ));

  if( tmplFile.isEmpty() && !prjPath.isEmpty() ) {
      foreach( QString searchPath, searchList ) {
          if( searchPath.startsWith(QLatin1String("kraft"))) {
              // remove the kraft-String here.
              searchPath.remove(0, 5); // remove "kraft"
          }
          const QString tFile = prjPath + searchPath;
          if( !tFile.isEmpty() && QFile::exists(tFile) ) {
              // qDebug () << "Found template file " << tFile;
              tmplFile = tFile;
              break;
          }
      }
  }

  if( tmplFile.isEmpty() ) {
      foreach( QString searchPath, searchList ) {
          const QString tFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, searchPath);

          if( !tFile.isEmpty() && tFile != searchPath && QFile::exists( tFile )) {
              tmplFile = tFile;
              // qDebug () << "Found template file " << tmplFile;
              break;
          }
      }
  }

  if( tmplFile.isEmpty() ) {
      qDebug () << "unable to find a template file for " << name();
  } else {
      qDebug () << "Found template file " << tmplFile;
  }
  return tmplFile;
}

QString DocType::defaultTemplateFile() const
{
    // first check for a country specific file
    QLocale locale;
    const QString country = locale.bcp47Name();
    QString findFile = QString("kraft/reports/%1/invoice.trml").arg(country);
    QString re = QStandardPaths::locate(QStandardPaths::GenericDataLocation, findFile);

    if( re.isEmpty() ) {
        // No lang specific one.
        findFile = "kraft/reports/invoice.trml";
        re = QStandardPaths::locate(QStandardPaths::GenericDataLocation, findFile);
    }
    return re;
}

void DocType::setTemplateFile( const QString& name )
{
  if ( name.isEmpty() || name == defaultTemplateFile() ) { // the default is returned anyway.
    // remove default value from map
    mAttributes.markDelete( "docTemplateFile" );
    // qDebug () << "Removing docTemplateFile Attribute";
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
    // qDebug () << "Removing docMergeIdent Attribute";
  }
  mDirty = true;

}

void DocType::setAttribute( const QString& attribute, const QString& val)
{
    if ( !(attribute.isEmpty() || val.isEmpty()) ) {
      Attribute att( attribute );
      att.setPersistant( true );
      att.setValue( val);
      mAttributes[attribute] = att;
      mDirty = true;
    }
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
    // qDebug () << "Removing docMergeFile Attribute";
  }
  mDirty = true;
}

QString DocType::generateDocumentIdent( const QDate& docDate, const QString& docType,
                                        const QString& addressUid, int id )
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
    qWarning() << "No %i found in identTemplate, appending it to meet law needs!";
    pattern += "-%i";
  }

  KraftDB::StringMap m;

  m[ "%yyyy" ] = docDate.toString( "yyyy" );
  m[ "%yy" ] = docDate.toString( "yy" );
  m[ "%y" ] = docDate.toString( "yyyy" );

  QString h;
  h = QString("%1").arg( docDate.weekNumber(), 2, 10, QChar('0') );
  m[ "%ww" ] = h;
  m[ "%w" ] = QString::number( docDate.weekNumber( ) );

  m[ "%dd" ] = docDate.toString( "dd" );
  m[ "%d" ] = docDate.toString( "d" );

  m[ "%m" ] = QString::number( docDate.month() );

  m[ "%MM" ] = docDate.toString( "MM" );
  m[ "%M" ] = docDate.toString( "M" );

  int i = id;
  if ( id == -1 ) { // hot mode: The database id is incremented by nextIdentId()
    i = nextIdentId();
  }

  h = QString("%1").arg(i, 6, 10, QChar('0') );
  m[ "%iiiiii" ] = h;

  h = QString("%1").arg(i, 5, 10, QChar('0') );
  m[ "%iiiii" ] = h;

  h = QString("%1").arg(i, 4, 10, QChar('0') );
  m[ "%iiii" ] = h;

  h = QString("%1").arg(i, 3, 10, QChar('0') );
  m[ "%iii" ] = h;

  h = QString("%1").arg(i, 2, 10, QChar('0') );
  m[ "%ii" ] = h;

  m[ "%i" ] = QString::number( i );

  m[ "%c" ] = addressUid;
  m[ "%type" ] = docType;
  m[ "%uid" ] = addressUid;

  QString re = KraftDB::self()->replaceTagsInWord( pattern, m );
  // qDebug () << "Generated document ident: " << re;

  return re;
}

// if hot, the id is updated in the database, otherwise not.
int DocType::nextIdentId( bool hot )
{
  QString numberCycle = numberCycleName();

  if ( numberCycle.isEmpty() ) {
    qCritical() << "NumberCycle name is empty";
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
    // qDebug () << "Got current number: " << num;

    if ( hot ) {
      QSqlQuery setQuery;
      setQuery.prepare( "UPDATE numberCycles SET lastIdentNumber=:newNumber WHERE name=:name" );
      setQuery.bindValue( ":name", numberCycle );
      setQuery.bindValue( ":newNumber", num );
      setQuery.exec();
      if ( setQuery.isActive() ) {
        // qDebug () << "Successfully created new id number for numbercycle " << numberCycle << ": " << num << endl;
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
    qCritical() << "Numbercycle for doctype is empty, returning default";
    mIdentTemplate = defaultTempl;
  }
  // qDebug () << "Picking ident Template for numberCycle " << numberCycle;

  q.prepare( "SELECT identTemplate FROM numberCycles WHERE name=:name" );

  q.bindValue( ":name", numberCycle );
  q.exec();
  if ( q.next() ) {
    tmpl = q.value( 0 ).toString();
    // qDebug () << "Read ident template from database: " << tmpl;
  }

  // FIXME: Check again.
  if ( tmpl.isEmpty() ) {
    // qDebug () << "Writing ident template to database: " << pattern;
    QSqlQuery insQuery;
    insQuery.prepare( "UPDATE numberCycles SET identTemplate=:pattern WHERE name=:name" );
    insQuery.bindValue( ":name", numberCycle );
    insQuery.bindValue( ":pattern", defaultTempl);
    insQuery.exec();
    tmpl = defaultTempl;
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
    // qDebug () << "Saving: not DIRTY!";
    return;
  }

  if ( !mNameMap.contains( mName ) ) {
    qCritical() << "nameMap does not contain id for " << mName;
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

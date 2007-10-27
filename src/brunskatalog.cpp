/***************************************************************************
                          brunskatalog.cpp  -
                             -------------------
    begin                : Mon Jul 11 2005
    copyright            : (C) 2003 by Klaas Freitag
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
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>

#include "dbids.h"
#include "brunskatalog.h"
#include "brunsrecord.h"
#include "katalogsettings.h"

BrunsKatalog::BrunsKatalog( const QString& n )
: Katalog( n ),
  m_wantToLower(true)
{
    m_chapterFile = KatalogSettings::brunsKeyFile();
    m_dataFile    = KatalogSettings::brunsDataFile();

    if( m_dataFile.isEmpty() ) {
        kdError() << "Unable to open bruns data file!" << endl;

        m_dataFile = KFileDialog::getOpenFileName( QString::null,
                                  "artikel2005.txt", 0,
                                  i18n("Select Bruns Catalog Data File") );
        KatalogSettings::setBrunsDataFile( m_dataFile );
        kdDebug() << "Set data file to " << m_dataFile << endl;
        KatalogSettings::writeConfig();

    } else {
        kdDebug() << "Opening bruns data file from " << m_dataFile << endl;
    }

    if( m_chapterFile.isEmpty() ) {
        kdError() << "Unable to open bruns key file!" << endl;

        m_chapterFile = KFileDialog::getOpenFileName( QString::null,
                                  "key_2006.txt", 0,
                                  i18n("Select Bruns Catalog Key File") );
        KatalogSettings::setBrunsKeyFile( m_chapterFile );
        KatalogSettings::writeConfig();
    } else {
        kdDebug() << "Opening bruns chapter file from " << m_chapterFile << endl;
    }

    setReadOnly( true );
}

BrunsKatalog::~BrunsKatalog() {}

QStringList BrunsKatalog::formatQuality( BrunsSize& bSize ) {
    QStringList res;

    int i = bSize.getFormNo();
    QString* str = m_forms[i];
    if( str ) {
      res << *str;
    } else {
      res << QString();
    }

    i = bSize.getFormAdd();
    str = m_formAdds[i];
    if( str ) {
      res << * str;
    } else {
      res << QString();
    }

    i = bSize.getSize();
    QString *h = m_sizes[i];

    i = bSize.getSizeAdd();
    str = m_sizeAdds[i];

    // kdDebug() << "H ist " << *h << " and Str ist " << str << endl;
    if( h && str ) {
      res << ( *h + " " + *str );
    } else if ( h ) {
      res << *h;
    }

    i = bSize.getRootPack();
    str = m_rootPacks[i];
    if( str ) {
      res << *str;

    } else {
        res << QString();
    }

    i = bSize.getQualityAdd();
    str = m_qualities[i];
    if( str ) {
      res << *str;
    } else {
      res << QString();
    }

    i = bSize.getGoodsGroup();
    str = m_goods[i];
    if( str ) {
      res << *str;
    } else {
      res << QString();
    }
    return res;
}

void BrunsKatalog::reload( dbID )
{

}

int BrunsKatalog::load()
{
  int cnt = 0;
  kdDebug() << "Loading brunskatalog from " << m_dataFile << endl;
  loadDBKeys();

  QFile file( m_dataFile );
  if ( file.open( IO_ReadOnly ) ) {
    QTextStream stream( &file );
    stream.setEncoding(QTextStream::Latin1);
    QString line;
    QString h;
    int d;
    bool ok = true;
    BrunsRecordList* recList;

    BrunsRecord *rec;
    rec = new BrunsRecord();

    while ( !stream.atEnd() ) {
      line = stream.readLine(); // line of text excluding '\n'
      d = intPart(line, 0,6);
      if( d > 0) {
        if( ! ok )
          kdDebug() << "failed to parse!" << endl;

        int pgroup = intPart(line, 12,18);
        int artID = intPart(line, 18, 24);
        // kdDebug() << "Have plant group " << pgroup << endl;

        BrunsSize size;
        size.setFormNo(intPart(line, 34, 38));
        size.setGrothNo(intPart(line, 38, 42));
        size.setRootPack(intPart(line, 42, 47));
        size.setQualityAdd(intPart(line, 52, 56));
        size.setFormAdd(intPart(line, 164, 168));
        size.setGoodsGroup(intPart(line, 267, 271));
        size.setPrimMatchcode( line.mid( 118, 23).stripWhiteSpace().local8Bit());
        size.setSizeAdd( intPart( line, 56, 60 ));
        size.setSize( intPart( line, 60, 64 ));

        if( rec->getArtId() == artID ) {
          // Only add an additional size
          rec->addSize(size);

        } else {
          // the record is new

          // save the last one away
          recList = m_recordLists[pgroup];
          if( ! recList ) {
            // create a new record list for this plantgroup */
            recList = new BrunsRecordList();
            m_recordLists.insert(pgroup, recList);
          }
          recList->append(rec);
          rec = new BrunsRecord();

          // and fill with the new data.
          rec->setPlantGroup(pgroup);
          rec->setArtId(intPart(line, 18, 24));
          rec->setArtMatch( line.mid(24, 10).local8Bit());

          QString n = line.mid( 271, 60 ).stripWhiteSpace();
          if( m_wantToLower ) {
            rec->setDtName( toLower( n ).local8Bit() );
          } else {
            rec->setDtName( n.local8Bit() );
          }
          n = line.mid( 331, 60  ).stripWhiteSpace();
          if( m_wantToLower ) {
            rec->setLtName( toLower( n ).local8Bit() );
          } else {
            rec->setLtName( n.local8Bit() );
          }

          rec->addSize(size);
        }
      }
    }
  } else {
    kdDebug() << "Unable to open " << m_dataFile << endl;
  }
  return cnt;
}

inline QString BrunsKatalog::toLower( const QString& line )
{
    QStringList li = QStringList::split( QChar(' '), line  );
    QString re;

    for ( QStringList::Iterator it = li.begin(); it != li.end(); ++it ) {
        re += toLowerWord( *it ) + " ";
    }
    return re;
}

inline QString BrunsKatalog::toLowerWord( const QString& str )
{
    if( str.length() < 2 ) return str;
    if( str.startsWith( "(" )) return str;

    bool quoted = false;

    if( str.startsWith( "'" ) ) {
        quoted = true;
    }
    QChar firstChar = str[0];
    if( quoted ) firstChar = str[1];

    QString re = str.lower();
    if( quoted )
        re[1] = firstChar;
    else
        re[0] = firstChar;

    return re;
}


BrunsRecordList* BrunsKatalog::getRecordList( const QString& chap )
{
    int id = chapterID(chap);
    if( id )
        return m_recordLists[id];
    else
        return 0;
}

inline int BrunsKatalog::intPart( const QString& str, int from, int to ) {
    bool ok = true;
    const QString s = str.mid(from, to-from);
    // kdDebug() << ">" << s << "<" << endl;
    return s.toInt(&ok, 10);
}

QStringList BrunsKatalog::getKatalogChapters() {
    return m_chapters;
}

void BrunsKatalog::loadDBKeys() {
    QStringList lines;
    QFile file( m_chapterFile );

    if ( file.open( IO_ReadOnly ) ) {
        QTextStream stream( &file );
        stream.setEncoding(QTextStream::Latin1);
        QString line;

        KatMap *currDict = 0; // m_chapterIDs;
        KatMap *longDict = 0;
        bool doChapters = false;
        const QRegExp rxpZusatz = QRegExp( "Tabelle der Gr.+senzus.+tze:", TRUE, FALSE );
        const QRegExp rxpStufe = QRegExp( "Tabelle der Gr.+senstufen:", TRUE, FALSE );

        while ( !stream.atEnd() ) {
            line = stream.readLine();

            QStringList li = QStringList::split(QChar(0x09), line );
            line = line.simplifyWhiteSpace();

            bool ok;

            int id = li[0].toInt( &ok, 10 );  // checks if convertible to number

            if( ok ) {
                QString katName = li[1];

                if( doChapters ) { // maintain Stringlist only for chapters.
                    m_chapterIDs->insert(katName, new dbID(id));
                    m_chapters.append(katName);
                } else {
                // kdDebug() << "Inserting Brunskatalog name " << katName << endl;
                    if( currDict == &m_rootPacks ) {
                        kdDebug() << "inserting RootPack: " << katName << endl;
                    }
                    if( currDict ) {
                        currDict->insert(id, new QString(katName));
                    }
                    if( longDict ) {
                        QString *str;
                        if( li.size() > 1 ) {
                            str = new QString( li[2] );
                        } else {
                            str = new QString();
                        }
                        longDict->insert(id, str );
                    }
                }
            } else {
                // kdDebug() << "THis is line : " << line << endl;
                if( line == "Tabelle der Pflanzengruppen:" ) {
                    doChapters = true;
                } else if( line == "Tabelle der Warenengruppen:" ) {
                    kdDebug() << "Loading Warengruppen" << endl;
                    currDict = &m_goods;
                    longDict = 0;
                    doChapters = false;
                } else if( line.startsWith("Tabelle der Formzus") ) {
                    kdDebug() << "Loading Formzusätze" << endl;
                    currDict = &m_formAdds;
                    longDict = &m_formAddsLong;
                    doChapters = false;
                } else if( line == "Tabelle der Formen:") {
                    kdDebug() << "Loading Formen" << endl;
                    currDict = &m_forms;
                    longDict = &m_formsLong;
                    doChapters = false;
                } else if( line == "Tabelle der Wuchsarten:") {
                    kdDebug() << "Loading Wuchsarten" << endl;
                    currDict = &m_grows;
                    longDict = 0;
                    doChapters = false;
                } else if( line == "Tabelle der Wurzelverpackungen:") {
                    kdDebug() << "Loading Wurzelverpackungen" << endl;
                    currDict = &m_rootPacks;
                    longDict = 0;
                    doChapters = false;
                }  else if( line.startsWith( "Tabelle der Qualit" ) ) { // \u00e4tszus\u00e4tze:") {
                    kdDebug() << "Loading Qualitätszusätze" << endl;
                    currDict = &m_qualities;
                    longDict = &m_qualitiesLong;
                    doChapters = false;
                } else if( line.contains( rxpZusatz ) ) {
                    kdDebug() << "Loading Grössenzusätze" << endl;
                    currDict = &m_sizeAdds;
                    longDict = &m_sizeAddsLong;
                    doChapters = false;
                } else if( line.contains( rxpStufe ) ) {
                    kdDebug() << "Loading Grössenstufen" << endl;
                    currDict = &m_sizes;
                    longDict = 0;
                    doChapters = false;
                }
            }
        }
        file.close();
    }
}

KatMap BrunsKatalog::m_goods;
KatMap BrunsKatalog::m_formAdds;
KatMap BrunsKatalog::m_formAddsLong;
KatMap BrunsKatalog::m_forms;
KatMap BrunsKatalog::m_formsLong;
KatMap BrunsKatalog::m_grows;
KatMap BrunsKatalog::m_rootPacks;
KatMap BrunsKatalog::m_qualities;
KatMap BrunsKatalog::m_qualitiesLong;
KatMap BrunsKatalog::m_sizeAdds;
KatMap BrunsKatalog::m_sizeAddsLong;
KatMap BrunsKatalog::m_sizes;


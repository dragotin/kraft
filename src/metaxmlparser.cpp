/***************************************************************************
               metaxmlparser.cpp - Parser for Meta XML files
                             -------------------
    begin                : Dec 29 2018
    copyright            : (C) 2018 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QDebug>

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include "metaxmlparser.h"

MetaXMLParser::MetaXMLParser()
{

}

bool MetaXMLParser::parse( QIODevice *device )
{
    _docTypeAddList.clear();

    if (!_domDocument.setContent(device, true, &_errorString, &_errorLine, &_errorColumn)) {
        qDebug() << "Not able to parse XML: " << _errorString << "Line"<< _errorLine << ", Colum" << _errorColumn;
        return false;
    }

    QDomElement root = _domDocument.documentElement();
    if( root.tagName() != "kraftmeta" ) {
        qDebug() << "XML file parse error: Not a kraftmeta root";
    }
    QDomElement migrateElem = root.firstChildElement("migrate");

    QDomElement addDtXml = migrateElem.firstChildElement("doctype");

    while( ! addDtXml.isNull() ) {
        MetaDocTypeAdd docTypeAdd;
        QDomElement elem = addDtXml.firstChildElement("name");
        if( !elem.isNull() ) {
            docTypeAdd.setName(elem.text());
        }
        elem = addDtXml.firstChildElement("numbercycle");
        if( !elem.isNull() ) {
            docTypeAdd.setNumbercycle(elem.text());
        }
        elem = addDtXml.firstChildElement("lang");
        if( !elem.isNull() ) {
            docTypeAdd.setLang(elem.text());
        }

        elem = addDtXml.firstChildElement("attrib");
        while( !elem.isNull() ) {
            const QString key = elem.firstChildElement("key").text();
            const QString val = elem.firstChildElement("value").text();
            if( !key.isEmpty() )
                docTypeAdd._attribs.insert(key, val);
            elem = elem.nextSiblingElement("attrib");
        }

        elem = addDtXml.firstChildElement("follower");
        while( !elem.isNull() ) {
            const QString fo = elem.text();
            if( !fo.isEmpty() )
                docTypeAdd._follower.append(fo);
            elem = elem.nextSiblingElement("follower");
        }
        // save and move on to next element.
        if( !docTypeAdd.name().isNull() ) {
            _docTypeAddList.append(docTypeAdd);
        }

        addDtXml = addDtXml.nextSiblingElement("doctype");
    }

    return true;
}

QList<MetaDocTypeAdd> MetaXMLParser::metaDocTypeAddList()
{
    return _docTypeAddList;
}

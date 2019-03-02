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

#ifndef METAXMLPARSER_H
#define METAXMLPARSER_H

#include <QObject>
#include <QtXml>
#include <QMap>


class MetaDocTypeAdd
{
public:
    void setName( const QString& name ) { _name = name; }
    QString name() const { return _name; }

    void setNumbercycle( const QString& name ) { _numbercycle = name; }
    QString numbercycle() const { return _numbercycle; }

    void setLang( const QString& name ) { _lang = name; }
    QString lang() const { return _lang; }

    QMap<QString, QString> _attribs;
    QStringList _follower;
private:
    QString _name;
    QString _lang;
    QString _numbercycle;
};


class MetaXMLParser
{
public:
    MetaXMLParser();
    bool parse( QIODevice *device );

    QList<MetaDocTypeAdd> metaDocTypeAddList();
private:
    QDomDocument _domDocument;
    QString _errorString;
    int _errorLine, _errorColumn;

    QList<MetaDocTypeAdd> _docTypeAddList;
};

#endif // METAXMLPARSER_H

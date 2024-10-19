/***************************************************************************
                docidentgenerator.h - Async doc ident generator
                             -------------------
    begin                : 12/2023
    copyright            : (C) 2023 by Klaas Freitag
    email                : opensource@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOCIDENTGENERATOR_H
#define DOCIDENTGENERATOR_H

#include <QObject>

class KraftDoc;

class DocIdentGenerator : public QObject
{
    Q_OBJECT
public:
    explicit DocIdentGenerator(QObject *parent = nullptr);

    bool generate(KraftDoc *doc);

    // returns an error string if the return string of newIdent is empty
    QString errorStr() const;
Q_SIGNALS:
    void newIdent(const QString&);

private:
    QString _error;

};

#endif // DOCIDENTGENERATOR_H

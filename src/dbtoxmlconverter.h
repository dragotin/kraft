/***************************************************************************
             DbToXMLConverter  - Convert the DB to XML
                             -------------------
    begin                : Feb. 2021
    copyright            : (C) 2021 by Klaas Freitag
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

#ifndef DBTOXMLCONVERTER_H
#define DBTOXMLCONVERTER_H

#include <QObject>

class DbToXMLConverter : public QObject
{
    Q_OBJECT
public:
    explicit DbToXMLConverter(QObject *parent = nullptr);

    QMap <int, int> yearMap();

    int convertDocs(int year);
    int amountOfDocs(int year);

signals:

};

#endif // DBTOXMLCONVERTER_H

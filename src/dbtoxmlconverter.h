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
#include <QPair>

class DbToXMLConverter : public QObject
{
    Q_OBJECT
public:
    explicit DbToXMLConverter(QObject *parent = nullptr);

    void convert();

private:
    QMap <int, int> yearMap();

    QPair<int, int> convertDocsOfYear(int year, const QString& basePath);
    bool convertLatestPdf(const QString &basePath, const QString& ident, const QString& uuid);
    int amountOfDocsOfYear(int year);
    int convertNumbercycles(const QString &baseDir);

signals:

};

#endif // DBTOXMLCONVERTER_H

/***************************************************************************
             DocumentSaverXML  - Save Documents as XML
                             -------------------
    begin                : Jan. 2021
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

#ifndef _DocumentSaverXML_H
#define _DocumentSaverXML_H

#include "documentsaverbase.h"
#include "geld.h"

#include <QDir>
#include <QXmlSchema>
#include <QDomDocument>

class KraftDoc;
class QSqlRecord;
class dbID;
class QString;
class DocBaseModel;

namespace XML {
struct Totals {
    Geld _netto;
    Geld _brutto;
    Geld _redTax;
    Geld _fullTax;
};
}

class DocumentSaverXML : public DocumentSaverBase
{
    Q_OBJECT

public:

    DocumentSaverXML();
    virtual ~DocumentSaverXML();

    virtual bool saveDocument(KraftDoc*) override;
    virtual bool loadByIdent( const QString& , KraftDoc * ) override;
    virtual bool loadByUuid(const QString&, KraftDoc*) override;

    int addDigestsToModel(DocBaseModel *model) override;

    bool loadFromFile(const QFileInfo& xmlFile, KraftDoc *doc, bool onlyMeta = false);

    void setBasePath(const QString& path);
    void setArchiveMode(bool am);
    QString xmlDocFileName(KraftDoc *doc);
    QString xmlDocFileNameFromIdent(const QString& id);

    bool verifyXmlFile(const QUrl& schemaFile, const QString& xmlFile);

    QString lastSavedFileName() const;

    XML::Totals getLastTotals() const;

protected:
    QString basePath();

private:
    QDir _basePath;
    QString _lastSaveFile;
    XML::Totals _totals;
    bool _archiveMode;
};

#endif

/* END */


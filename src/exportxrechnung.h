/***************************************************************************
             exporterXRechnung  - Save Documents as XRechnung
                             -------------------
    begin                : Feb. 2022
    copyright            : (C) 2022 by Klaas Freitag
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

#ifndef _EXPORTERXRECHNUNG_H
#define _EXPORTERXRECHNUNG_H

#include <QDir>
#include <QObject>
#include <QScopedPointer>
#include <QMap>
#include <QStringList>
#include <QVariantHash>

#include "kraftcontact.h"
#include "kraftdoc.h"

class QSqlRecord;
class dbID;
class QString;

class ExporterXRechnung : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    /**
     * @brief xRechnungTmpFile
     * emits the file name of a temporary file that is the XRechnung result file.
     * Needs to be deleted after copied to the right target file.
     */
    void xRechnungTmpFile(const QString&);

public:
    ExporterXRechnung(QObject *parent = nullptr);
    virtual ~ExporterXRechnung();

    void setDueDate(const QDate&);
    void setBuyerRef(const QString&);
    bool exportDocument(const QString&);
    QString error() { return _error; };

protected:
    QStringList missingSellerData(const KContacts::Addressee& myContact,
                                  const QMap<QString, QString>& own) const;
    QStringList missingBuyerData(const KContacts::Addressee& customer) const;

protected Q_SLOTS:
    void slotAddresseeFound(const QString &uid = QString(), const KContacts::Addressee &contact = KContacts::Addressee());

private:
    QString templateFile() const;

    bool _validateWithSchema;
    // The customer of the document that is exported, valid for one export.
    KraftContact *_customer;
    QString _uuid;
    QString _docTypeStr;
    QString _buyerRef;
    QDate _dueDate;
    QString _error;
};

#endif

/* END */


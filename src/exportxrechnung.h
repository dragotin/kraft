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

#include "addressprovider.h"

class QSqlRecord;
class dbID;
class QString;

class ExporterXRechnung : public QObject
{
    Q_OBJECT

signals:
    /**
     * @brief xRechnungTmpFile
     * emits the file name of a temporary file that is the XRechnung result file.
     * Needs to be deleted after copied to the right target file.
     */
    void xRechnungTmpFile(const QString&);

public:
    ExporterXRechnung(QObject *parent = nullptr);
    virtual ~ExporterXRechnung();

    virtual bool exportDocument(const QString &uuid);
    QString templateFile() const;

    void setDueDate(const QDate&);
    void setBuyerRef(const QString&);

protected:
    void lookupCustomerAddress();

protected slots:
    void slotAddresseeFound(const QString &uid = QString(), const KContacts::Addressee &contact = KContacts::Addressee());
    void slotSkipLookup();

private:
    bool _validateWithSchema;
    AddressProvider *mAddressProvider;
    KContacts::Addressee _customerContact;

    QString _buyerRef;
    QDate _dueDate;
};

#endif

/* END */


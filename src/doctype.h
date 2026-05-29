/***************************************************************************
                 doctype.h - doc type class
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
#ifndef DOCTYPE_H
#define DOCTYPE_H

// include files for Qt
#include <QString>
#include <QMap>
#include <QDomDocument>

#include "kraftcat_export.h"

#include "attribute.h"
#include "kraftobj.h"
#include "xmldirlister.h"

/**
@author Klaas Freitag
*/

class KRAFTCAT_EXPORT DocType
        :public KraftObj
{
public:
    DocType();

    static const QString AllowDemandStr;
    static const QString AllowAlternativeStr;
    static const QString HidePricesStr;
    static const QString SubstPartialInoiceStr;
    static const QString PartialInvoiceStr;
    static const QString XRechnungTmplStr;
    static const QString WatermarkFileStr;
    static const QString DocTemplateFileStr;
    static const QString IdentNumberCycleStr;
    static const QString DocMergeIdentStr;
    static const QString DayCounterDateStr;
    static const QString DayCounterStr;
    static const QString AppendPDFStr;
    static const QString IsInvoiceStr;
    static const QString XRechnungEnabledStr;
    static const QString NeedsArchivingStr;

    static const QString DefaultTmplFileName;

    QString name() const;
    void setName( const QString& );

    bool allowDemand() const;
    void setAllowDemand(bool);

    bool allowAlternative() const;
    void setAllowAlternative(bool);

    bool pricesHidden() const;
    void setPricesHidden(bool);

    bool isInvoice() const;
    void setIsInvoice(bool);

    bool isXRechnungEnabled() const;
    void setXRechnungEnabled(bool);

    bool partialInvoice() const;
    void setPartialInvoice(bool);

    bool substractPartialInvoice() const;
    void setSubstractPartialInvoice(bool);

    bool needsArchiving() const;
    void setNeedsArchiving(bool);

    QStringList follower() const;
    void setFollowers(const QStringList& followers);

    QString     numberCycleName() const;
    void        setNumberCycleName( const QString& );

    QString     templateFile() const;
    void        setTemplateFile( const QString& );

    QString     watermarkFile() const;
    void        setWatermarkFile( const QString& );

    int         mergeIdent() const;
    void        setMergeIdent(int);

    QString     xRechnungTemplate() const;
    void        setXRechnungTemplate(const QString&);

    QString     appendPDF() const;
    void        setAppendPDFFile(const QString& file);

    const QString toXml() const;
    void        parseXml(QDomDocument &domDoc);

    void        readIdentTemplate();

    bool        isEmpty();

private:
    QStringList  mFollowerList;
    QString      mName;
    QString      mLocale;

    bool dtFlag(const QString& str) const;
    void setDtFlag(const QString& name, bool f);
    QString attributeValueString(const QString& attribName) const;
    void setStringAttribute( const QString& attribName, const QString& val, const QString& defaultValue = QString());
};


class KRAFTCAT_EXPORT DocTypes
        : public XmlDirLister<DocType>
{
public:
    DocTypes();

    QStringList allNames();
private:
    bool saveDTXml(const QString& name, const QString& xml, const QString& baseDir = QString());

    bool tryLock();
    void unlock();
};
#endif

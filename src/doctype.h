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

#include "dbids.h"
#include "attribute.h"
#include "kraftobj.h"
#include "lister.h"
#include "defaultprovider.h"

/**
@author Klaas Freitag
*/

typedef QMap<QString, dbID> idMap;

class KRAFTCAT_EXPORT DocType
        :public KraftObj
{
public:
    DocType();
    /**
   * create a doctype from its localised or tech name
   */
    DocType( const QString&, bool dirty = false );

    QString name() const;
    void setName( const QString& );

    bool allowDemand() const;
    void setAllowDemand(bool);

    bool allowAlternative() const;
    void setAllowAlternative(bool);

    bool pricesHidden() const;
    void setPricesHidden(bool);

    bool partialInvoice() const;
    void setPartialInvoice(bool);

    bool substractPartialInvoice() const;
    void setSubstractPartialInvoice(bool);

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

    static void  clearMap();

    const QString toXml() const;
    void        parseXml(QDomDocument &domDoc);

    void        readIdentTemplate();

    bool        isXRechnungEnabled() const;
    void        setXRechnungEnabled(bool);

private:
    QStringList  mFollowerList;
    QString      mName;
    bool         mDirty;

    bool dtFlag(const QString& str) const;
    void setDtFlag(const QString& name, bool f);
    QString attributeValueString(const QString& attribName) const;
    void setStringAttribute( const QString& attribName, const QString& val, const QString& defaultValue = QString());

    static idMap mNameMap;
};

class KRAFTCAT_EXPORT DocTypes
        : public Lister<DocType>
{
public:
    DocTypes();

    QStringList all();
    QStringList allLocalised();

private:
    bool saveDTXml(const QString& name, const QString& xml, const QString& baseDir = QString());

    bool tryLock();
    void unlock();
};
#endif

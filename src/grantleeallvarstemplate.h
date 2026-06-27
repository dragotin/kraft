/***************************************************************************
        Description of template variabbles for Kraft Documents
                             -------------------
    begin                : June 2026
    copyright            : (C) 2026 by Klaas Freitag
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

#ifndef GRANTLEEALLVARSTEMPLATE_H
#define GRANTLEEALLVARSTEMPLATE_H

#include "documenttemplate.h"

class TemplateVariable : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString desc READ desc)
    Q_PROPERTY(QString example READ example)

public:
    TemplateVariable(const QString& name, const QString& desc, const QString& example);

    QString name() const { return _name; }
    QString desc() const { return _desc; }
    QString example() const { return _exampleVal; }

private:
    QString _name;
    QString _desc;
    QString _exampleVal;
};

// ==================================================================================

class TemplateNameSpace : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString description READ desc WRITE setDesc)
    Q_PROPERTY(QList<TemplateVariable*> vars READ vars)
    Q_PROPERTY(QString prefix READ prefix)

public:
    explicit TemplateNameSpace(const QString& p);
    ~TemplateNameSpace();

    QList<TemplateVariable*> vars() const { return _vars; }
    QString desc() const { return _desc; }
    void setDesc(const QString& d) { _desc = d; }
    void addVar(TemplateVariable* var) {
        _vars.append(var);
    }
    QString prefix() const { return _prefix; }

private:
    QList<TemplateVariable*> _vars;
    QString _desc;
    QString _prefix;

};


// ==================================================================================

class GrantleeAllVarsTemplate : public DocumentTemplate
{
public:
    GrantleeAllVarsTemplate(const QString& tmplFile);

    const QString expand(const QString& uuid,
                         const KContacts::Addressee &myContact,
                         const KContacts::Addressee &customerContact) override;

};

#endif // GRANTLEEALLVARSTEMPLATE_H

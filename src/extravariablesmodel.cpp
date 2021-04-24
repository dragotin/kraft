/***************************************************************************
   ExtraVariabels - Model to handle extra variables that appear in docs
                             -------------------
    begin                : Mar. 2021
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

#include "extravariablesmodel.h"

#include "klocalizedstring.h"

ExtraVariable::ExtraVariable(const QString n, const QString& kind, const QString&val)
    :_name(n),
      _kind(Invalid),
      _value(val)
{
    if (kind == kindToStr(Text)) _kind = Text;
    else if (kind == kindToStr(Date)) _kind = Date;
    else if (kind == kindToStr(Number)) _kind = Number;
}

QString ExtraVariable::kindToStr(const Kind& kind)
{
    QString re;

    switch (kind) {
    case Invalid: re = QStringLiteral("Invalid"); break;
    case Text:    re = QStringLiteral("Text"); break;
    case Date:    re = QStringLiteral("Date"); break;
    case Number:  re = QStringLiteral("Number"); break;
    }
    return re;
}

QString ExtraVariable::kindToI18n(const Kind& kind)
{
    QString re;

    switch (kind) {
    case Invalid: re = i18n("Invalid"); break;
    case Text:    re = i18n("Text"); break;
    case Date:    re = i18n("Date"); break;
    case Number:  re = i18n("Number"); break;
    }
    return re;

}

QString ExtraVariable::kindToStr()
{
    return kindToStr(_kind);
}

QString ExtraVariable::kindToI18n()
{
    return kindToI18n(_kind);
}
/* ===============================================================================*/

ExtraVariablesModel::ExtraVariablesModel(QObject *parent)
    :QAbstractTableModel(parent)
{
    _extraVars.append(ExtraVariable("Paydate", "Date", "$DOCDATE+7"));
}

QList<ExtraVariable> ExtraVariablesModel::extraVariables()
{
    return _extraVars;
}

void ExtraVariablesModel::setVariablesList(QList<ExtraVariable> list)
{
    _extraVars = list;
}

int ExtraVariablesModel::rowCount(const QModelIndex & /*parent*/) const
{
   return _extraVars.count();
}

int ExtraVariablesModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant ExtraVariablesModel::data(const QModelIndex &index, int role) const
{
    QString re;
    if (role == Qt::DisplayRole)
        if (index.row() < _extraVars.size() ) {
            ExtraVariable var = _extraVars.at(index.row());

            if (index.column() == 0)
                re = var.name();
            else if (index.column() == 1)
                re = var.kindToI18n();
            else if (index.column() == 2)
                re = var.value();
            else re = QStringLiteral("out of bounds");
        }
       return re;
}

QVariant ExtraVariablesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return i18n("Name");
        case 1:
            return i18n("Type");
        case 2:
            return i18n("Value");
        }
    }
    return QVariant();
}

bool ExtraVariablesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (!checkIndex(index))
            return false;
        //save value from editor to member m_gridData
        if (index.column() == 2) { // only set the value so far.
            int r = index.row();
            _extraVars[r].setValue(value.toString());
        }
        return true;
    }
    return false;
}

Qt::ItemFlags ExtraVariablesModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

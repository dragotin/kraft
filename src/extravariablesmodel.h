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

#ifndef EXTRAVARIABLESMODEL_H
#define EXTRAVARIABLESMODEL_H

#include <QAbstractTableModel>

const int COLS= 3;
const int ROWS= 2;

class ExtraVariable
{
public:
    explicit ExtraVariable(const QString n, const QString& kind, const QString&val);

    enum Kind {
        Invalid,
        Text,
        Date,
        Number
    };

    QString name() { return _name; }

    QString kindToStr(const Kind& kind);
    QString kindToStr();
    QString kindToI18n(const Kind& kind);
    QString kindToI18n();
    QString value() { return _value; }

    void setValue( const QString& val) {_value = val;}

private:
    QString _name;
    Kind    _kind;
    QString _value;
};


class ExtraVariablesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ExtraVariablesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QList<ExtraVariable> extraVariables();
    void setVariablesList(QList<ExtraVariable> list);

private:

    QList<ExtraVariable> _extraVars;


signals:
    void editCompleted(const QString &);

};

#endif // EXTRAVARIABLESMODEL_H

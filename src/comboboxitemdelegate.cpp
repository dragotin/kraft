/***************************************************************************
   ComboBoxItemDelete - a delegate that shows an Combobox
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

#include "comboboxitemdelegate.h"

#include "comboboxitemdelegate.h"
#include <QComboBox>

#include <klocalizedstring.h>

ComboBoxItemDelegate::ComboBoxItemDelegate()
{

}

QWidget *ComboBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    QComboBox *cb = new QComboBox(parent);
    cb->addItem(i18n("Text"));
    cb->addItem(i18n("Date"));
    cb->addItem(i18n("Number"));
    return cb;
}

void ComboBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {

    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    // Lookup current string
    const QString currentText = index.data(Qt::EditRole).toString();
    const int cbIndex = cb->findText(currentText);
    // ...to set it in the combo box
    if (cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);

}


void ComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {

    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    model->setData(index, cb->currentText(), Qt::EditRole);

}



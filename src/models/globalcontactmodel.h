/*
    This file is part of KAddressBook.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef GLOBALCONTACTMODEL_H
#define GLOBALCONTACTMODEL_H

namespace Akonadi
{
  class ChangeRecorder;
  class ContactsTreeModel;
  class Monitor;
  class Session;
}

/**
 * @short Provides the global model for all contacts
 *
 * This model provides the EntityTreeModel for all contacts.
 * The model is accessable via the static instance() method.
 */
class GlobalContactModel
{
  public:
    /**
     * Destroys the global contact model.
     */
    ~GlobalContactModel();

    /**
     * Returns the global contact model instance.
     */
    static GlobalContactModel* instance();

    /**
     * Returns the item model of the global instance.
     */
    Akonadi::ContactsTreeModel* model() const;

  private:
    GlobalContactModel();

    static GlobalContactModel *mInstance;

    Akonadi::Session *mSession;
    Akonadi::ChangeRecorder *mMonitor;
    Akonadi::ContactsTreeModel *mModel;
};

#endif

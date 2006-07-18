//
// C++ Interface: catalogchapteredit
//
// Description: 
//
//
// Author: Klaas Freitag <freitag@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CATALOGCHAPTEREDIT_H
#define CATALOGCHAPTEREDIT_H

#include <keditlistbox.h>
#include <kdialogbase.h>

#include <dbids.h>
/**
@author Klaas Freitag
*/
class CatalogChapterEdit : public KEditListBox
{
    Q_OBJECT
public:
    CatalogChapterEdit(QWidget*);

    virtual ~CatalogChapterEdit();

    void setCatalog( const QString& );
};

class Katalog;

class CatalogChapterEditDialog: public KDialogBase
{
    Q_OBJECT
public:
    CatalogChapterEditDialog(QWidget*, const QString& );
    bool dirty() { return mDirty; }
public slots:
    void accept();    
    
protected slots:
    void slotAdded( const QString& );
    void slotRemoved( const QString& );
    void slotSelectionChanged();
    void slotTextChanged();
private:
    QStringList m_newItems;
    QStringList m_removedItems;

    CatalogChapterEdit *m_chapEdit;
    Katalog *m_katalog;
    dbIdDict mEntryDict;
    QString mLastSelection;
    bool mDirty;
};


#endif

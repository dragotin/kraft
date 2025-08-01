/***************************************************************************
           alldocsview.h - digest view of all docs with filter
                             -------------------
    begin                : Sat Mar 11 2017
    copyright            : (C) 2017 by Klaas Freitag
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
#ifndef ALLDOCSVIEW_H
#define ALLDOCSVIEW_H

#include <QWidget>
#include <QMap>
#include <QTreeWidgetItem>
#include <QTableView>
#include <QVector>
#include <QStackedWidget>

#include "models/datemodel.h"
#include "docdigest.h"
#include "docguardedptr.h"
#include "models/documentproxymodels.h"

class QPushButton;
class dbID;
class QContextMenuEvent;
class QToolBox;
class DocDigestDetailView;

class AllDocsView : public QWidget
{
    Q_OBJECT

public:
    typedef enum {FlatList, TreeView} ViewType;

    AllDocsView( QWidget *parent = 0 );
    ~AllDocsView();

    int currentDocumentRow() const;

    QString currentDocumentIdent( ) const;
    QString currentDocumentUuid( ) const;

    QVector<QMenu*> contextMenus();

    void initDetailViewActions(const std::array<QAction*, 4> actions);
public Q_SLOTS:

    void slotBuildView();
    void slotUpdateView(DocGuardedPtr doc);

    void setView(ViewType type);
    void setErrorMsg(const QString& header, const QString& details);

protected:
    void contextMenuEvent( QContextMenuEvent* );
    QWidget *initializeTreeWidget();

protected Q_SLOTS:
    void slotCurrentChanged( QModelIndex, QModelIndex );
    void slotSearchTextChanged(const QString& newStr );
    void slotAmountFilterChanged(int entryNo);

Q_SIGNALS:
    void openDocument();
    void docSelected(const QString&);

private:

    QTableView *_tableView;
    QTreeView  *_dateView;
    QStackedWidget *_stack;

    DocDigestDetailView *mAllViewDetails;

    QModelIndex mCurrentlySelected;

    DocumentFilterModel *mTableModel;
    DocumentFilterModel *mDateModel;

    QMenu *mAllMenu;

    QPushButton            *mNewDocButton;
    QLineEdit              *_searchLine;
};

#endif

/***************************************************************************
                          katalogview.h
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by Klaas Freitag
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

#ifndef KATALOGVIEW_H
#define KATALOGVIEW_H

#include <QAction>
#include <QUrl>
#include <QMainWindow>

#include "kraftcat_export.h"

class KatalogListView;
class Katalog;
class FilterHeader;
class CatalogWidget;
class QBoxLayout;
class QActionMenu;
class DocPosition;
class CalcPartList;
class QTreeWidgetItem;
class QLabel;
class QProgressBar;
class CatalogTemplate;

/**
  * The base class for Kraft katalog view.
  *
  * @author Klaas Freitag <kraft@freisturz.de>
  * @version $Id$
  */
class KRAFTCAT_EXPORT KatalogView : public QMainWindow
{
  Q_OBJECT

  public:
    /** construtor of a catalog view.
     * Note: init must be called immediately after instanciating a inherited
     * class of KatalogView
     */
    KatalogView(QWidget* parent=0, const char* name=0);
    virtual ~KatalogView();

    /**
     * create a special listview for the kind of catalog. This method must
     * be overwritten in inherited catalog view classes.
     */
    virtual void createCentralWidget(QBoxLayout*, QWidget*);
    virtual KatalogListView* getListView(){return 0;};
    virtual void init( const QString& );

  protected:
    virtual Katalog* getKatalog( const QString& );

  public slots:
    /** clears the document in the current view to reuse it as the new document */
    void openDocumentFile(const QUrl &url);

     /** changes the statusbar contents for the standard label permanently, used to indicate current actions.
     * @param text the text that is displayed in the statusbar
     */
    void slotStatusMsg(const QString &text);

    virtual void slTreeviewItemChanged( QTreeWidgetItem *, QTreeWidgetItem *);
    void slExport();
    void slImport();

    // virtual void slEditChapters();
    virtual void slAddSubChapter();
    virtual void slEditSubChapter();
    virtual void slRemoveSubChapter();
    virtual void slNewTemplate() = 0;
    virtual void slEditTemplate() = 0;
    virtual void slDeleteTemplate() = 0;

    void slotShowTemplateDetails( CatalogTemplate*);
    void setProgressValue( int );

    void closeEvent( QCloseEvent *event );

protected slots:
    void slotSaveState();

  protected:

    /** the configuration object of the application */
    QAction* m_acEditChapter;
    QAction* m_acAddChapter;
    QAction* m_acRemChapter;

    QAction* m_acEditItem;
    QAction* m_acNewItem;
    QAction* m_acDeleteItem;
    QAction* m_acExport;
    QAction* m_acImport;

    // KToggleAction* viewToolBar;
    // KToggleAction* viewStatusBar;
    QString            m_katalogName;
    FilterHeader      *m_filterHead;
    QTreeWidgetItem   *m_editListViewItem;
    QLabel            *mTemplateText;
    QLabel            *mTemplateStats;
    QProgressBar      *mProgress;
    // Fills the DocPosition with the data from the currently selected item in the view
    virtual bool currentItemToDocPosition( DocPosition& ){ return false; }

    /** initializes the QActions of the application */
    void initActions();
    /** sets up the statusbar for the main window by initialzing a statuslabel.
     */

    /** initializes the document object of the main window that is connected to the view in initView().
     * @see initView();
     */
    void initView();

    /** Save the state of the window to the right settings value. Needs to be reimplemented in the special window implementation,
     * thus virtual here.
     */
    virtual void saveWindowState( const QByteArray& arr ) = 0;
    virtual QByteArray windowState() = 0;

    /** Save the geomentry of the window to the right settings value. Needs to be reimplemented in the special window implementation,
     * thus virtual here.
     */
    virtual void saveWindowGeo( const QByteArray& arr ) = 0;
    virtual QByteArray windowGeo() = 0;

};

#endif // KATALOGVIEW_H

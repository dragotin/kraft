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

// include files for Qt

// include files for KDE
#include <kapplication.h>
#include <kxmlguiwindow.h>
#include <kaction.h>
#include <kurl.h>

#include "kraftcat_export.h"

class KatalogListView;
class Katalog;
class FilterHeader;
class CatalogWidget;
class QBoxLayout;
class KActionMenu;
class DocPosition;
class CalcPartList;
class QTreeWidgetItem;
class QLabel;
class QProgressBar;
class CatalogTemplate;

/**
  * The base class for Kraft katalog view.
  * @see KMainWindow
  * @see KApplication
  * @see KConfig
  *
  * @author Klaas Freitag <freitag@kde.org>
  * @version $Id$
  */
class KRAFTCAT_EXPORT KatalogView : public KXmlGuiWindow
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
    /** open a new application window by creating a new instance of KraftApp */
    void slotFileNewWindow();
    /** clears the document in the current view to reuse it as the new document */
    void openDocumentFile(const KUrl& url);

    void slotFileOpen();
    /** save a document */
    void slotFileSave();
    /** asks for saving if the file is modified, then closes the current file and window*/
    void slotFileClose();
    /** print the current file */
    void slotFilePrint();
    /** put the marked text/object into the clipboard and remove
     *	it from the document
     */
    void slotEditCut();
    /** put the marked text/object into the clipboard
     */
    void slotEditCopy();
    /** paste the clipboard into the document
     */
    void slotEditPaste();

    /** changes the statusbar contents for the standard label permanently, used to indicate current actions.
     * @param text the text that is displayed in the statusbar
     */
    void slotStatusMsg(const QString &text);

    virtual void slTreeviewItemChanged( QTreeWidgetItem *, QTreeWidgetItem *);
    void slExport();

    // virtual void slEditChapters();
    virtual void slAddSubChapter();
    virtual void slEditSubChapter();
    virtual void slRemoveSubChapter();

    void slotShowTemplateDetails( CatalogTemplate*);
    void setProgressValue( int );

  protected:

    /** the configuration object of the application */
    KConfig *config;
    KAction* m_acFileClose;
    KAction* m_acFilePrint;
    KAction* m_acEditCut;
    KAction* m_acEditCopy;
    KAction* m_acEditPaste;

    KAction* m_acEditChapter;
    KAction* m_acAddChapter;
    KAction* m_acRemChapter;

    KAction* m_acEditItem;
    KAction* m_acNewItem;
    KAction* m_acDeleteItem;
    KAction* m_acExport;

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

    /** initializes the KActions of the application */
    void initActions();
    /** sets up the statusbar for the main window by initialzing a statuslabel.
     */

    /** initializes the document object of the main window that is connected to the view in initView().
     * @see initView();
     */
    void initView();
    /** queryClose is called by KTMainWindow on each closeEvent of a window. Against the
     * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
     * be saved if Modified; on cancel the closeEvent is rejected.
     * @see KTMainWindow#queryClose
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryClose();
    /** queryExit is called by KTMainWindow when the last window of the application is going to be closed during the closeEvent().
     * Against the default implementation that just returns true, this calls saveOptions() to save the settings of the last window's
     * properties.
     * @see KTMainWindow#queryExit
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryExit();

};

#endif // KATALOGVIEW_H

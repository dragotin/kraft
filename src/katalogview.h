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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for Qt

// include files for KDE
#include <kapp.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kurl.h>

class KatalogListView;
class Katalog;
class KListViewItem;
class QListViewItem;
class FilterHeader;
class CatalogWidget;
class KListView;
class QBoxLayout;
class KActionMenu;
class DocPosition;

/**
  * The base class for Kange katalog view.
  * @see KMainWindow
  * @see KApplication
  * @see KConfig
  *
  * @author Klaas Freitag <freitag@kde.org>
  * @version $Id$
  */
class KatalogView : public KMainWindow
{
  Q_OBJECT

  public:
    /** construtor of a catalog view.
     * Note: init must be called immediately after instanciating a inherited
     * class of KatalogView
     */
    KatalogView(QWidget* parent=0, const char* name=0);
    ~KatalogView();

    /**
     * create a special listview for the kind of catalog. This method must
     * be overwritten in inherited catalog view classes.
     */
    virtual void createCentralWidget(QBoxLayout*, QWidget*);
    virtual KatalogListView* getListView(){return 0;};
    void init( const QString& );

  protected:
    virtual Katalog* getKatalog( const QString& );
      
  public slots:
    /** open a new application window by creating a new instance of KangeApp */
    void slotFileNewWindow();
    /** clears the document in the actual view to reuse it as the new document */
    void openDocumentFile(const KURL& url);

    void slotFileOpen();
    /** save a document */
    void slotFileSave();
    /** asks for saving if the file is modified, then closes the actual file and window*/
    void slotFileClose();
    /** print the actual file */
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

    virtual void slListviewExecuted(QListViewItem*);
    void slExport();
    
    virtual void slEditChapters();
    virtual void slAddToDocument();
    
    signals:
    /**
    * emitted if an entry in a catalog gets selected to be appended
    * to one or more documents. 
    */
    void newDocPosition( const DocPosition& );
    
    protected:

    /** the configuration object of the application */
    KConfig *config;
    KAction* fileClose;
    KAction* filePrint;
    KAction* editCut;
    KAction* editCopy;
    KAction* editPaste;

    KAction* m_acEditChapters;
    KAction* m_acEditItem;
    KAction* m_acNewItem;
    KAction* m_acExport;
    KAction* m_acToDocument;
    
    // KToggleAction* viewToolBar;
    // KToggleAction* viewStatusBar;
    QString         m_katalogName;
    FilterHeader    *m_filterHead;
    KListViewItem   *m_editListViewItem;

    // Fills the DocPosition with the data from the currently selected item in the view
    virtual bool currentItemToDocPosition( DocPosition& ){ return false; }
    
      /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
     * file
       */
    void saveOptions();
    /** read general Options again and initialize all variables like the recent file list
     */
    void readOptions();
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
    /** saves the window properties for each open window during session end to the session config file, including saving the currently
     * opened file by a temporary filename provided by KApplication.
     * @see KTMainWindow#saveProperties
     */
    virtual void saveProperties(KConfig *_cfg);
    /** reads the session config file and restores the application's state including the last opened files and documents by reading the
     * temporary files saved by saveProperties()
     * @see KTMainWindow#readProperties
     */
    virtual void readProperties(KConfig *_cfg);

};

#endif // KATALOGVIEW_H

/***************************************************************************
                     kraftdoc.h - Kraft document class
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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

#ifndef KRAFTDOC_H
#define KRAFTDOC_H

// include files for QT
#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>

#include "docposition.h"
#include "dbids.h"
#include "docguardedptr.h"

// forward declaration of the Kraft classes

class DocumentSaverBase;
class Geld;

class QLocale;
class KraftView;

class KraftDoc : public QObject
{
  Q_OBJECT
    public:
  enum Part { Header,  Positions, Footer, Unknown };

  static QString partToString( Part );

  /** Constructor for the fileclass of the application */
  KraftDoc(QWidget *parent = nullptr);
  /** Destructor for the fileclass of the application */
  ~KraftDoc();

  KraftDoc& operator=( KraftDoc& );  

  /** sets the modified flag for the document after a modifying action 
   *  on the view connected to the document.*/
  void setModified(bool _m=true){ modified=_m; }
  /** returns if the document is modified or not. Use this to determine 
   *  if your document needs saving by the user on closing.*/
  bool isModified(){ return modified; }
  /** deletes the document's contents */
  void deleteContents();
  /** initializes the document generally */
  bool newDocument( const QString& docType = QString() );
  /** closes the current document */
  void closeDocument();
  /** loads the document by filename and format and emits the updateViews() signal */
  bool openDocument(const QString& );
  /** fetch the document from database back */
  bool reloadDocument();
  /** saves the document under filename and format.*/
  bool saveDocument( );

  QLocale* locale();

  DocPosition* createPosition( DocPositionBase::PositionType t = DocPositionBase::Position );
  DocPositionList positions() { return mPositions; }
  void setPositionList( DocPositionList );

  QDate date() const { return mDate; }
  void setDate( QDate d ) { mDate = d; }

  QDate lastModified() const { return mLastModified; }
  void setLastModified( QDate d ) { mLastModified = d; }

  QString docType() const { return mDocType; }
  void setDocType( const QString& s );
  bool docTypeChanged() { return mDocTypeChanged; }

  QString addressUid() const { return mAddressUid; }
  void setAddressUid( const QString& id ) { mAddressUid = id; }

  QString address() const { return mAddress; }
  void setAddress( const QString& adr ) { mAddress = adr; }

  bool isNew() const { return mIsNew; }

  QString ident() const   { return mIdent;    }
  void setIdent( const QString& str ) { mIdent = str; }

  QString salut() const   { return mSalut;    }
  void setSalut( const QString& str ) { mSalut = str; }

  QString goodbye() const   { return mGoodbye;    }
  void setGoodbye( const QString& str ) { mGoodbye = str; }

  QString preText() const   { return mPreText;  }
  void setPreText( const QString& str ) { mPreText = str; }

  QString postText() const { return mPostText; }
  void setPostText( const QString& str ) { mPostText = str; }

  QString whiteboard() const { return mWhiteboard; }
  void setWhiteboard( const QString& w ) { mWhiteboard = w; }

  QString projectLabel() const { return mProjectLabel; }
  void setProjectLabel( const QString& w ) { mProjectLabel = w; }

  QString predecessor() const { return mPredecessor; }
  void setPredecessor( const QString& w ) { mPredecessor = w; }
  QString predecessorDbId() const { return mPredecessorDbId; }
  void setPredecessorDbId( const QString& pId ) { mPredecessorDbId = pId; }

  void setDocID( dbID id ) { mDocID = id; }
  dbID docID() const { return mDocID; }

  QString docIdentifier() const;
  DBIdList removePositionList() { return mRemovePositions; }

  Geld nettoSum();
  Geld bruttoSum();
  Geld vatSum();

  QString country() const;
  QString language() const;


  public slots:
  /** calls redrawDocument() on all views connected to the document object and is 
   *  called by the view by which the document has been changed.
   *  As this view normally repaints itself, it is excluded from the paintEvent.
   */
  int slotAppendPosition( const DocPosition& );

  // The following slots take get the db id as argument
  void slotRemovePosition( int );
  void slotMoveUpPosition( int );
  void slotMoveDownPosition( int );
private:
  DocumentSaverBase* getSaver( const QString& saverHint = QString() );
  /** the modified flag of the current document */
  bool modified;
  bool mIsNew;

  QString mAddressUid;
  QString mProjectLabel;
  QString mAddress;
  QString mPreText;
  QString mPostText;
  QString mDocType;
  bool    mDocTypeChanged;
  QString mSalut;
  QString mGoodbye;
  QString mIdent;
  QString mWhiteboard;
  QString mPredecessor;
  QString mPredecessorDbId;

  // Two qualifiers for the locale settings.
  QString mCountry;
  QString mLanguage;
  QScopedPointer<QLocale> mLocale;

  QDate   mDate;
  QDate   mLastModified;
  DocPositionList mPositions;
  DBIdList mRemovePositions;
  DocumentSaverBase *mSaver;
  dbID    mDocID;
};

#endif // KraftDoc_H

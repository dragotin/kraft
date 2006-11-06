/***************************************************************************
                    reportgenerator.h - report generation
                             -------------------
    begin                : July 2006
    copyright            : (C) 2006 by Klaas Freitag
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
#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <qdom.h>
#include <qobject.h>

#include "kraftdoc.h"

class dbID;
class KProcess;
class QFile;
class QTextStream;

class ReportGenerator : public QObject
{
  Q_OBJECT

public:
  ~ReportGenerator();

  static ReportGenerator *self();
  void docPreview( const dbID& );
  void runTrml2Pdf( const QString&, const QString& );

public slots:
  void slotViewerClosed( KProcess * );
  void createRmlFromArchive( dbID );
  void createRmlFromDoc( DocGuardedPtr );

protected slots:
  void slotWroteStdin( KProcess* );
  void slotRecStdout( KProcess *, char *, int );
  void slotRecStderr( KProcess *, char *, int );
  QString readTemplate( const QString& );
private:
  QString fillupTemplateFromDoc( DocGuardedPtr );
  QString fillupTemplateFromArchive( const dbID& );
  int replaceTag( QString&, const QString&, const QString& = QString() );

  ReportGenerator();
  QString mOutFile;
  QString mErrors;

  QFile mFile;
  QTextStream mTargetStream;

  static ReportGenerator *mSelf;
  static KProcess *mProcess;
};

#endif

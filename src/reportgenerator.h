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

class KraftDoc;
class dbID;
class KProcess;

class ReportGenerator : public QObject
{
  Q_OBJECT

  public:
    ~ReportGenerator();

    static ReportGenerator *self();
    void docPreview( const dbID& );

  public slots:
    void slotViewerClosed( KProcess * );  
  private:
    ReportGenerator();
    
    static ReportGenerator *mSelf;
    KProcess *mProcess;
};

#endif

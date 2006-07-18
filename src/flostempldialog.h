/***************************************************************************
             flostempldialog  -
                             -------------------
    begin                : 2004-15-08
    copyright            : (C) 2004 by Klaas Freitag
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

#ifndef _FLOSTEMPLDIALOG_H
#define _FLOSTEMPLDIALOG_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files
#include <qptrdict.h>

#include <klocale.h>

#include "kraftglobals.h"
#include "flostab.h"
#include "calcpart.h"
#include "stockmaterial.h"
/**
 *
 */
class QListViewItem;
class FloskelTemplate;
class ZeitCalcPart;
class MaterialCalcPart;
class FixCalcPart;
class ZeitCalcPart;
class FixCalcDialog;
class MatCalcDialog;
class ZeitCalcDialog;
class MatEditor;
class Katalog;

class FlosTemplDialog : public d_calcTempl
{
    Q_OBJECT

public:
    FlosTemplDialog(QWidget *parent=0, const char* name=0, bool modal=false, WFlags fl=0);
    ~FlosTemplDialog();

    void setVorlage( FloskelTemplate* t, const QString& );

signals:
    void takeMaterialAnswer(const QString&);
    void editAccepted( FloskelTemplate* );
    void editRejected();
    void chapterChanged(int);
    
public slots:
    virtual void slAddFixPart();
    virtual void slEditFixPart();
    virtual void slRemoveFixPart();

    virtual void slEditTimePart();
    virtual void slAddTimePart();
    virtual void slRemoveTimePart();

    virtual void slAddMatPart();
    virtual void slEditMatPart();
    virtual void slRemoveMatPart();

    virtual void slCalcOrFix(int);
    virtual void slGewinnChange( int neuPreis );

    /* slot, wenn im Materialeditor ein Material fr die Kalkulation ausgew�lt wird */
    void slNewMaterial( int, double );
    void refreshPrices();
    void slManualPriceChanged(double );
    void slSetNewText();
    void setCalcparts();
    
    void slFixCalcPartChanged(FixCalcPart*);
    void slTimeCalcPartChanged(ZeitCalcPart*);
    void slMatCalcPartChanged(StockMaterial *, double );
    
protected slots:
    virtual void accept();
    virtual void reject();

private:
    virtual void drawTimeListEntry( QListViewItem *, ZeitCalcPart * );
    virtual void drawFixListEntry( QListViewItem*, FixCalcPart* );
    virtual void drawMatListEntry( QListViewItem*, MaterialCalcPart*, StockMaterial* );

    bool askChapterChange( FloskelTemplate*, int);
    
    virtual QString stdMaterialKalcPartName()
    {
        return i18n("Kalkuliertes Material");
    }

    FloskelTemplate *m_template;
    FloskelTemplate *m_saveTempl;
    Katalog         *m_katalog;

    /* dict das qlistviewitems auf calcparts abbildet */
    QPtrDict<CalcPart> m_cpDict;
    QPtrDict<StockMaterial> m_matDict;

    MatEditor *m_matEdit;
    FixCalcDialog *m_fixCalcDia;
    ZeitCalcDialog *m_timePartDialog;
    MatCalcDialog *m_matPartDialog;
    CalcPart *m_cpChange;

};

#endif

/* END */


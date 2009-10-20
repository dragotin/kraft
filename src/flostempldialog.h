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

#include <klocale.h>
#include <kdialog.h>

#include "kraftglobals.h"
#include "ui_flostab.h"
#include "calcpart.h"
#include "stockmaterial.h"
/**
 *
 */
class Q3ListViewItem;
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

class FlosTemplDialog : public KDialog, protected Ui::d_calcTempl
{
    Q_OBJECT

public:
    FlosTemplDialog(QWidget *parent=0, bool modal=false );
    virtual ~FlosTemplDialog();

    void setTemplate( FloskelTemplate* t, const QString&, bool );
    bool templateIsNew() {
      return m_templateIsNew;
    };
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
    virtual void drawTimeListEntry( Q3ListViewItem *, ZeitCalcPart * );
    virtual void drawFixListEntry( Q3ListViewItem*, FixCalcPart* );
    virtual void drawMatListEntry( Q3ListViewItem*, MaterialCalcPart*, StockMaterial* );

    bool askChapterChange( FloskelTemplate*, int);

    virtual QString stdMaterialKalcPartName()
    {
        return i18n("Kalkuliertes Material");
    }

    FloskelTemplate *m_template;
    Katalog         *m_katalog;

    /* dict das qlistviewitems auf calcparts abbildet */
    QHash<Q3ListViewItem*, CalcPart*> mCalcPartDict;
    QHash<Q3ListViewItem*, StockMaterial*> m_matDict;

    MatEditor *m_matEdit;
    FixCalcDialog *m_fixCalcDia;
    ZeitCalcDialog *m_timePartDialog;
    MatCalcDialog *m_matPartDialog;
    CalcPart *m_cpChange;
    bool m_templateIsNew;
};

#endif

/* END */


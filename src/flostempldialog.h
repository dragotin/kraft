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

// include files

#include <QDialog>

#include "kraftglobals.h"
#include "ui_calctemplate.h"
#include "calcpart.h"
#include "stockmaterial.h"

class FloskelTemplate;
class TimeCalcPart;
class MaterialCalcPart;
class FixCalcPart;
class FixCalcDialog;
class MatCalcDialog;
class TimeCalcDialog;
class Katalog;
class QDialogButtonBox;

class FlosTemplDialog : public QDialog, protected Ui::d_calcTempl
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
    virtual void slBenefitChange(int newBen );

    /* slot for adding a new material to the material calculation */
    void slNewMaterial( int, double );
    void refreshPrices();
    void slManualPriceChanged(double );
    void slSetNewText();
    void setCalcparts();

    void slFixCalcPartChanged(FixCalcPart*);
    void slTimeCalcPartChanged(TimeCalcPart*);
    void slMatCalcPartChanged(MaterialCalcPart*);

    virtual void accept();
    virtual void reject();
    virtual void closeEvent ( QCloseEvent * event );
    bool confirmClose();

private:
    void setupConnections();
    void setButtonIcons();
    double benefitValue();
    bool templModified();

    virtual void drawTimeListEntry( QTreeWidgetItem *, TimeCalcPart * );
    virtual void drawFixListEntry( QTreeWidgetItem*, FixCalcPart* );
    virtual void drawMatListEntry( QTreeWidgetItem*, MaterialCalcPart* );

    bool askChapterChange( FloskelTemplate*, int);

    virtual QString stdMaterialKalcPartName()
    {
        return i18n("Calculated material");
    }

    FloskelTemplate *m_template;
    Katalog         *m_katalog;

    /* dict das qlistviewitems auf calcparts abbildet */
    QHash<QTreeWidgetItem*, CalcPart*> mCalcPartDict;

    QButtonGroup *m_gbPriceSrc;
    QDialogButtonBox *_buttonBox;

    FixCalcDialog   *m_fixCalcDia;
    TimeCalcDialog  *m_timePartDialog;
    MatCalcDialog   *m_matPartDialog;
    bool             m_templateIsNew;
    bool             _calcPartsModified;
    CatalogTemplate::CalculationType _origCalcType;
};

#endif

/* END */


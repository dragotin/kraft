/****************************************************************************
** Form interface generated from reading ui file './timecalcpart.ui'
**
** Created: Fr Okt 22 17:46:22 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CALCDETAIL_TIME_H
#define CALCDETAIL_TIME_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class Q3Frame;
class KLineEdit;
class KIntNumInput;
class QCheckBox;
class QComboBox;
class QPushButton;

class calcdetail_time : public QDialog
{
    Q_OBJECT

public:
    calcdetail_time( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~calcdetail_time();

    QLabel* textLabel1;
    QLabel* textLabel2;
    Q3Frame* frame3;
    KLineEdit* m_nameEdit;
    QLabel* textLabel3_2_2;
    QLabel* textLabel3_2;
    QLabel* textLabel3;
    KIntNumInput* m_dauer;
    QLabel* textLabel4;
    QCheckBox* m_stdGlobal;
    QComboBox* m_hourSets;
    QPushButton* buttonHelp;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

protected:
    Q3GridLayout* calcdetail_timeLayout;
    QSpacerItem* spacer3;
    Q3VBoxLayout* layout6;
    Q3GridLayout* frame3Layout;
    Q3GridLayout* layout4;
    Q3HBoxLayout* layout2;
    Q3HBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

};

#endif // CALCDETAIL_TIME_H

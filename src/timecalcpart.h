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

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QFrame;
class KLineEdit;
class KIntNumInput;
class QCheckBox;
class QComboBox;
class QPushButton;

class calcdetail_time : public QDialog
{
    Q_OBJECT

public:
    calcdetail_time( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~calcdetail_time();

    QLabel* textLabel1;
    QLabel* textLabel2;
    QFrame* frame3;
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
    QGridLayout* calcdetail_timeLayout;
    QSpacerItem* spacer3;
    QVBoxLayout* layout6;
    QGridLayout* frame3Layout;
    QGridLayout* layout4;
    QHBoxLayout* layout2;
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

};

#endif // CALCDETAIL_TIME_H

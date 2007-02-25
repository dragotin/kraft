
#ifndef TEMPLTOPOSITIONDIALOGBASE
#define TEMPLTOPOSITIONDIALOGBASE

#include <kdialogbase.h>

class QWidget;
class DocPosition;
class DocPositionList;
class QComboBox;

class TemplToPositionDialogBase: public KDialogBase
{
  Q_OBJECT

public:
  TemplToPositionDialogBase( QWidget* );
  ~TemplToPositionDialogBase( );

  virtual void setDocPosition( DocPosition* ) = 0;
  void setPositionList( DocPositionList, int );
  int insertAfterPosition();
  virtual DocPosition docPosition() = 0;
protected:
  /**
   * Needs to be reimplemented to return a pointer to a
   * combobox which can be filled with the current list
   * of positions to let the user select where the new
   * pos should go to.
   */
  virtual QComboBox* getPositionCombo() = 0;
};

#endif

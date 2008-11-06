#ifndef KRAFTDOCPOSITIONSEDIT_H
#define KRAFTDOCPOSITIONSEDIT_H

#include "docheader.h"
#include "kraftdocedit.h"

class KraftViewScroll;
class KPushButton;

class KraftDocPositionsEdit : public KraftDocEdit
{
  Q_OBJECT
public:
  KraftDocPositionsEdit( QWidget* );

  // FIXME: Remove access to internal widgets
  KraftViewScroll *positionScroll() { return m_positionScroll; }

signals:
  void addPositionClicked();
  void addExtraClicked();
  void importItemsClicked();

private:
  KraftViewScroll *m_positionScroll;
};

#endif


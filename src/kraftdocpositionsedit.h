#ifndef KRAFTDOCPOSITIONSEDIT_H
#define KRAFTDOCPOSITIONSEDIT_H

#include "ui_docheader.h"
#include "kraftdocedit.h"

#include <QScrollArea>

class KraftViewScroll;
class KPushButton;

class PositionViewWidget;

class KraftViewScroll : public QScrollArea
{
  Q_OBJECT

public:
  KraftViewScroll( QWidget* );
  ~KraftViewScroll() { }

  void addChild( QWidget *child, int index );
  void removeChild( PositionViewWidget *child );
  void moveChild( PositionViewWidget *child, int index);
  int indexOf( PositionViewWidget *child);

private:
  QWidget *myWidget;
  QVBoxLayout *layout;
};

// ###########################################################################

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


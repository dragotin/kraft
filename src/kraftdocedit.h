#ifndef KRAFTDOCEDIT_H
#define KRAFTDOCEDIT_H

#include "ui_docheader.h"

class KraftDocEdit : public QWidget
{
  Q_OBJECT
public:
  KraftDocEdit( QWidget *parent );

  void setTitle( const QString & );
  QString title() const;
  
  void setColor( const QColor & );
  QColor color() const;

Q_SIGNALS:
  void modified(); 

public Q_SLOTS:
  void slotModified();

private:
  QString mTitle;
  QColor mColor;
};

#endif


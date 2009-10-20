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

signals:
  void modified(); 

public slots:
  void slotModified();

private:
  QString mTitle;
  QColor mColor;
};

#endif


#include "kraftdocedit.h"

#include <QDebug>

KraftDocEdit::KraftDocEdit( QWidget *parent )
  : QWidget( parent )
{
}

void KraftDocEdit::setTitle( const QString &title )
{
  mTitle = title;
}

QString KraftDocEdit::title() const
{
  return mTitle;
}

void KraftDocEdit::setColor( const QColor &color )
{
  mColor = color;
}

QColor KraftDocEdit::color() const
{
  return mColor;
}

void KraftDocEdit::slotModified()
{
  emit modified();
}


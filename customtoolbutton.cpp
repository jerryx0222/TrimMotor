#include "customtoolbutton.h"
#include <QMouseEvent>

CustomToolButton::CustomToolButton(QWidget *parent)
    : QToolButton(parent)
{

    setCheckable(true);

}

void CustomToolButton::mousePressEvent(QMouseEvent *event)
{
    event->ignore();
}

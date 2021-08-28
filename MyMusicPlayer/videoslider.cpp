#include "videoslider.h"
#include<QStyle>

VideoSlider::VideoSlider(QWidget *parent) : QSlider(parent)
{

}

//鼠标点击跳转
void VideoSlider::mousePressEvent(QMouseEvent *event)
{
    int value = QStyle::sliderValueFromPosition(
    minimum(), maximum(), event->pos().x(), width());
    setValue(value);
    emit SIG_valueChanged(value);
}



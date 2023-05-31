#include "mycombobox.h"

using namespace Ui;

MyComboBox::MyComboBox(QWidget *parent):QComboBox(parent)
{

}

MyComboBox::~MyComboBox()
{

}

//点击这个combobox会触发clicked信号
void MyComboBox::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit mouseSingleClickd();  //触发clicked信号
    }

    QComboBox::mousePressEvent(event);  //将该事件传给父类处理，这句话很重要，如果没有，父类无法处理本来的点击事件
}

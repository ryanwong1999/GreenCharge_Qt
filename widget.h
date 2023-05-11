#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QStringList>
#include <QList>
#include <QtSerialPort/QSerialPort>         // 提供访问串口的功能
#include <QtSerialPort/QSerialPortInfo>     // 提供系统中存在的串口信息
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include "JQChecksum.h"

QT_BEGIN_NAMESPACE
namespace Ui { class widget; }
QT_END_NAMESPACE

class widget : public QWidget
{
    Q_OBJECT

public:
    widget(QWidget *parent = nullptr);
    ~widget();
    void initUi();
    void scanSerial();
    void getData(const QByteArray &data);
    void Delay_MSec(unsigned int);
    void Send_Data(const QByteArray &data);
    QByteArray crc16Hex(QString);

private slots:
    void readData();
    void on_comboBox_serial_mouseSingleClickd();
    void getStatus();
    void on_pushButton_connect_clicked();
    void on_pushButton_set_clicked();
    void on_pushButton_close_clicked();

private:
    Ui::widget *ui;
    QSerialPort *serial;
    QTimer *timer1;
};
#endif // WIDGET_H

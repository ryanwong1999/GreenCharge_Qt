#include "widget.h"
#include "ui_widget.h"

widget::widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::widget)
{
    ui->setupUi(this);
    this->setProperty("canMove",true);
    this->setWindowTitle("电源管理");
    this->initUi();
    this->scan_serial();
}

widget::~widget()
{
    delete ui;
}

void widget::initUi()
{
    //初始化窗口边框
    //设置窗体透明
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    //设置无边框
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    //实例阴影shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    //设置阴影距离
    shadow->setOffset(0, 0);
    //设置阴影颜色
    shadow->setColor(QColor("#444444"));
    //设置阴影圆角
    shadow->setBlurRadius(16);
    //给嵌套QDialog设置阴影
    ui->w_bg->setGraphicsEffect(shadow);
    //给垂直布局器设置边距(此步很重要, 设置宽度为阴影的宽度)
    ui->horizontalLayout_3->setMargin(12);
}

void widget::scan_serial()
{
    ui->comboBox_serial->clear();
    QList<QSerialPortInfo> list  = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &info, list)
    {
        ui->comboBox_serial->addItem(info.portName());
    }
}

//读取接收到的数据
void widget::Read_Data()
{
    QByteArray buf;
    buf = serial->readAll();
    qDebug() << buf;
}

void widget::on_pushButton_connect_clicked()
{
    QString name = ui->pushButton_connect->text();//获取按钮上的文字
    if(name == tr("打开串口"))
    {
        serial = new QSerialPort;
        serial->setPortName(ui->comboBox_serial->currentText());//选择串口号
        if(!serial->open(QIODevice::ReadWrite))//打开串口
        {
            QMessageBox::information(this, tr("提示"),
                tr("打开串口失败,请查看串口是否被占用"), QMessageBox::Ok);
            qDebug() << "打开串口失败";
            return;
        }
        ui->pushButton_connect->setText(tr("关闭串口"));//改变按钮上的文字
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);//8位数据位
        serial->setParity(QSerialPort::NoParity);//无检验
        serial->setStopBits(QSerialPort::OneStop);//1位停止位
        serial->setFlowControl(QSerialPort::NoFlowControl);//无硬件控制
        ui->comboBox_serial->setDisabled(true);//串口号下拉列表变灰
        ui->label_state->setText("连接成功");
        qDebug() << "打开串口";
        //连接信号槽
        QObject::connect(serial, &QSerialPort::readyRead, this, &widget::Read_Data);
    }
    else
    {
        serial->close();//关闭串口
        ui->comboBox_serial->setEnabled(true);//串口号下拉列表变亮
        ui->pushButton_connect->setText(tr("打开串口"));
        ui->label_state->setText("连接断开");
        qDebug() << "关闭串口";
    }
}

void widget::on_pushButton_set_clicked()
{

}

void widget::on_pushButton_close_clicked()
{

}

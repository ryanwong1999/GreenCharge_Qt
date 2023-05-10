#include "widget.h"
#include "ui_widget.h"

bool connect_flag = false;
bool receive_flag = false;
int connect_cnt = 0;

widget::widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::widget)
{
    ui->setupUi(this);
    this->setProperty("canMove", true);
    this->setWindowTitle("电源管理");
    this->initUi();
    this->scanSerial();

    //初始化定时器
    timer1 = new QTimer(this);
    //连接定时器
    connect(timer1,SIGNAL(timeout()),this,SLOT(getStatus()));
    //开始倒计时
    timer1->start(1100);
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

void widget::getStatus()
{
    qDebug() << "进入GetStatus1";
    if(connect_flag)
    {
        if(!receive_flag)
        {
            connect_cnt++;
            //如果没有收到单片机发过来的数据则显示连接失败
            if(connect_cnt > 3) ui->label_state->setText("连接失败");
        }
        else
        {
            ui->label_state->setText("连接成功");
            connect_cnt = 0;
        }
    }
    else connect_cnt = 0;
}

void widget::scanSerial()
{
    ui->comboBox_serial->clear();
    QList<QSerialPortInfo> list  = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &info, list)
    {
        ui->comboBox_serial->addItem(info.portName());
    }
}

//读取接收到的数据
void widget::readData()
{
    receive_flag = true;
    QByteArray buf = serial->readAll();
    qDebug() << "buf:" << buf;
    //处理收到的数据
    getData(buf);
}

//处理收到的数据
//0x55  0xAA  0x21    0x14 cmd data 0x0D 0x0A
//HEAD1 HEAD2 address Len  cmd data end1 end2
void widget::getData(const QByteArray &data)
{
    bool bStatus = false;
    if(data.toHex().mid(0,2) == "55" && data.toHex().mid(2,2) == "aa")
    {
        int len = data.toHex().mid(4,2).toInt(&bStatus,16);
        if(data.toHex().mid(len*2-4,2) == "0d" && data.toHex().mid(len*2-2,2) == "0a")
        {
            qDebug() << "收到的数据:" << data;
            int cmd = data.toHex().mid(6,2).toInt(&bStatus,16);
            switch (cmd) {
            case 20:
                //设置电压
                float setV = data.toHex().mid(8,4).toInt(&bStatus,16);
                ui->lineEdit_vol->setText(QString::number(setV/10));
                //设置电流
                float setA = data.toHex().mid(12,4).toInt(&bStatus,16);
                ui->lineEdit_cur->setText(QString::number(setA/10));
                //当前电压
                float nowV = data.toHex().mid(16,4).toInt(&bStatus,16);
                ui->lineEdit_cur->setText(QString::number(nowV/10));
                //当前电流
                float nowA = data.toHex().mid(20,4).toInt(&bStatus,16);
                ui->lineEdit_cur->setText(QString::number(nowA/10));
                //停止电流
                float stopA = data.toHex().mid(24,2).toInt(&bStatus,16);
                ui->lineEdit_cur->setText(QString::number(stopA/10));
                //充电模式
                int chargeType = data.toHex().mid(26,2).toInt(&bStatus,16);
                if(chargeType == 0) ui->comboBox_mode->setCurrentIndex(0);
                else if(chargeType == 0) ui->comboBox_mode->setCurrentIndex(1);
                else if(chargeType == 0) ui->comboBox_mode->setCurrentIndex(2);
                else if(chargeType == 0) ui->comboBox_mode->setCurrentIndex(3);
                //充电状态
                //电池容量
                break;
            }
        }
    }
}

//点击连接按钮
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
        ui->pushButton_connect->setStyleSheet("background-color: #00C5CD;" "color: #FFFFFF;");//按键颜色改变
        connect_flag = true;
        qDebug() << "打开串口";
        //连接信号槽
        QObject::connect(serial, &QSerialPort::readyRead, this, &widget::readData);
    }
    else
    {
        serial->close();//关闭串口
        ui->comboBox_serial->setEnabled(true);//串口号下拉列表变亮
        ui->pushButton_connect->setText(tr("打开串口"));
        ui->label_state->setText("连接断开");
        ui->pushButton_connect->setStyleSheet("background-color: #f2f2f2;" "color: #000000;");
        connect_flag = false;
        receive_flag = false;
        qDebug() << "关闭串口";
    }
}

//点击串口下拉框 点击combobox刷新
void widget::on_comboBox_serial_mouseSingleClickd()
{
    //刷新串口
    scanSerial();
}

//SET_VOL_CUR  0x10
//SET_MODE     0x11
//CMD_CHG      0x20
//点击设置按钮
void widget::on_pushButton_set_clicked()
{

}

//点击关闭按钮
void widget::on_pushButton_close_clicked()
{

}

// CRC16校验函数（Modbus算法）
bool checkCRC(QByteArray data)
{
    unsigned int length = data.length();
    unsigned int crc = 0xFFFF;

    for (unsigned int i = 0; i < length; i++) {
        crc ^= (unsigned char)data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return (crc == 0);
}

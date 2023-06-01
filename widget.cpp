#include "widget.h"
#include "ui_widget.h"

bool connect_flag = false;
bool receive_flag = false;
int connect_cnt = 0;
int first_show = 0;
int change_show = 0;
int a = 0;
QString ChargeType;

QString SET_VOL_CUR = "10";
QString SET_MODE = "11";
QString CMD_CHG = "20";

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
    timer1->start(500);
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
//    qDebug() << "buf:" << buf;
    //处理收到的数据
    getData(buf);
}

void widget::Send_Data(const QByteArray &data)
{
    if(connect_flag)
    {
//        qDebug() << "Send_Data: " << data;
        serial->write(data);
        serial->waitForBytesWritten(10);
    }
}

//处理收到的数据
//0x55  0xAA  0x21    0x14 cmd data 0x0D 0x0A
//HEAD1 HEAD2 address Len  cmd data end1 end2
void widget::getData(const QByteArray &data)
{
    bool bStatus = false;
//    qDebug() << "data:" << data.toHex();
    if(data.toHex().mid(0,2) == "55" && data.toHex().mid(2,2) == "aa")
    {
        int len = data.toHex().mid(6,2).toInt(&bStatus,16);
        if(data.toHex().mid(len*2-4,2) == "0d" && data.toHex().mid(len*2-2,2) == "0a")
        {
//            qDebug() << "收到:" << data.toHex();
            int cmd = data.toHex().mid(8,2).toInt(&bStatus,16);
            switch (cmd) {
            case 32:    //16进制的20
                //设置电压
                float setV = data.toHex().mid(10,4).toInt(&bStatus,16);
                //设置电流
                float setA = data.toHex().mid(14,4).toInt(&bStatus,16);
                //当前电压
                float nowV = data.toHex().mid(18,4).toInt(&bStatus,16);
                //当前电流
                float nowA = data.toHex().mid(22,4).toInt(&bStatus,16);
                //设置停止电流
                float stopA = data.toHex().mid(26,2).toInt(&bStatus,16);
                //充电模式
                int chargeType = data.toHex().mid(28,2).toInt(&bStatus,16);
                //充电状态
                //电池容量
                //地址
                int addr = data.toHex().mid(36,2).toInt(&bStatus,16);
                //改变ui显示
                ui->label_vol_now->setText(QString::number(nowV/10));
                ui->label_cur_now->setText(QString::number(nowA/10));
                if(first_show == 0 || change_show == 1)
                {
                    ui->lineEdit_vol->setText(QString::number(setV/10));
                    ui->lineEdit_cur->setText(QString::number(setA/10));
                    ui->lineEdit_stop_cur->setText(QString::number(stopA/10));
                    ui->lineEdit_addr->setText(QString::number(addr));
                    if(chargeType == 0) ui->comboBox_mode->setCurrentIndex(0);
                    else if(chargeType == 1) ui->comboBox_mode->setCurrentIndex(1);
                    else if(chargeType == 2) ui->comboBox_mode->setCurrentIndex(2);
                    else if(chargeType == 3) ui->comboBox_mode->setCurrentIndex(3);
                    change_show = 0;
                    first_show = 1;
                }
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
        serial->setBaudRate(QSerialPort::Baud115200);
        serial->setDataBits(QSerialPort::Data8);//8位数据位
        serial->setParity(QSerialPort::NoParity);//无检验
        serial->setStopBits(QSerialPort::OneStop);//1位停止位
        serial->setFlowControl(QSerialPort::NoFlowControl);//无硬件控制
        ui->comboBox_serial->setDisabled(true);//串口号下拉列表变灰
        ui->comboBox_serial->setStyleSheet("background-color: #F2F2F2;");//按键颜色改变
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
        ui->comboBox_serial->setStyleSheet("background-color: #FFFFFF;");//按键颜色改变
        ui->pushButton_connect->setText(tr("打开串口"));
        ui->label_state->setText(tr("连接断开"));
        ui->pushButton_connect->setStyleSheet("background-color: #f2f2f2;" "color: #000000;");
        connect_flag = false;
        receive_flag = false;
        first_show = 0;
        qDebug() << "关闭串口";
    }
}

//点击串口下拉框 点击combobox刷新
void widget::on_comboBox_serial_mouseSingleClickd()
{
    //刷新串口
    scanSerial();
}

/*************************************************
接收命令: 55  AA  11  10  10  00  00  00  00  00  00  00  00 00 0D  0A
          head  adr len cmd voltage current cap tmp sta         ends
**************************************************/
//SET_VOL_CUR  0x10
//SET_MODE     0x11
//CMD_CHG      0x20
//点击设置按钮
void widget::on_pushButton_set_clicked()
{
    //电池类型
    switch(ui->comboBox_mode->currentIndex())
    {
      case 0: ChargeType = "00"; break;
      case 1: ChargeType = "01"; break;
      case 2: ChargeType = "02"; break;
      case 3: ChargeType = "03"; break;
    }
    //设置电压
    int setV = ui->lineEdit_vol->text().toFloat()*10;
    QString SetV = QString("%1").arg(setV, 4, 16, QLatin1Char('0'));
    //设置电流
    int setA = ui->lineEdit_cur->text().toFloat()*10;
    QString SetA = QString("%1").arg(setA, 4, 16, QLatin1Char('0'));
    //停止电流
    int stopA = ui->lineEdit_stop_cur->text().toFloat()*10;
    QString StopA = QString("%1").arg(stopA, 2, 16, QLatin1Char('0'));
    //地址
    int addr = ui->lineEdit_addr->text().toInt();
    if(addr > 65535) addr = 65535;  //最大值限制
    QString Addr = QString("%1").arg(addr, 4, 16, QLatin1Char('0'));

    QString setVolCur = "55aa1110" + SET_VOL_CUR + SetV + SetA + "00" + StopA + Addr + "000d0a";
    QString setMode = "55aa110a" + SET_MODE + "00" + ChargeType + "000d0a";
    auto sendVolCur = QByteArray::fromHex(setVolCur.toLatin1());
    auto sendMode = QByteArray::fromHex(setMode.toLatin1());

    qDebug() <<"sendVolCur:"<< sendVolCur.toHex();
    qDebug() <<"sendMode:"<< sendMode.toHex();
    Send_Data(sendVolCur);
    Delay_MSec(100);
    Send_Data(sendMode);
    Delay_MSec(500);
    change_show = 1;
}

//点击关闭按钮
void widget::on_pushButton_close_clicked()
{

}


//定时器用于判断是否通讯成功
void widget::getStatus()
{
//    qDebug() << "进入GetStatus1";
    if(connect_flag)
    {
        if(!receive_flag)
        {
            connect_cnt++;
            //如果没有收到单片机发过来的数据则显示连接失败
            if(connect_cnt > 6)
            {
                if(a == 0)
                {
                    ui->label_state->setText(tr("连接失败"));
                    a = 1;
                }
                else if(a == 1)
                {
                    ui->label_state->setText(tr("       "));
                    a = 0;
                }
            }
        }
        else
        {
            ui->label_state->setText(tr("连接成功"));
            connect_cnt = 0;
        }
    }
    else connect_cnt = 0;
}

//延时毫秒
void widget::Delay_MSec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

//crc校验
QByteArray widget::crc16Hex(QString originData)
{
    auto data = QByteArray::fromHex(originData.toLatin1());
    auto crc16ForModbus = JQChecksum::crc16ForModbus(data);
    QString n = QString::number(crc16ForModbus, 16);
    QString temp1 = n.mid(0, 2);
    QString temp2 = n.mid(2, 2);
    QString result = temp2 + temp1;
    QString all = originData + result;
    qDebug() << "crc16Hex: " << all;
    QByteArray allData = QByteArray::fromHex(all.toLatin1());
    return allData;
}

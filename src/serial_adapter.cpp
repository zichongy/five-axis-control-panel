#include "serial_adapter.h"
#include <QDebug>

#include "moc_serial_adapter.cpp"

SerialAdapter::SerialAdapter(QObject *parent) : QObject(parent)
{

}

void SerialAdapter::serialInitialize(bool serial_permission, QString desired_serial_name)
{
    is_serial_open_ = serial_permission;

    const QList<QSerialPortInfo> availableSerialList = QSerialPortInfo::availablePorts();

    //如果已经打开，则先关闭
    if(serial_port_->isOpen())
    {
        serial_port_->clear();
        serial_port_->close();
    }

    //如果要求继续打开，则重新设置串口
    if (is_serial_open_)
    {

        foreach(const QSerialPortInfo &portInfo, availableSerialList)
        {
            if (portInfo.portName() == desired_serial_name)
            {
                serial_port_->setPort(portInfo);
                if (!serial_port_->open(QIODevice::ReadWrite)) qDebug() << "Port" << portInfo.portName() << "open failed!";
                else
                {
                    qDebug() << "Port" << portInfo.portName() << "open success!";
                    serial_port_->setBaudRate(QSerialPort::Baud9600,QSerialPort::AllDirections);//设置波特率和读写方向
                    serial_port_->setDataBits(QSerialPort::Data8);                              //数据位为8位
                    serial_port_->setFlowControl(QSerialPort::NoFlowControl);                   //无流控制
                    serial_port_->setParity(QSerialPort::NoParity);                             //无校验位
                    serial_port_->setStopBits(QSerialPort::OneStop);                            //一位停止位
                }
            }
        }
    }
}

void SerialAdapter::manualPostureReset(bool is_position_reset_,bool is_orientation_reset_)
{
    if(is_position_reset_)
    {
        serialWrite(x_reset_);
        serialWrite(y_reset_);
        serialWrite(z_reset_);
    }

    if(is_orientation_reset_)
    {
        serialWrite(r1_reset_);
        serialWrite(r2_reset_);
    }
}

void SerialAdapter::manualAngleAdjust(bool is_r1_,bool is_plus_,double changed_angle)
{
    if (is_r1_)
    {
        if (is_plus_) serialWrite(r1_plus_speed_);
        if (!is_plus_) serialWrite(r1_minus_speed_);
        serialWrite(r1_start_);
        QTimer::singleShot(angleToTime(changed_angle),this,SLOT(serialWrite_r1_stop()));
    }
    if (!is_r1_)
    {
        if (is_plus_) serialWrite(r2_plus_speed_);
        if (!is_plus_) serialWrite(r2_minus_speed_);
        serialWrite(r2_start_);
        QTimer::singleShot(angleToTime(changed_angle),this,SLOT(serialWrite_r2_stop()));
    }
}

void SerialAdapter::serialWrite_r1_stop() {serialWrite(r1_stop_);}
void SerialAdapter::serialWrite_r2_stop() {serialWrite(r2_stop_);}

void SerialAdapter::serialWrite(QString command_to_send_)
{
    QString command_text_ = command_to_send_;
    if (command_text_.contains(" ")) command_text_.replace(QString(" "),QString(""));

    //qDebug() << "Write to serial: " << command_text_;

    QByteArray command_byteArray_ = QByteArray::fromHex(command_text_.toLatin1());
    serial_port_->write(command_byteArray_);

    Sleep(30);//测试最小多少

}

void SerialAdapter::slotAutoConSerialWrite(int x_stat, int y_stat) {
    //flag: 只有状态改变时才重新发串口指令
    if (x_stat != flag_x_stat_){
        switch (x_stat) {
            case 0: {
                qDebug() << "Auto: X STOP.";
                if (flag_x_stat_ == 1) serialWrite(x_plus_stop_);
                if (flag_x_stat_ == 2) serialWrite(x_minus_stop_);
                break;
            }
            case 1: {
                qDebug() << "Auto: X PLUS.";
                serialWrite(x_plus_speed_medium_);
                serialWrite(x_plus_start_);
                break;
            }
            case 2: {
                qDebug() << "Auto: X MINUS.";
                serialWrite(x_minus_speed_medium_);
                serialWrite(x_minus_start_);
                break;
            }
        }
    }

    if (y_stat != flag_y_stat_){
        switch (y_stat) {
            case 0: {
                qDebug() << "Auto: Y STOP.";
                if (flag_y_stat_ == 1) serialWrite(y_plus_stop_);
                if (flag_y_stat_ == 2) serialWrite(y_minus_stop_);
                break;
            }
            case 1: {
                qDebug() << "Auto: Y PLUS.";
                serialWrite(y_plus_speed_medium_);
                serialWrite(y_plus_start_);
                break;
            }
            case 2: {
                qDebug() << "Auto: Y MINUS.";
                serialWrite(y_minus_speed_medium_);
                serialWrite(y_minus_start_);
                break;
            }
        }
    }

    flag_x_stat_ = x_stat;
    flag_y_stat_ = y_stat;
}

void SerialAdapter::Sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

int SerialAdapter::angleToTime(double the_angle_)
{
    return (int) ((the_angle_ / 6) * 1000);
}

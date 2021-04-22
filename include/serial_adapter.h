#ifndef SERIALADAPTER_H
#define SERIALADAPTER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTime>
#include <QTimer>
#include <QCoreApplication>

class SerialAdapter : public QObject
{
    Q_OBJECT

public:
    explicit SerialAdapter(QObject *parent = nullptr);

private slots:
    void serialInitialize(bool,QString);//初始化

    //来自窗口的手动信号
    void manualPostureReset(bool,bool);//分别对应位置和角度
    void manualAngleAdjust(bool,bool,double);
    void serialWrite_r1_stop();
    void serialWrite_r2_stop();

    //来自自动控制的信号
    void slotAutoConSerialWrite(int x_stat, int y_stat);

public slots:
    void serialWrite(QString command_to_send_);

private:
    QSerialPort *serial_port_ = new QSerialPort;

    bool is_serial_open_;

    void Sleep(int msec);//延时程序
    int angleToTime(double the_angle_);//将角度转换为时间

    //记录自动控制中的运行状态
    //0-stop, 1-plus, 2-minus
    int flag_x_stat_ = 0;
    int flag_y_stat_ = 0;

signals:

public:
    //串口信号
    const QString x_reset_ = "01 06 00 30 00 01 48 05";
    const QString y_reset_ = "02 06 00 30 00 01 48 36";
    const QString z_reset_ = "03 06 00 30 00 01 49 E7";
    const QString r1_reset_ = "04 06 00 30 00 01 48 50";
    const QString r2_reset_ = "05 06 00 30 00 01 49 81";

    //正向启动停止
    const QString x_plus_start_ = "01 06 00 27 00 02 B8 00";
    const QString y_plus_start_ = "02 06 00 27 00 02 B8 33";
    const QString z_plus_start_ = "03 06 00 27 00 02 B9 E2";

    const QString x_plus_stop_ = "01 06 00 28 00 00 09 C2";
    const QString y_plus_stop_ = "02 06 00 28 00 00 09 F1";
    const QString z_plus_stop_ = "03 06 00 28 00 00 08 20";

    //反向启动停止
    const QString x_minus_start_ = "01 06 00 27 00 02 B8 00";
    const QString y_minus_start_ = "02 06 00 27 00 02 B8 33";
    const QString z_minus_start_ = "03 06 00 27 00 02 B9 E2";

    const QString x_minus_stop_ = "01 06 00 28 00 00 09 C2";
    const QString y_minus_stop_ = "02 06 00 28 00 00 09 F1";
    const QString z_minus_stop_ = "03 06 00 28 00 00 08 20";

    //正向调速
    const QString x_plus_speed_slow_ = "01 06 00 23 FF FB 78 73";
    const QString y_plus_speed_slow_ = "02 06 00 23 FF FB 78 40";
    const QString z_plus_speed_slow_ = "03 06 00 23 FF FB 79 91";

    const QString x_plus_speed_medium_ = "01 06 00 23 FF F6 B9 B6";
    const QString y_plus_speed_medium_ = "02 06 00 23 FF F6 B9 85";
    const QString z_plus_speed_medium_ = "03 06 00 23 FF F6 B8 54";

    const QString x_plus_speed_high_ = "01 06 00 23 FF EC 38 7D";
    const QString y_plus_speed_high_ = "02 06 00 23 FF EC 38 4E";
    const QString z_plus_speed_high_ = "03 06 00 23 FF EC 39 9F";

    //反向调速
    const QString x_minus_speed_slow_ = "01 06 00 23 00 05 B8 03";
    const QString y_minus_speed_slow_ = "02 06 00 23 00 05 B8 30";
    const QString z_minus_speed_slow_ = "03 06 00 23 00 05 B9 E1";

    const QString x_minus_speed_medium_ = "01 06 00 23 00 0C 78 05";
    const QString y_minus_speed_medium_ = "02 06 00 23 00 0C 78 36";
    const QString z_minus_speed_medium_ = "03 06 00 23 00 0C 79 E7";

    const QString x_minus_speed_high_ = "01 06 00 23 00 14 78 0F";
    const QString y_minus_speed_high_ = "02 06 00 23 00 14 78 3C";
    const QString z_minus_speed_high_ = "03 06 00 23 00 14 79 ED";

    //角度
    const QString r1_start_ = "04 06 00 27 00 02 B8 55";
    const QString r2_start_ = "05 06 00 27 00 02 B9 84";

    const QString r1_stop_ = "04 06 00 28 00 00 09 97";
    const QString r2_stop_ = "05 06 00 28 00 00 08 46";

    const QString r1_plus_speed_ = "04 06 00 23 00 01 B9 95";
    const QString r2_plus_speed_ = "05 06 00 23 FF FF 78 34";

    const QString r1_minus_speed_ = "04 06 00 23 FF FF 79 E5";
    const QString r2_minus_speed_ = "05 06 00 23 00 01 B8 44";
};

#endif // SERIALADAPTER_H

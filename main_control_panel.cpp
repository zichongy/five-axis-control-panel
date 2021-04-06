#include "main_control_panel.h"
#include "ui_main_control_panel.h"

#include <QCameraInfo>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //User Interface
    ui->setupUi(this);
    ui->gridLayout_camera_switch->addWidget(toggle_switch_camera_);
    ui->gridLayout_serial_switch->addWidget(toggle_switch_serial_);

    //相机
    //获取相机列表并列出
    const QList<QCameraInfo> availableCameraList = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, availableCameraList) ui->comboBox_camera->addItem(cameraInfo.description());
    //默认相机
    if (ui->comboBox_camera->findText("SPCA2100 PC Camera",Qt::MatchContains) != -1){
        ui->comboBox_camera->setCurrentIndex(ui->comboBox_camera->findText("SPCA2100 PC Camera",Qt::MatchContains));
    }
    else qDebug() << "Cannot find the default camera!";
    //连接相机开关和VideoAdapter
    connect(toggle_switch_camera_,SIGNAL(toggled(bool)),this,SLOT(toggle_camera_switched(bool)));
    connect(this,SIGNAL(cameraInitializeInfo(bool,QString)),ui->video_window,SLOT(cameraStatusChanged(bool,QString)));

    //串口
    //串口UI初始化
    const QList<QSerialPortInfo> availableSerialList = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &portInfo, availableSerialList)
    {
        ui->comboBox_serial->addItem(portInfo.portName());
    }
    //默认串口
    if (ui->comboBox_serial->findText("cu.usbserial",Qt::MatchContains) != -1 ){
        ui->comboBox_serial->setCurrentIndex(ui->comboBox_serial->findText("cu.usbserial",Qt::MatchContains));
    }
    else qDebug() << "Cannot find the default serial port!";
    //连接串口开关和SerialAdapter
    connect(toggle_switch_serial_,SIGNAL(toggled(bool)),this,SLOT(toggle_serial_switched(bool)));
    connect(this,SIGNAL(serialIntializeInfo(bool,QString)),serial_adapter_,SLOT(serialInitialize(bool,QString)));

    //五轴控制
    //位姿重置
    connect(this,SIGNAL(resetPostureMessage(bool,bool)),serial_adapter_,SLOT(manualPostureReset(bool,bool)));
    //角度控制
    connect(this,SIGNAL(angleRelativeChange(bool,bool,double)),serial_adapter_,SLOT(manualAngleAdjust(bool,bool,double)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 相
// 机
// 控
// 制

void MainWindow::toggle_camera_switched(bool camera_status)
{
    camera_permission_ = camera_status;
    on_comboBox_camera_currentTextChanged();
}

void MainWindow::on_comboBox_camera_currentTextChanged()
{
    camera_name_selection_ = ui->comboBox_camera->currentText();
    emit cameraInitializeInfo(camera_permission_, camera_name_selection_);
}

// 串
// 口
// 控
// 制

void MainWindow::toggle_serial_switched(bool serial_status)
{
    serial_permission_ = serial_status;
    on_comboBox_serial_currentTextChanged();
}

void MainWindow::on_comboBox_serial_currentTextChanged()
{
    serial_name_selection_ = ui->comboBox_serial->currentText();
    emit serialIntializeInfo(serial_permission_, serial_name_selection_);
}


// 以
// 下
// 为
// 位
// 置
// 更
// 改

void MainWindow::on_pushButton_resetPosition_clicked()
{
    emit resetPostureMessage(true,false);
}

void MainWindow::on_pushButton_resetOrientation_clicked()
{
    emit resetPostureMessage(false,true);
    r1_current_angle_ = 0.0;
    r2_current_angle_ = 0.0;
    ui->doubleSpinBox_r1_angle->setValue(0.0);
    ui->doubleSpinBox_r2_angle->setValue(0.0);
}

void MainWindow::on_pushButton_resetAll_clicked()
{
    emit resetPostureMessage(true,true);
    r1_current_angle_ = 0.0;
    r2_current_angle_ = 0.0;
    ui->doubleSpinBox_r1_angle->setValue(0.0);
    ui->doubleSpinBox_r2_angle->setValue(0.0);
}

void MainWindow::on_pushButton_x_plus_pressed()
{
    is_force_stop_ = false;
    is_press_function_end_ = false;

    switch (ui->comboBox_x_speed->currentIndex())
    {
        case 0:{
            serial_adapter_->serialWrite(serial_adapter_->x_plus_speed_slow_);
            qDebug() << "Set X+ speed to slow";
            break;
        }
        case 1:{
            serial_adapter_->serialWrite(serial_adapter_->x_plus_speed_medium_);
            qDebug() << "Set X+ speed to medium";
            break;
        }
        case 2:{
            serial_adapter_->serialWrite(serial_adapter_->x_plus_speed_high_);
            qDebug() << "Set X+ speed to high";
            break;
        }
    }

    serial_adapter_->serialWrite(serial_adapter_->x_plus_start_);

    if (is_force_stop_)
        {
            qDebug() <<  "Force Stop.";
            serial_adapter_->serialWrite(serial_adapter_->x_plus_stop_);
        }

    is_force_stop_ = false;
    is_press_function_end_ = true;
    qDebug() << "End of X+ press slot function.";
}

void MainWindow::on_pushButton_x_plus_released()
{
    if (is_press_function_end_ == false)
    {
        is_force_stop_ = true;
    }
    else serial_adapter_->serialWrite(serial_adapter_->x_plus_stop_);
    qDebug() << "End of X+ release slot function.";
}

void MainWindow::on_pushButton_x_minus_pressed()
{
    is_force_stop_ = false;
    is_press_function_end_ = false;

    switch (ui->comboBox_x_speed->currentIndex())
    {
        case 0:{
            serial_adapter_->serialWrite(serial_adapter_->x_minus_speed_slow_);
            qDebug() << "Set X- speed to slow";
            break;
        }
        case 1:{
            serial_adapter_->serialWrite(serial_adapter_->x_minus_speed_medium_);
            qDebug() << "Set X- speed to medium";
            break;
        }
        case 2:{
            serial_adapter_->serialWrite(serial_adapter_->x_minus_speed_high_);
            qDebug() << "Set X- speed to high";
            break;
        }
    }

    serial_adapter_->serialWrite(serial_adapter_->x_minus_start_);

    if (is_force_stop_)
        {
            qDebug() <<  "Force Stop.";
            serial_adapter_->serialWrite(serial_adapter_->x_minus_stop_);
        }

    is_force_stop_ = false;
    is_press_function_end_ = true;
    qDebug() << "End of X- press slot function.";
}

void MainWindow::on_pushButton_x_minus_released()
{
    if (is_press_function_end_ == false)
    {
        is_force_stop_ = true;
    }
    else serial_adapter_->serialWrite(serial_adapter_->x_minus_stop_);
    qDebug() << "End of X- release slot function.";
}

void MainWindow::on_pushButton_y_plus_pressed()
{
    is_force_stop_ = false;
    is_press_function_end_ = false;

    switch (ui->comboBox_y_speed->currentIndex())
    {
        case 0:{
            serial_adapter_->serialWrite(serial_adapter_->y_plus_speed_slow_);
            qDebug() << "Set Y+ speed to slow";
            break;
        }
        case 1:{
            serial_adapter_->serialWrite(serial_adapter_->y_plus_speed_medium_);
            qDebug() << "Set Y+ speed to medium";
            break;
        }
        case 2:{
            serial_adapter_->serialWrite(serial_adapter_->y_plus_speed_high_);
            qDebug() << "Set Y+ speed to high";
            break;
        }
    }

    serial_adapter_->serialWrite(serial_adapter_->y_plus_start_);

    if (is_force_stop_)
        {
            qDebug() <<  "Force Stop.";
            serial_adapter_->serialWrite(serial_adapter_->y_plus_stop_);
        }

    is_force_stop_ = false;
    is_press_function_end_ = true;
    qDebug() << "End of Y+ press slot function.";
}

void MainWindow::on_pushButton_y_plus_released()
{
    if (is_press_function_end_ == false)
    {
        is_force_stop_ = true;
    }
    else serial_adapter_->serialWrite(serial_adapter_->y_plus_stop_);
    qDebug() << "End of Y+ release slot function.";
}

void MainWindow::on_pushButton_y_minus_pressed()
{
    is_force_stop_ = false;
    is_press_function_end_ = false;

    switch (ui->comboBox_y_speed->currentIndex())
    {
        case 0:{
            serial_adapter_->serialWrite(serial_adapter_->y_minus_speed_slow_);
            qDebug() << "Set Y- speed to slow";
            break;
        }
        case 1:{
            serial_adapter_->serialWrite(serial_adapter_->y_minus_speed_medium_);
            qDebug() << "Set Y- speed to medium";
            break;
        }
        case 2:{
            serial_adapter_->serialWrite(serial_adapter_->y_minus_speed_high_);
            qDebug() << "Set Y- speed to high";
            break;
        }
    }

    serial_adapter_->serialWrite(serial_adapter_->y_minus_start_);

    if (is_force_stop_)
        {
            qDebug() <<  "Force Stop.";
            serial_adapter_->serialWrite(serial_adapter_->y_minus_stop_);
        }

    is_force_stop_ = false;
    is_press_function_end_ = true;
    qDebug() << "End of Y- press slot function.";
}

void MainWindow::on_pushButton_y_minus_released()
{
    if (is_press_function_end_ == false)
    {
        is_force_stop_ = true;
    }
    else serial_adapter_->serialWrite(serial_adapter_->y_minus_stop_);
    qDebug() << "End of Y- release slot function.";
}

void MainWindow::on_pushButton_z_plus_pressed()
{
    is_force_stop_ = false;
    is_press_function_end_ = false;

    switch (ui->comboBox_z_speed->currentIndex())
    {
        case 0:{
            serial_adapter_->serialWrite(serial_adapter_->z_plus_speed_slow_);
            qDebug() << "Set Z+ speed to slow";
            break;
        }
        case 1:{
            serial_adapter_->serialWrite(serial_adapter_->z_plus_speed_medium_);
            qDebug() << "Set Z+ speed to medium";
            break;
        }
        case 2:{
            serial_adapter_->serialWrite(serial_adapter_->z_plus_speed_high_);
            qDebug() << "Set Z+ speed to high";
            break;
        }
    }

    serial_adapter_->serialWrite(serial_adapter_->z_plus_start_);

    if (is_force_stop_)
        {
            qDebug() <<  "Force Stop.";
            serial_adapter_->serialWrite(serial_adapter_->z_plus_stop_);
        }

    is_force_stop_ = false;
    is_press_function_end_ = true;
    qDebug() << "End of Z+ press slot function.";
}

void MainWindow::on_pushButton_z_plus_released()
{
    if (is_press_function_end_ == false)
    {
        is_force_stop_ = true;
    }
    else serial_adapter_->serialWrite(serial_adapter_->z_plus_stop_);
    qDebug() << "End of Z+ release slot function.";
}

void MainWindow::on_pushButton_z_minus_pressed()
{
    is_force_stop_ = false;
    is_press_function_end_ = false;

    switch (ui->comboBox_z_speed->currentIndex())
    {
        case 0:{
            serial_adapter_->serialWrite(serial_adapter_->z_minus_speed_slow_);
            qDebug() << "Set Z- speed to slow";
            break;
        }
        case 1:{
            serial_adapter_->serialWrite(serial_adapter_->z_minus_speed_medium_);
            qDebug() << "Set Z- speed to medium";
            break;
        }
        case 2:{
            serial_adapter_->serialWrite(serial_adapter_->z_minus_speed_high_);
            qDebug() << "Set Z- speed to high";
            break;
        }
    }

    serial_adapter_->serialWrite(serial_adapter_->z_minus_start_);

    if (is_force_stop_)
        {
            qDebug() <<  "Force Stop.";
            serial_adapter_->serialWrite(serial_adapter_->z_minus_stop_);
        }

    is_force_stop_ = false;
    is_press_function_end_ = true;
    qDebug() << "End of Z- press slot function.";
}

void MainWindow::on_pushButton_z_minus_released()
{
    if (is_press_function_end_ == false)
    {
        is_force_stop_ = true;
    }
    else serial_adapter_->serialWrite(serial_adapter_->z_minus_stop_);
    qDebug() << "End of Z- release slot function.";
}

// 以
// 下
// 为
// 角
// 度
// 更
// 改

void MainWindow::on_doubleSpinBox_r1_angle_valueChanged(double r1_angle)
{
    double r1_relative_value_change = r1_angle - r1_current_angle_;

    qDebug() << "Relative angle value change" << abs(r1_relative_value_change);

    if (r1_relative_value_change > 0) emit angleRelativeChange(true,true,abs(r1_relative_value_change));
    if (r1_relative_value_change < 0) emit angleRelativeChange(true,false,abs(r1_relative_value_change));

    r1_current_angle_ = r1_angle;
}

void MainWindow::on_doubleSpinBox_r2_angle_valueChanged(double r2_angle)
{
    double r2_relative_value_change = r2_angle - r2_current_angle_;

    qDebug() << "Relative angle value change" << abs(r2_relative_value_change);

    if (r2_relative_value_change > 0) emit angleRelativeChange(false,true,abs(r2_relative_value_change));
    if (r2_relative_value_change < 0) emit angleRelativeChange(false,false,abs(r2_relative_value_change));

    r2_current_angle_ = r2_angle;
}

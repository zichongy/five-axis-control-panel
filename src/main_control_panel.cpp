#include "main_control_panel.h"
#include "ui_main_control_panel.h"

#include "moc_main_control_panel.cpp"

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
    ui->horizontalLayout_laser->addWidget(toggle_switch_laserTrack_);
    ui->horizontalLayout_autoTrace->addWidget(toggle_switch_autoTrace_);
    ui->horizontalLayout_calibrate->addWidget(toggle_switch_calibrate_);

    //相机
    //获取相机列表并列出
    const QList<QCameraInfo> availableCameraList = QCameraInfo::availableCameras();
    QList<QString> cameraList;
    foreach (const QCameraInfo &cameraInfo, availableCameraList)
    {
        ui->comboBox_camera->addItem(cameraInfo.description());
        cameraList.insert(cameraList.size(),cameraInfo.description());
    }
    qDebug() << "Available camera list: " << cameraList;
    //默认相机
    if (ui->comboBox_camera->findText("SPCA2100 PC Camera",Qt::MatchContains) != -1){
        ui->comboBox_camera->setCurrentIndex(ui->comboBox_camera->findText("SPCA2100 PC Camera",Qt::MatchContains));
    }
    else qDebug() << "Cannot find the default camera!";
    //连接相机开关和VideoAdapter
    connect(toggle_switch_camera_,SIGNAL(toggled(bool)),this,SLOT(toggle_camera_switched(bool)));
    connect(this,SIGNAL(cameraInitializeInfo(bool,int)),ui->video_window,SLOT(cameraStatusChanged(bool,int)));

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
    connect(this,SIGNAL(serialInitializeInfo(bool,QString)),serial_adapter_,SLOT(serialInitialize(bool,QString)));

    //五轴控制
    //位姿重置
    connect(this,SIGNAL(resetPostureMessage(bool,bool)),serial_adapter_,SLOT(manualPostureReset(bool,bool)));
    //角度控制
    connect(this,SIGNAL(angleRelativeChange(bool,bool,double)),serial_adapter_,SLOT(manualAngleAdjust(bool,bool,double)));

    //视觉控制
    connect(toggle_switch_laserTrack_,SIGNAL(toggled(bool)),this,SLOT(toggle_laserTrack_switched(bool)));
    connect(toggle_switch_autoTrace_,SIGNAL(toggled(bool)),this,SLOT(toggle_autoTrace_switched(bool)));
    connect(toggle_switch_calibrate_,SIGNAL(toggled(bool)),this,SLOT(toggle_calibrate_switched(bool)));
    connect(this,SIGNAL(sigLaserTrackFlag(bool)),ui->video_window,SLOT(slotLaserTrackFlag(bool)));
    connect(this,SIGNAL(sigAutoTraceFleg(bool)),ui->video_window,SLOT(slotAutoTraceFlag(bool)));
    connect(this,SIGNAL(sigRotateAngle(double)),ui->video_window,SLOT(slotRotateAngle(double)));
    connect(ui->video_window,SIGNAL(sigAutoConSerialWrite(int,int)),serial_adapter_,SLOT(slotAutoConSerialWrite(int,int)));
    connect(ui->video_window,SIGNAL(sigStopAutoTrace()),this,SLOT(slotStopAutoTrace()));
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
    camera_index_selection_ = ui->comboBox_camera->currentIndex();
    emit cameraInitializeInfo(camera_permission_, camera_index_selection_);
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
    emit serialInitializeInfo(serial_permission_, serial_name_selection_);
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
    if (!is_press_function_end_)
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
    if (!is_press_function_end_)
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
    if (!is_press_function_end_)
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
    if (!is_press_function_end_)
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
    if (!is_press_function_end_)
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
    if (!is_press_function_end_)
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

    sigRotateAngle(r1_angle);
}

void MainWindow::on_doubleSpinBox_r2_angle_valueChanged(double r2_angle)
{
    double r2_relative_value_change = r2_angle - r2_current_angle_;

    qDebug() << "Relative angle value change" << abs(r2_relative_value_change);

    if (r2_relative_value_change > 0) emit angleRelativeChange(false,true,abs(r2_relative_value_change));
    if (r2_relative_value_change < 0) emit angleRelativeChange(false,false,abs(r2_relative_value_change));

    r2_current_angle_ = r2_angle;
}

void MainWindow::toggle_laserTrack_switched(bool laserTrack_on_) {
    if(!laserTrack_on_ && toggle_switch_autoTrace_->isChecked()){
        toggle_switch_autoTrace_->setChecked(false);
    }
    emit sigLaserTrackFlag(laserTrack_on_);
}

void MainWindow::toggle_autoTrace_switched(bool autoTrace_on_) {
    if(autoTrace_on_ && !toggle_switch_laserTrack_->isChecked()){
        toggle_switch_laserTrack_->setChecked(true);
    }
    if(autoTrace_on_ && !toggle_switch_serial_->isChecked()) {
        toggle_switch_serial_->setChecked(true);
    }
    if(autoTrace_on_ && !toggle_switch_camera_->isChecked()) {
        toggle_switch_camera_->setChecked(true);
    }
    emit sigAutoTraceFleg(autoTrace_on_);
}

void MainWindow::toggle_calibrate_switched(bool _is_calibrate) {
    QString msg_title = "Angle Conversion";

    //关了要开
    if (!flag_angle_calibration_ && _is_calibrate) {
        QString msg_content = "Do you want to convert the angles to match the video stream?";
        if (QMessageBox::question(this,msg_title,msg_content) == QMessageBox::Yes ) {
            qDebug() << "Converting the angles to match the video stream！";

            r1_original_angle_ = r1_current_angle_;
            r2_original_angle_ = r2_current_angle_;
            flag_angle_calibration_ = true;
        }
        else {
            qDebug() << "Convert cancelled!";
            flag_angle_calibration_ = false;
            toggle_switch_calibrate_->setChecked(false);
        }
    }

    //开了要关
    if(flag_angle_calibration_ && !_is_calibrate) {
        QString msg_content = "Do you want to revert the converted angles to original?";
        if (QMessageBox::question(this,msg_title,msg_content) == QMessageBox::Yes ) {
            qDebug() << "Reverting the angle conversion!";
            ui->doubleSpinBox_r1_angle->setValue(r1_original_angle_);
            ui->doubleSpinBox_r2_angle->setValue(r2_original_angle_);
            flag_angle_calibration_ = false;
        }
        else {
            qDebug() << "Keep angle converted!";
            flag_angle_calibration_ = true;
            toggle_switch_calibrate_->setChecked(true);
        }
    }
}

void MainWindow::slotStopAutoTrace() {
    toggle_switch_autoTrace_->setChecked(false);
}
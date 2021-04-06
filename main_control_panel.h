#ifndef CONTROL_PANEL_MAIN_H
#define CONTROL_PANEL_MAIN_H

#include <QMainWindow>
#include "toggle_switch.h"
#include "serial_adapter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //User Interface
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    //User Interface
    Ui::MainWindow *ui;  

    //Camera
    Switch* toggle_switch_camera_ = new Switch("");
    bool camera_permission_ = false;
    QString camera_name_selection_;

    //Serial
    SerialAdapter *serial_adapter_ = new SerialAdapter;
    Switch* toggle_switch_serial_ = new Switch("");
    bool serial_permission_ = false;
    QString serial_name_selection_;

    //5 Axis Control
    bool is_press_function_end_ = false;
    bool is_force_stop_ = true;

    //记录当前角度
    double r1_current_angle_ = 0.0;
    double r2_current_angle_ = 0.0;

private slots:
    //Camera
    void toggle_camera_switched(bool);
    void on_comboBox_camera_currentTextChanged();

    //Serial
    void toggle_serial_switched(bool);
    void on_comboBox_serial_currentTextChanged();

    //5-Axis Control
    void on_pushButton_resetPosition_clicked();
    void on_pushButton_resetOrientation_clicked();
    void on_pushButton_resetAll_clicked();

    void on_pushButton_x_plus_pressed();
    void on_pushButton_x_plus_released();
    void on_pushButton_x_minus_pressed();
    void on_pushButton_x_minus_released();

    void on_pushButton_y_plus_pressed();
    void on_pushButton_y_plus_released();
    void on_pushButton_y_minus_pressed();
    void on_pushButton_y_minus_released();

    void on_pushButton_z_plus_pressed();
    void on_pushButton_z_plus_released();
    void on_pushButton_z_minus_pressed();
    void on_pushButton_z_minus_released();

    void on_doubleSpinBox_r1_angle_valueChanged(double r1_angle);
    void on_doubleSpinBox_r2_angle_valueChanged(double r2_angle);

signals:
    //Camera
    void cameraInitializeInfo(bool,QString);

    //Serial Settings
    void serialIntializeInfo(bool,QString);

    //5 Axis Control
    void resetPostureMessage(bool,bool);//分别对应位置和角度
    void angleRelativeChange(bool is_r1_,bool is_plus_,double changed_angle);
};
#endif // MAINWINDOW_H

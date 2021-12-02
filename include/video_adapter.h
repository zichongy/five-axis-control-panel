#ifndef VIDEOADAPTER_H
#define VIDEOADAPTER_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <QThread>
#include <QWidget>
#include <QDebug>
#include <QObject>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>

//OpenCV解码类
class OpenCVAdapter: public QThread
{
    Q_OBJECT

public:
    explicit OpenCVAdapter(QObject *parent = nullptr);
    ~OpenCVAdapter() override;

    bool flag_camera_stop_ = true;

protected:
    void run() override;

private:
    int camera_index_ = 0;

public slots:
    void cvFrameInitialize(bool,int);

signals:
    void sigSendFrame(cv::Mat mat);
};

QT_BEGIN_NAMESPACE
namespace Ui { class VideoAdapter; }
QT_END_NAMESPACE

class VideoAdapter : public QWidget
{
    Q_OBJECT

public:
    explicit VideoAdapter(QWidget *parent = nullptr);
    ~VideoAdapter() override;

private slots:
    //Camera
    void cameraStatusChanged(bool,int); //接受相机名称

    //OpenCV
    void slotGetFrame(cv::Mat mat);
    static QImage cvMat2QImage(const cv::Mat &);

    //Visual Trace
    void slotLaserTrackFlag(bool);
    void slotAutoTraceFlag(bool);
    void slotRotateAngle(double);

private:
    Ui::VideoAdapter *ui;
    OpenCVAdapter *m_thread = nullptr;
    bool flag_camera_open_ = false; //接受主窗口信息，在statusChanged中修改

    //ArUco图像处理部分
    //ArUco字典
    cv::Ptr<cv::aruco::Dictionary> aruco_dictionary_;
    cv::Ptr<cv::aruco::DetectorParameters> aruco_parameters_;
    //ArUco标记数量、位置、类型信息
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    //ArUco处理间隔
    const int frame_interval_ = 4; //30FPS,每4帧处理一次
    int frame_countdown_ = frame_interval_;
    //ArUco识别到的目标点和激光点
    cv::Point2f dest_point_ = cv::Point2f(0,0);
    cv::Point2f laser_point_ = cv::Point2f(0,0);
    //分析相关函数
    cv::Mat framePosAnalyze(cv::Mat frame_,bool is_key_frame_);
    cv::Point2f findDestPoint(std::vector<std::vector<cv::Point2f>> _corners, std::vector<int> _ids);
    cv::Point2f findLaserPoint(cv::Mat &);

    //根据识别到的点自动运动控制
    bool flag_laser_track_ = false;
    bool flag_auto_trace_ = false;
    int flag_x_stat_ = 0;
    int flag_y_stat_ = 0;
    bool in_thresh_ = false;
    double centimeter_threshold = 0.4;
    int pixel_threshold = 0;
    double rotate_angle_ = 0.0;//旋转角，本代码来自R1，单位rad
    void autoMotionPlanning(cv::Point2f &, cv::Point2f &);
    bool diffInThresh(cv::Point2f &, cv::Point2f &); //false=圈外

signals:
    void sigCVInitialize(bool,int);

    //发送写串口的信号
    //此处 X Y 代表五轴的坐标系，像素坐标的X Y正方向和五轴相反
    //0-stop, 1-plus, 2-minus
    void sigAutoConSerialWrite(int x_stat, int y_stat);
    void sigStopAutoTrace();
};

#endif // VIDEOADAPTER_H

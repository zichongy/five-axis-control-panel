#ifndef VIDEOADAPTER_H
#define VIDEOADAPTER_H

#include <QWidget>
#include <QDebug>
#include <QObject>
//#include <QTimer>
//#include <QCamera>
//#include <QKeyEvent>
//#include <QCameraImageCapture>
//#include <QMediaRecorder>
//#include <QScopedPointer>

#include <QThread>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>

//OpenCV解码类
class OpenCVAdapter: public QThread
{
    Q_OBJECT

public:
    OpenCVAdapter(QObject *parent = nullptr);
    ~OpenCVAdapter();

    bool is_camera_stop_ = true;

protected:
    void run();

private:
    int camera_index_;

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
    VideoAdapter(QWidget *parent = nullptr);
    ~VideoAdapter();

private slots:
    //⬇️OpenCV测试用
    QImage cvMat2QImage(const cv::Mat mat);
    void slotGetFrame(cv::Mat mat);
    //⬆️OpenCV测试用

    void cameraStatusChanged(bool,int); //接受相机名称

private:
    Ui::VideoAdapter *ui;
    OpenCVAdapter *m_thread = nullptr;
    bool is_camera_open_ = false; //接受主窗口信息，在statusChanged中修改

    //ArUco图像处理部分
    cv::Ptr<cv::aruco::Dictionary> dictionary;
    cv::Ptr<cv::aruco::DetectorParameters> parameters;
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    const int frame_interval_ = 5; //30FPS,每6帧处理一次
    int frame_countdown_;
    cv::Mat framePosAnalyze(cv::Mat frame_,bool is_key_frame_);

signals:
    void sigCVInitialize(bool,int);
};

#endif // VIDEOADAPTER_H

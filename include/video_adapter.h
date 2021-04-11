#ifndef VIDEOADAPTER_H
#define VIDEOADAPTER_H

#include <QWidget>
#include <QTimer>
#include <QCamera>
#include <QKeyEvent>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QScopedPointer>

#include <QThread>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>


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
    void setCamera(const QCameraInfo &cameraInfo);

    void startCamera(); //打开相机，切换stackWidget
    void stopCamera(); //如果打开，关闭相机，切换stackWidget
    void displayViewfinder();
    void cameraStatusChanged(bool camera_permission, QString desired_camera_description); //接受相机名称

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::VideoAdapter *ui;

    QScopedPointer<QCamera> m_camera;
    bool is_camera_open_ = false; //接受主窗口信息，在statusChanged中修改

    QScopedPointer<QCameraImageCapture> m_imageCapture;
    QScopedPointer<QMediaRecorder> m_mediaRecorder;

    QImageEncoderSettings m_imageSettings;
    QAudioEncoderSettings m_audioSettings;
    QVideoEncoderSettings m_videoSettings;
    QString m_videoContainerFormat;
    bool m_isCapturingImage = false;
    bool m_applicationExiting = false;
};

class DecodeOpencv: public QThread
{
    Q_OBJECT

public:
    DecodeOpencv(QObject *parent = nullptr);
    ~DecodeOpencv();

    void setUrl(QString url);
    void setStopped (bool stop);

protected:
    void run();

private:
    QString m_rstp;
    bool m_is_stop_ = false;

signals:
    //void sigSendFrame(cv::Mat mat);
};


#endif // VIDEOADAPTER_H

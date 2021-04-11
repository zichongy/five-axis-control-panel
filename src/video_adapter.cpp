#include "video_adapter.h"
#include "ui_video_adapter.h"

#include "moc_video_adapter.cpp"

#include <QMediaService>
#include <QMediaRecorder>
#include <QCameraInfo>
#include <QMediaMetaData>

#include <QMessageBox>

VideoAdapter::VideoAdapter(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoAdapter)
{
    ui->setupUi(this);

    //Camera devices:
    const QList<QCameraInfo> availableCameraList = QCameraInfo::availableCameras();
    QList<QString> cameraList;
    foreach (const QCameraInfo &cameraInfo, availableCameraList)
    {
        cameraList.insert(cameraList.size(),cameraInfo.description());
    }
    qDebug() << "可用的相机" << cameraList;

    ui->stackedWidget->setCurrentIndex(1);
}

VideoAdapter::~VideoAdapter()
{
    delete ui;
}

void VideoAdapter::cameraStatusChanged(bool camera_permission, QString desired_camera_description)
{
    is_camera_open_ = camera_permission;
    const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
    QList<QString> cameraList;

    if( is_camera_open_ )
    {
        ui->stackedWidget->setCurrentIndex(0);
        foreach (const QCameraInfo &cameraInfo, availableCameras)
        {
            cameraList.insert(cameraList.size(),cameraInfo.description());
            if ( cameraInfo.description() == desired_camera_description )
            {
                setCamera(cameraInfo);
            }
        }
    }
    else stopCamera();
}

void VideoAdapter::setCamera(const QCameraInfo &cameraInfo)
{
    m_camera.reset(new QCamera(cameraInfo));
    m_mediaRecorder.reset(new QMediaRecorder(m_camera.data()));
    m_imageCapture.reset(new QCameraImageCapture(m_camera.data()));
    m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Camera")));
    m_camera->setViewfinder(ui->viewfinder);
    startCamera();
}

void VideoAdapter::startCamera()
{
    m_camera->start();
    ui->stackedWidget->setCurrentIndex(0);
}

void VideoAdapter::stopCamera()
{
    if( !m_camera.isNull() ) m_camera->stop();
    //ui->stackedWidget->setCurrentIndex(1);//取消注释可以在关闭相机后清空
}

void VideoAdapter::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void VideoAdapter::closeEvent(QCloseEvent *event)
{
    if (m_isCapturingImage) {
        setEnabled(false);
        m_applicationExiting = true;
        event->ignore();
    }
    else event->accept();
}

//ceshidaima
//ceshidaima
//ceshidaima
//ceshidaima
//ceshidaima
//ceshidaima
//ceshidaima

DecodeOpencv::DecodeOpencv(QObject *parent)
    : QThread(parent)
{
    //qRegisterMetaType<cv::Mat>("cv::Mat");
}

DecodeOpencv::~DecodeOpencv()
{
    m_is_stop_ = true;
}

void DecodeOpencv::setUrl(QString url)
{
    m_rstp = url;
}

void DecodeOpencv::setStopped(bool stop)
{
    m_is_stop_ = stop;
}

void DecodeOpencv::run()
{
//    cv::VideoCapture cap;
//    bool suc = cap.open(0);

//    if (!suc)
//    {
//        qDebug() << "播放失败";
//        return;
//    }
//    if (suc)
//    {
//        while (true) {
//            if(!m_is_stop_)
//            {
//                cv::Mat frame;
//                cap >> frame;
//                if(frame.empty())
//                {
//                    qDebug() << "解析失败";
//                    continue;
//                }
//                emit sigSendFrame(frame);
//            }
//            else
//            {
//                qDebug() << "停止播放";
//                break;
//            }
//        }
//    }
}

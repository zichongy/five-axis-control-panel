#include "video_adapter.h"
#include "ui_video_adapter.h"
#include "video_settings.h"
#include "image_settings.h"

#include <QMediaService>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QMediaMetaData>

#include <QMessageBox>
#include <QPalette>

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

    connect(m_camera.data(), &QCamera::stateChanged, this, &VideoAdapter::updateCameraState);
    connect(m_camera.data(), QOverload<QCamera::Error>::of(&QCamera::error), this, &VideoAdapter::displayCameraError);

    m_mediaRecorder.reset(new QMediaRecorder(m_camera.data()));
    //connect(m_mediaRecorder.data(), &QMediaRecorder::stateChanged, this, &VideoAdapter::updateRecorderState);

    m_imageCapture.reset(new QCameraImageCapture(m_camera.data()));

    //connect(m_mediaRecorder.data(), &QMediaRecorder::durationChanged, this, &VideoAdapter::updateRecordTime);
    connect(m_mediaRecorder.data(), QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error),
            this, &VideoAdapter::displayRecorderError);

    m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));

    //connect(ui->exposureCompensation, &QAbstractSlider::valueChanged, this, &VideoAdapter::setExposureCompensation);

    m_camera->setViewfinder(ui->viewfinder);

    updateCameraState(m_camera->state());
    updateLockStatus(m_camera->lockStatus(), QCamera::UserRequest);

//    connect(m_imageCapture.data(), &QCameraImageCapture::readyForCaptureChanged, this, &VideoAdapter::readyForCapture);
//    connect(m_imageCapture.data(), &QCameraImageCapture::imageCaptured, this, &VideoAdapter::processCapturedImage);
//    connect(m_imageCapture.data(), &QCameraImageCapture::imageSaved, this, &VideoAdapter::imageSaved);
//    connect(m_imageCapture.data(), QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
//            this, &VideoAdapter::displayCaptureError);

//    connect(m_camera.data(), QOverload<QCamera::LockStatus, QCamera::LockChangeReason>::of(&QCamera::lockStatusChanged),
//            this, &VideoAdapter::updateLockStatus);

//    ui->captureWidget->setTabEnabled(0, (m_camera->isCaptureModeSupported(QCamera::CaptureStillImage)));
//    ui->captureWidget->setTabEnabled(1, (m_camera->isCaptureModeSupported(QCamera::CaptureVideo)));

    updateCaptureMode();
    startCamera();
}

void VideoAdapter::keyPressEvent(QKeyEvent * event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        //displayViewfinder();
        m_camera->searchAndLock();
        event->accept();
        break;
    case Qt::Key_Camera:
        if (m_camera->captureMode() == QCamera::CaptureStillImage) {
            takeImage();
        } else {
            if (m_mediaRecorder->state() == QMediaRecorder::RecordingState)
                qDebug()<<"This feature has been removed.";
            else
                qDebug()<<"This feature has been removed.";
        }
        event->accept();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void VideoAdapter::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        m_camera->unlock();
        break;
    default:
        QWidget::keyReleaseEvent(event);
    }
}

void VideoAdapter::processCapturedImage(int requestId, const QImage& img)
{
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->viewfinder->size(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

    // Display captured image for 4 seconds.
    //displayCapturedImage();
    QTimer::singleShot(4000, this, &VideoAdapter::displayViewfinder);
}

void VideoAdapter::configureCaptureSettings()
{
    switch (m_camera->captureMode()) {
    case QCamera::CaptureStillImage:
        configureImageSettings();
        break;
    case QCamera::CaptureVideo:
        configureVideoSettings();
        break;
    default:
        break;
    }
}

void VideoAdapter::configureVideoSettings()
{
    VideoSettings settingsDialog(m_mediaRecorder.data());
    settingsDialog.setWindowFlags(settingsDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    settingsDialog.setAudioSettings(m_audioSettings);
    settingsDialog.setVideoSettings(m_videoSettings);
    settingsDialog.setFormat(m_videoContainerFormat);

    if (settingsDialog.exec()) {
        m_audioSettings = settingsDialog.audioSettings();
        m_videoSettings = settingsDialog.videoSettings();
        m_videoContainerFormat = settingsDialog.format();

        m_mediaRecorder->setEncodingSettings(
                    m_audioSettings,
                    m_videoSettings,
                    m_videoContainerFormat);

        m_camera->unload();
        m_camera->start();
    }
}

void VideoAdapter::configureImageSettings()
{
    ImageSettings settingsDialog(m_imageCapture.data());
    settingsDialog.setWindowFlags(settingsDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    settingsDialog.setImageSettings(m_imageSettings);

    if (settingsDialog.exec()) {
        m_imageSettings = settingsDialog.imageSettings();
        m_imageCapture->setEncodingSettings(m_imageSettings);
    }
}

void VideoAdapter::toggleLock()
{
    switch (m_camera->lockStatus()) {
    case QCamera::Searching:
    case QCamera::Locked:
        m_camera->unlock();
        break;
    case QCamera::Unlocked:
        m_camera->searchAndLock();
    }
}

void VideoAdapter::updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason)
{
//    QColor indicationColor = Qt::black;

//    switch (status) {
//    case QCamera::Searching:
//        indicationColor = Qt::yellow;
//        ui->statusbar->showMessage(tr("Focusing..."));
//        ui->lockButton->setText(tr("Focusing..."));
//        break;
//    case QCamera::Locked:
//        indicationColor = Qt::darkGreen;
//        ui->lockButton->setText(tr("Unlock"));
//        ui->statusbar->showMessage(tr("Focused"), 2000);
//        break;
//    case QCamera::Unlocked:
//        indicationColor = reason == QCamera::LockFailed ? Qt::red : Qt::black;
//        ui->lockButton->setText(tr("Focus"));
//        if (reason == QCamera::LockFailed)
//            ui->statusbar->showMessage(tr("Focus Failed"), 2000);
//    }

//    QPalette palette = ui->lockButton->palette();
//    palette.setColor(QPalette::ButtonText, indicationColor);
//    ui->lockButton->setPalette(palette);
}

void VideoAdapter::takeImage()
{
    m_isCapturingImage = true;
    m_imageCapture->capture();
}

void VideoAdapter::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    m_isCapturingImage = false;
}

void VideoAdapter::startCamera()
{
    m_camera->start();
    ui->stackedWidget->setCurrentIndex(0);
}

void VideoAdapter::stopCamera()
{
    if( !m_camera.isNull() )
    {
        m_camera->stop();
    }
    //ui->stackedWidget->setCurrentIndex(1);//取消注释可以在关闭相机后清空
}

void VideoAdapter::updateCaptureMode()
{
//    int tabIndex = ui->captureWidget->currentIndex();
//    QCamera::CaptureModes captureMode = tabIndex == 0 ? QCamera::CaptureStillImage : QCamera::CaptureVideo;

//    if (m_camera->isCaptureModeSupported(captureMode))
//        m_camera->setCaptureMode(captureMode);
}

void VideoAdapter::updateCameraState(QCamera::State state)
{
//    switch (state) {
//    case QCamera::ActiveState:
//        ui->actionStartCamera->setEnabled(false);
//        ui->actionStopCamera->setEnabled(true);
//        ui->captureWidget->setEnabled(true);
//        ui->actionSettings->setEnabled(true);
//        break;
//    case QCamera::UnloadedState:
//    case QCamera::LoadedState:
//        ui->actionStartCamera->setEnabled(true);
//        ui->actionStopCamera->setEnabled(false);
//        ui->captureWidget->setEnabled(false);
//        ui->actionSettings->setEnabled(false);
//    }
}

void VideoAdapter::setExposureCompensation(int index)
{
    m_camera->exposure()->setExposureCompensation(index*0.5);
}

void VideoAdapter::displayRecorderError()
{
    QMessageBox::warning(this, tr("Capture Error"), m_mediaRecorder->errorString());
}

void VideoAdapter::displayCameraError()
{
    QMessageBox::warning(this, tr("Camera Error"), m_camera->errorString());
}

void VideoAdapter::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void VideoAdapter::displayCapturedImage()
{
    //ui->stackedWidget->setCurrentIndex(1);
}

void VideoAdapter::readyForCapture(bool ready)
{
//    ui->takeImageButton->setEnabled(ready);
}

void VideoAdapter::imageSaved(int id, const QString &fileName)
{
//    Q_UNUSED(id);
//    ui->statusbar->showMessage(tr("Captured \"%1\"").arg(QDir::toNativeSeparators(fileName)));

//    m_isCapturingImage = false;
//    if (m_applicationExiting)
//        close();
}

void VideoAdapter::closeEvent(QCloseEvent *event)
{
    if (m_isCapturingImage) {
        setEnabled(false);
        m_applicationExiting = true;
        event->ignore();
    } else {
        event->accept();
    }
}

//void VideoAdapter::updateCameraDevice(QString device)
//{
//    const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
//    foreach (const QCameraInfo &cameraInfo, availableCameras)
//    {
//        if (cameraInfo.description() == device)
//        {
//            setCamera(cameraInfo);
//        }
//    }
//}
